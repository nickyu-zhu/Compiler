#include "def.h"
#include <stdio.h>

struct node *mknode(int kind,struct node *first,struct node *second, struct node *third, struct node *forth,int pos ){
  struct node *tempnode = (struct node*)malloc(sizeof(struct node));
  tempnode->kind = kind;
  tempnode->ptr[0] = first;
  tempnode->ptr[1] = second;
  tempnode->ptr[2] = third;
  tempnode->ptr[3] = forth;
  tempnode->pos = pos;
  return tempnode;
}

void display(struct node* T,int indent){
    if(T){
        switch (T->kind){
            case EXT_DEF_LIST:  
                display(T->ptr[0],indent);
                display(T->ptr[1],indent);
                break;
            case EXT_VAR_DEF:
                printf("%*c%s\n",indent,' ',"外部变量定义：");     
                display(T->ptr[0],indent+5);
                printf("%*c%s\n",indent+5,' ',"变量名：");
                display(T->ptr[1],indent+5);
                break;
            case FUNC_DEF:
                printf("%*c%s\n",indent,' ',"函数定义：");
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent+5);
                display(T->ptr[2],indent+5);
                break;
            case ARRAY_DEF:
                printf("%*c%s\n",indent,' ',"数组定义：");
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent+5);
                break;
            case FUNC_DEC:
                printf("%*c%s%s\n",indent,' ',"函数名：",T->type_id);
                printf("%*c%s\n",indent,' ',"函数型参：");
                display(T->ptr[0],indent+5);
                break;
            case ARRAY_DEC:
                printf("%*c%s%s\n",indent,' ',"数组名：",T->type_id);
                printf("%*c%s\n",indent,' ',"数组大小：");
                display(T->ptr[0],indent+5);
                break;
            case EXT_DEC_LIST:
                display(T->ptr[0],indent+5);
                if(T->ptr[1]->ptr[0]==NULL)
                    display(T->ptr[1],indent+5);
                else
                    display(T->ptr[1],indent);
                break;
            case PARAM_LIST:
                display(T->ptr[0],indent);
                display(T->ptr[1],indent);
                break;
            case PARAM_DEC:
                display(T->ptr[0],indent);
                display(T->ptr[1],indent);
                break;
            case VAR_DEF:
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent+5);
                break;
            case DEC_LIST:
                printf("%*c%s\n",indent,' ',"变量名：");
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent);
                break;
            case DEF_LIST:
                printf("%*c%s\n",indent+5,' ',"LOCAL VAR_NAME：");
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent);
                break;
            case COMP_STM:
                printf("%*c%s\n",indent,' ',"复合语句：");
                printf("%*c%s\n",indent+5,' ',"复合语句的变量定义：");
                display(T->ptr[0],indent+5);
                printf("%*c%s\n",indent+5,' ',"复合语句的语句部分：");
                display(T->ptr[1],indent+5);
                break;
            case STM_LIST:
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent);
                break;
            case EXP_STMT:
                printf("%*c%s\n",indent,' ',"表达式语句：");
                display(T->ptr[0],indent+5);
                break;
            case IF_THEN:
                printf("%*c%s\n",indent,' ',"条件语句（if-else）：");
                printf("%*c%s\n",indent,' ',"条件：");
                display(T->ptr[0],indent+5);
                printf("%*c%s\n",indent,' ',"IF语句：");
                display(T->ptr[1],indent+5);
                break;
            case IF_THEN_ELSE:
                printf("%*c%s\n",indent,' ',"条件语句（if-else-if）：");
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent+5);
                break;
            case WHILE:
                printf("%*c%s\n",indent,' ',"循环语句（while）：");
                printf("%*c%s\n",indent+5,' ',"循环条件：");
                display(T->ptr[0],indent+5);
                printf("%*c%s\n",indent+5,' ',"循环体：");
                display(T->ptr[1],indent+5);
                break;
            case FOR:
                printf("%*c%s\n",indent,' ',"循环语句（for）：");
                printf("%*c%s\n",indent+5,' ',"循环条件：");
                display(T->ptr[0],indent+5);
                printf("%*c%s\n",indent+5,' ',"循环体：");
                display(T->ptr[1],indent+5);
                break;
            case FOR_DEC:
                display(T->ptr[0], indent+5);
                display(T->ptr[1], indent+5);
                display(T->ptr[2], indent+5);
                break;
            case FOR_EXP1:
                printf("%*c循环初始语句\n", indent, ' ');
                display(T->ptr[0], indent+5);
                break;
            case FOR_EXP2:
                printf("%*c循环结束条件\n", indent, ' ');
                display(T->ptr[0], indent+5);
                break;
            case FOR_EXP3:
                printf("%*c循环更新语句\n", indent, ' ');
                display(T->ptr[0], indent+5);
                break;
            case FUNC_CALL:
                printf("%*c%s\n",indent,' ',"函数调用：");
                printf("%*c%s%s\n",indent+5,' ',"函数名：",T->type_id);
                printf("%*c%s\n",indent+5,' ',"实际参数：");
                display(T->ptr[0],indent+5);
                break;
            case ARGS:
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent);
                break;
            case ID:
                printf("%*cID： %s\n",indent,' ',T->type_id);//控制新的一行输出的空格数，indent代替%*c中*
                if(T->isarr) display(T->ptr[0],indent+5);
                break;
            case ARRAY_LIST:
                printf("%*c数组下标：\n",indent,' ');
                display(T->ptr[0],indent+5);
                break;
            case INT_LITERAL:
                printf("%*cINT： %d\n",indent,' ',T->type_int);  
                break;
            case CHAR_LITERAL:
                printf("%*cCHAR： %c\n",indent,' ',T->type_char[0]);
                break;
            case ARRAY:
                printf("%*c数组名称： %s\n",indent,' ',T->type_id);  
                break;
            case TYPE:
                if(T->type==INT_LITERAL)
                    printf("%*c%s\n",indent,' ',"类型：int");
                else if(T->type==CHAR_LITERAL)
                    printf("%*c%s\n",indent,' ',"类型：char");
                break;
            case BREAK:
                printf("%*cbreak语句\n",indent,' ');
                break;
            case CONTINUE:
                printf("%*ccontinue语句\n",indent,' ');
                break;
            case ASSIGNOP:
            case OR:
            case SELFADD:
            case SELFMINUS:
            case AND:
            case RELOP:
            case PLUS:
            case MINUS:
            case MUL:
            case DIV:
            case ADDASS:
            case MINUSASS:
                printf("%*c%s\n",indent,' ',T->type_id);
                display(T->ptr[0],indent+5);
                display(T->ptr[1],indent+5);
                break;
            case RETURN:
                printf("%*c%s\n",indent,' ',"返回语句：");
                display(T->ptr[0],indent+5);
                break;  
            default:
                printf("Type error.\n");
        }
    }
}
