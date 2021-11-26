#include "def.h"

extern struct symboltable symbolTable;

int main_flag = 0;
int LEV;
int current_offset = 0;

int searchSymbolTableByAlias(char *alias)
{
    int i;
    for (i = symbolTable.length - 1; i >= 0; i--)
        if (!strcmp(symbolTable.symbols[i].alias, alias))
            return i;
    return -1;
}

void object_code(struct codenode *head) {
    char opnstr1[32],opnstr2[32],resultstr[32];
    struct codenode *h=head,*p;
    int i;
    FILE *fp;
    fp=fopen("object.s","w");
    fprintf(fp,".data\n");
    fprintf(fp,"_Prompt: .asciiz \"Enter your input:  \"\n");
    fprintf(fp,"_ret: .asciiz \"\\n\"\n");
    fprintf(fp,".globl main\n");
    fprintf(fp,".text\n");
    fprintf(fp, "addi $gp, $gp, -400\n");
    fprintf(fp, "j main\n");
    fprintf(fp,"read:\n");
    fprintf(fp,"  li $v0,4\n");
    fprintf(fp,"  la $a0,_Prompt\n");
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  li $v0,5\n");
    fprintf(fp,"  syscall\n");   //将读取的值保存在v0寄存器中
    fprintf(fp,"  jr $ra\n");
    fprintf(fp,"write:\n");
    fprintf(fp,"  li $v0,1\n");
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  li $v0,4\n");
    fprintf(fp,"  la $a0,_ret\n");//将a0寄存器中的值打印出来
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  move $v0,$0\n");
    fprintf(fp,"  jr $ra\n");
    int cnt = 0;
    do{
        cnt++;
        switch (h->op)
        {
        case ASSIGNOP:
            if(h->opn1.kind == INT_LITERAL) {
                fprintf(fp, "  li $t3, %d\n", h->opn1.const_int);
            }
            else if(h->opn1.kind == CHAR_LITERAL) {
                fprintf(fp, "  li $t3, %c\n", h->opn1.const_char);
            }
            else{
                int rtn;
                if(strstr(h->opn1.id, "*") != NULL) {
                    
                    rtn = searchSymbolTableByAlias(h->opn1.name);
                    //printf("%s  %d\n",symbolTable.symbols[rtn].alias,symbolTable.symbols[rtn].offset);

                    if(rtn != -1 && symbolTable.symbols[rtn].level == 0){
                        fprintf(fp, "  lw $t2, %d($gp)\n",symbolTable.symbols[rtn].offset);
                        fprintf(fp, "  add $t2,$t2,$gp\n");
                        fprintf(fp, "  lw $t1, ($t2)\n");
                        fprintf(fp, "  move $t3, $t1\n");
                    }
                    else{
                        fprintf(fp, "  lw $t2, %d($sp)\n", symbolTable.symbols[rtn].offset);
                        //printf("%d\n",symbolTable.symbols[rtn].offset);
                        fprintf(fp, "  add $t2,$t2,$sp\n");
                        fprintf(fp, "  lw $t1, ($t2)\n");
                        fprintf(fp, "  move $t3, $t1\n");
                    }
                }
                else{
                    rtn = searchSymbolTableByAlias(h->opn1.id);
                    if(rtn != -1 && symbolTable.symbols[rtn].level == 0){
                        fprintf(fp, "  lw $t1, %d($gp)\n", h->opn1.offset);
                        fprintf(fp, "  move $t3, $t1\n");
                    }
                    else{
                        fprintf(fp, "  lw $t1, %d($sp)\n", h->opn1.offset);
                        fprintf(fp, "  move $t3, $t1\n");
                    }
                }
            
            }
            int rtn;
            
            if(strstr(h->result.id, "*") != NULL) {
                rtn = searchSymbolTableByAlias(h->result.name);
                    //printf("%s\n",h->result.name);
                
                if(rtn != -1 && symbolTable.symbols[rtn].level == 0){
                    fprintf(fp, "  lw $t2, %d($gp)\n",symbolTable.symbols[rtn].offset);
                    fprintf(fp, "  add $t2,$t2,$gp\n");
                    fprintf(fp, "  sw $t3, ($t2)\n");
                }
                else{
                    fprintf(fp, "  lw $t2, %d($sp)\n", symbolTable.symbols[rtn].offset);
                    fprintf(fp, "  add $t2,$t2,$sp\n");
                    fprintf(fp, "  sw $t3, ($t2)\n");
                }
            }
            else{
                rtn = searchSymbolTableByAlias(h->result.id);
                if(rtn != -1 && symbolTable.symbols[rtn].level == 0){
                    fprintf(fp, "  sw $t3, %d($gp)\n", h->result.offset);
                }
                else{
                    fprintf(fp, "  sw $t3, %d($sp)\n", h->result.offset);
                }
            }

            break;

        case PLUS:
        case MINUS:
        case MUL:
        case DIV:
            rtn = searchSymbolTableByAlias(h->opn1.id);
            if(rtn != -1 && (symbolTable.symbols[rtn].flag == 'A'||symbolTable.symbols[rtn].type == STRUCT)){
                fprintf(fp, "  li $t1, %d\n",symbolTable.symbols[rtn].offset);
                if(symbolTable.symbols[rtn].level==0)
                    fprintf(fp, "  add $t1,$t1,-400\n");
                if(h->opn2.kind==ID)
                    fprintf(fp, "  lw $t2, %d($sp)\n", h->opn2.offset);
                else 
                    fprintf(fp, "  li $t2, %d\n", h->opn2.const_int);
                fprintf(fp, "  add $t3, $t1, $t2\n");
                fprintf(fp, "  sw $t3, %d($sp)\n", h->result.offset);
            }else{
                fprintf(fp, "  lw $t1, %d($sp)\n", h->opn1.offset);
                fprintf(fp, "  lw $t2, %d($sp)\n", h->opn2.offset);
                if(h->op == PLUS) 
                    fprintf(fp, "  add $t3, $t1, $t2\n");
                else if(h->op == MINUS)
                    fprintf(fp, "  sub $t3, $t1, $t2\n");
                else if(h->op == MUL)
                    fprintf(fp, "  mul $t3, $t1, $t2\n");
                else if(h->op == DIV){
                    fprintf(fp, "  div $t1, $t2\n");
                    fprintf(fp, "  mflo $t3\n");
                }
                else{
                    fprintf(fp, "  div $t1, $t2\n");
                    fprintf(fp, "  mfhi $t3\n");
                }
                fprintf(fp, "  sw $t3, %d($sp)\n", h->result.offset);
            }
            break;

        case FUNCTION:
            if(!strcmp(h->result.id, "main")){
                main_flag = 1;
                LEV = 0;
            }
            fprintf(fp, "\n%s:\n", h->result.id);
            if(!strcmp(h->result.id, "main"))
                fprintf(fp, "  addi $sp, $sp, -%d\n", symbolTable.symbols[h->result.offset].offset);
            break;

        case PARAM:
            break;

        case LABEL:
            fprintf(fp, "%s:\n", h->result.id);
            break;
        case GOTO:  
            fprintf(fp, "  j %s\n", h->result.id);
            break;

        case JLE:
        case JLT:
        case JGE:
        case JGT:
        case EQ:
        case NEQ:
            fprintf(fp, "  lw $t1, %d($sp)\n", h->opn1.offset);
            fprintf(fp, "  lw $t2, %d($sp)\n", h->opn2.offset);
            if (h->op==JLE) 
                fprintf(fp, "  ble $t1,$t2,%s\n", h->result.id);
            else if (h->op==JLT) 
                fprintf(fp, "  blt $t1,$t2,%s\n", h->result.id);
            else if (h->op==JGE) 
                fprintf(fp, "  bge $t1,$t2,%s\n", h->result.id);
            else if (h->op==JGT) 
                fprintf(fp, "  bgt $t1,$t2,%s\n", h->result.id);
            else if (h->op==EQ)  
                fprintf(fp, "  beq $t1,$t2,%s\n", h->result.id);
            else                 
                fprintf(fp, "  bne $t1,$t2,%s\n", h->result.id);
            break;
        case ARG:
            break;

        case RETURN:
            if(!main_flag){
                fprintf(fp, "  lw $v0,%d($sp)\n",h->result.offset); //
                fprintf(fp, "  jr $ra\n");
            }
            else{
                fprintf(fp, "  li $v0, 10\n");
                fprintf(fp, "  syscall\n");
                main_flag = 0;
            }
            LEV--;
            break;

        case CALL:
            LEV++;
            if (!strcmp(h->opn1.id,"read")){ 
                fprintf(fp, "  addi $sp, $sp, -4\n");
                fprintf(fp, "  sw $ra,0($sp)\n"); 
                fprintf(fp, "  jal read\n"); 
                fprintf(fp, "  lw $ra,0($sp)\n"); 
                fprintf(fp, "  addi $sp, $sp, 4\n");
                fprintf(fp, "  sw $v0, %d($sp)\n",h->result.offset);
                break;
            }
            if (!strcmp(h->opn1.id,"write")){ 
                fprintf(fp, "  lw $a0, %d($sp)\n",h->prior->result.offset);
                fprintf(fp, "  addi $sp, $sp, -4\n");
                fprintf(fp, "  sw $ra,0($sp)\n");
                fprintf(fp, "  jal write\n");
                fprintf(fp, "  lw $ra,0($sp)\n");
                fprintf(fp, "  addi $sp, $sp, 4\n");
                break;
            }
            for(p = h,i = 0;i < symbolTable.symbols[h->opn1.offset].paramnum; i++)  
                p=p->prior;

            fprintf(fp, "  move $t0,$sp\n"); 
            fprintf(fp, "  addi $sp, $sp, -%d\n", symbolTable.symbols[h->opn1.offset].offset);
            fprintf(fp, "  sw $ra,0($sp)\n"); 
            i=h->opn1.offset+1;  
            while (symbolTable.symbols[i].flag == 'P'){
                fprintf(fp, "  lw $t1, %d($t0)\n", p->result.offset);
                fprintf(fp, "  move $t3,$t1\n");
                fprintf(fp, "  sw $t3,%d($sp)\n",  symbolTable.symbols[i].offset);
                p=p->next; 
                i++;
            }
            fprintf(fp, "  jal %s\n",h->opn1.id);
            fprintf(fp, "  lw $ra,0($sp)\n");
            fprintf(fp, "  addi $sp,$sp,%d\n",symbolTable.symbols[h->opn1.offset].offset);
            fprintf(fp, "  sw $v0,%d($sp)\n", h->result.offset); 
            break;
        
        default:
            break;
        }
    h = h->next;
    }while(h != head);
    fclose(fp);
}