%define parse.error verbose
%locations /* 记录行号 */
//声明部分
%{
    #include "stdio.h"
    #include "def.h"
    #include "math.h"
    extern int yylineno;
    extern char *yytext;
    extern FILE *yyin;
    void yyerror(const char* fmt, ...);
    void display(struct node *, int);
%}

%union{
    int type_int;       // 对应int_literal
    char type_char[3];
    char type_id[32];
    struct node *ptr;
}

// 非终结符的语义值类型
%type <ptr> program ExtDefList ExtDef Specifier ExtDecList FuncDec ArrayDec CompSt VarList VarDec ParamDec Stmt StmList DefList Def DecList Dec Exp Args
%type <ptr> ForDec ForExp1 ForExp2 ForExp3 INT_L ArrayList

// 终结符的语义值类型
%token <type_int> INT_LITERAL
%token <type_id> ID RELOP TYPE
%token <type_char> CHAR_LITERAL
%token LP RP LC RC SEMI COMMA LB RB
%token PLUS MINUS MUL DIV ASSIGNOP AND OR NOT ADDASS MINUSASS SELFADD SELFMINUS
%token IF ELSE WHILE RETURN FOR CONTINUE BREAK
%token FOR_DEC FOR_EXP1 FOR_EXP2 FOR_EXP3

// 优先级
%left ADDASS MINUSASS
%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left MUL DIV
%left SELFADD SELFMINUS
%right UMINUS NOT DPLUS

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

// 规则定义
%%
program: ExtDefList {display($1,0);Semantic_Analysis($1,0,0,' ',0);} /*归约到program，开始显示语法树,语义分析*/
       ;
/*ExtDefList：外部定义列表，即是整个语法树*/
ExtDefList: {$$=NULL;}/*整个语法树为空*/
            | ExtDef ExtDefList {$$=mknode(EXT_DEF_LIST,$1,$2,NULL,yylineno);}//每一个EXTDEFLIST的结点,其第1棵子树对应一个外部变量声明或函数
            ;
ExtDef: Specifier ExtDecList SEMI {$$=mknode(EXT_VAR_DEF,$1,$2,NULL,yylineno);}//该结点对应一个外部变量声明
        | Specifier ArrayDec SEMI {$$=mknode(ARRAY_DEF,$1,$2,NULL,yylineno);}//数组定义
        | Specifier FuncDec CompSt {$$=mknode(FUNC_DEF,$1,$2,$3,yylineno);}//该结点对应一个函数定义,类型+函数声明+复合语句
        | error SEMI {$$=NULL; printf("---缺少分号---\n");}
        ;
/* 表示一个类型，int和char */
Specifier: TYPE {$$=mknode(TYPE,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);$$->type=(!strcmp($1,"int")?INT_LITERAL:CHAR_LITERAL);}
           ;
/*变量名称列表，由一个或多个变量组成，多个变量之间用逗号隔开*/
ExtDecList: VarDec {$$=$1;}/*每一个EXT_DECLIST的结点,其第一棵子树对应一个变量名(ID类型的结点),第二棵子树对应剩下的外部变量名*/
            | VarDec COMMA ExtDecList {$$=mknode(EXT_DEC_LIST,$1,$3,NULL,yylineno);}
            ;
/*变量名称，由一个ID组成*/
VarDec: ID {$$=mknode(ID,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);$$->isarr=0;}//ID结点,标识符符号串存放结点的type_id
        ;
/*函数名+参数定义*/
FuncDec: ID LP VarList RP {$$=mknode(FUNC_DEC,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id
         | ID LP RP {$$=mknode(FUNC_DEC,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id
         | error RP {$$=NULL; printf("---函数左括号右括号不匹配---\n");}
         ;
/*数组声明*/
INT_L:  INT_LITERAL {$$=mknode(INT_LITERAL,NULL,NULL,NULL,yylineno);$$->type_int=$1;$$->type=INT_LITERAL;}
        ;
ArrayDec: LB INT_L RB ID {$$=mknode(ARRAY_DEC,$2,NULL,NULL,yylineno);strcpy($$->type_id,$4);}
          | error RB {$$=NULL;printf("---数组定义错误---\n");}
          ;
ArrayList: LB Exp RB {$$=mknode(ARRAY_LIST,$2,NULL,NULL,yylineno);}
/*参数定义列表，有一个到多个参数定义组成，用逗号隔开*/
VarList: ParamDec {$$=mknode(PARAM_LIST,$1,NULL,NULL,yylineno);}
         | ParamDec COMMA VarList {$$=mknode(PARAM_LIST,$1,$3,NULL,yylineno);}
         ;
/*参数定义，固定有一个类型和一个变量组成*/
ParamDec: Specifier VarDec {$$=mknode(PARAM_DEC,$1,$2,NULL,yylineno);}
          ;
/*复合语句，左右分别用大括号括起来，中间有定义列表和语句列表*/
CompSt: LC DefList StmList RC {$$=mknode(COMP_STM,$2,$3,NULL,yylineno);}
        | error RC {$$=NULL; printf("---复合语句内存在错误---\n");}
        ;
/*语句列表，由0个或多个语句stmt组成*/
StmList: {$$=NULL;}
         | Stmt StmList {$$=mknode(STM_LIST,$1,$2,NULL,yylineno);}
         ;
/*语句，可能为表达式，复合语句，return语句，if语句，if-else语句，while语句，for*/
Stmt: Exp SEMI {$$=mknode(EXP_STMT,$1,NULL,NULL,yylineno);}
      | CompSt {$$=$1;}//复合语句结点直接最为语句结点,不再生成新的结点
      | RETURN Exp SEMI {$$=mknode(RETURN,$2,NULL,NULL,yylineno);}
      | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE {$$=mknode(IF_THEN,$3,$5,NULL,yylineno);}
      | IF LP Exp RP Stmt ELSE Stmt {$$=mknode(IF_THEN_ELSE,$3,$5,$7,yylineno);}
      | WHILE LP Exp RP Stmt {$$=mknode(WHILE,$3,$5,NULL,yylineno);}
      | FOR LP ForDec RP Stmt {$$=mknode(FOR,$3,$5,NULL,yylineno);}
      | BREAK SEMI {$$=mknode(BREAK,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"BREAK");}
      | CONTINUE SEMI {$$=mknode(CONTINUE,NULL,NULL,NULL,yylineno);strcpy($$->type_id,"CONTINUE");}
      ;
ForDec: ForExp1 SEMI ForExp2 SEMI ForExp3 {$$=mknode(FOR_DEC,$1,$3,$5,yylineno);}
        ;
ForExp1: DecList {$$=mknode(FOR_EXP1,$1,NULL,NULL,yylineno);}
        ;
ForExp2: Exp {$$=mknode(FOR_EXP2,$1,NULL,NULL,yylineno);}
        ;
ForExp3: Exp {$$=mknode(FOR_EXP3,$1,NULL,NULL,yylineno);}
        ;
/*定义列表，由0个或多个定义语句组成*/
DefList: {$$=NULL; }
         | Def DefList {$$=mknode(DEF_LIST,$1,$2,NULL,yylineno);}
         ;
/*定义一个或多个语句语句，由分号隔开*/
Def: Specifier DecList SEMI {$$=mknode(VAR_DEF,$1,$2,NULL,yylineno);}
     | Specifier ArrayDec SEMI {$$=mknode(ARRAY_DEF,$1,$2,NULL,yylineno);}
     ;
/*语句列表，由一个或多个语句组成，由逗号隔开，最终都成一个表达式*/
DecList: Dec {$$=mknode(DEC_LIST,$1,NULL,NULL,yylineno);}
         | Dec COMMA DecList {$$=mknode(DEC_LIST,$1,$3,NULL,yylineno);}
         ;
/*语句，一个变量名称或者一个赋值语句（变量名称等于一个表达式）*/
Dec: VarDec {$$=$1;}
     | VarDec ASSIGNOP Exp {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"ASSIGNOP");}
     ;
/*表达式*/
Exp: Exp ASSIGNOP Exp {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"ASSIGNOP");}//$$结点type_id空置未用,正好存放运算符
     | Exp AND Exp {$$=mknode(AND,$1,$3,NULL,yylineno);strcpy($$->type_id,"AND");}
     | Exp OR Exp {$$=mknode(OR,$1,$3,NULL,yylineno);strcpy($$->type_id,"OR");}
     | Exp RELOP Exp {$$=mknode(RELOP,$1,$3,NULL,yylineno);strcpy($$->type_id,$2);}//词法分析关系运算符号自身值保存在$2中
     | Exp PLUS Exp {$$=mknode(PLUS,$1,$3,NULL,yylineno);strcpy($$->type_id,"PLUS");}
     | Exp MINUS Exp {$$=mknode(MINUS,$1,$3,NULL,yylineno);strcpy($$->type_id,"MINUS");}
     | Exp MUL Exp {$$=mknode(MUL,$1,$3,NULL,yylineno);strcpy($$->type_id,"MUL");}
     | Exp DIV Exp {$$=mknode(DIV,$1,$3,NULL,yylineno);strcpy($$->type_id,"DIV");}
     | Exp ADDASS Exp {$$=mknode(ADDASS,$1,$3,NULL,yylineno);strcpy($$->type_id,"ADDASS");}
     | Exp MINUSASS Exp {$$=mknode(MINUSASS,$1,$3,NULL,yylineno);strcpy($$->type_id,"MINUSASS");}
     | NOT Exp {$$=mknode(NOT,$2,NULL,NULL,yylineno);strcpy($$->type_id,"NOT");}
     | SELFADD Exp {$$=mknode(SELFADD_L,$2,NULL,NULL,yylineno);strcpy($$->type_id,"SELFADD");}
     | Exp SELFADD {$$=mknode(SELFADD_R,$1,NULL,NULL,yylineno);strcpy($$->type_id,"SELFADD");}
     | Exp SELFMINUS {$$=mknode(SELFMINUS_R,$1,NULL,NULL,yylineno);strcpy($$->type_id,"SELFMINUS");}
     | SELFMINUS Exp {$$=mknode(SELFMINUS_L,$2,NULL,NULL,yylineno);strcpy($$->type_id,"SELFMINUS");}
     | ID LP Args RP {$$=mknode(FUNC_CALL,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}/*函数定义后面的括号部分，只需要把括号里面的内容传入即可*/
     | ID LP RP {$$=mknode(FUNC_CALL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}/*函数定义后面的括号部分没有参数*/
     | ID {$$=mknode(ID,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);$$->isarr=0;}
     | INT_LITERAL {$$=mknode(INT_LITERAL,NULL,NULL,NULL,yylineno);$$->type_int=$1;$$->type=INT_LITERAL;}
     | CHAR_LITERAL {$$=mknode(CHAR_LITERAL,NULL,NULL,NULL,yylineno);strcpy($$->type_char,$1);$$->type=CHAR_LITERAL;}
     | ID ArrayList {$$=mknode(ID,$2,NULL,NULL,yylineno);strcpy($$->type_id,$1);$$->isarr=1;}
     ;
/*用逗号隔开的参数*/
Args: Exp COMMA Args {$$=mknode(ARGS,$1,$3,NULL,yylineno);}
      | Exp {$$=mknode(ARGS,$1,NULL,NULL,yylineno);}
      ;
%%

int main(int argc, char *argv[]){
  yyin=fopen(argv[1],"r");
  if (!yyin) 
    return 0;
  yylineno=1;
  yyparse();
  return 0;
}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}
