%{
    #include "./parser.tab.h" 
    #include "def.h"
    int yycolumn = 1;
    #define YY_USER_ACTION yylloc.first_line=yylloc.last_line=yylineno; yylloc.first_column=yycolumn; yylloc.last_column=yycolumn+yyleng-1; yycolumn+=yyleng;
    typedef union{
        int type_int;
        float type_float;
        char type_char;
        char type_id[32];
        struct node *ptr;
    } YYLVAL;
    #define YYSTYPE YYLVAL
%}

id              [A-Za-z_][A-Za-z0-9_]*
wrongid         [0-9][A-Za-z0-9_]*

/*通用字符名*/
UCN (\\u[0-9a-fA-F]{4}|\\U[0-9a-fA-F]{8})
char_literal \'([^'\\]|\\['"?\\abfnrtv]|\\[0-7]{1,3}|\\[Xx][0-9A-Fa-f]+|{UCN})+\'

digit           [0-9]
hex_digit       {digit}|[a-fA-F]
oct_digit       [0-7]
int_literal     {digit}+|0x{hex_digit}+|0{oct_digit}+

relop           "<"|">"|"<="|">="|"=="|"!="

comment         ("/*"[/]*([^*/][/]*|[*])*"*/")|("//".*)

%%
{int_literal}   {printf("(INTEGER,  %s)\n", yytext);yylval.type_int=atoi(yytext);return INT_LITERAL;}
{char_literal}  {printf("(CHAR,     %s)\n", yytext);yylval.type_char[0]=yytext[1];if(yytext[2] != '\'') {yylval.type_char[1]=yytext[2];yylval.type_char[2]='\0';} return CHAR_LITERAL;}

"int"           {printf("(INT,      %s)\n", yytext);strcpy(yylval.type_id,yytext);return TYPE;}
"char"          {printf("(CHAR,     %s)\n", yytext);strcpy(yylval.type_id,yytext);return TYPE;}
"break"         {printf("(BREAK,    %s)\n", yytext);return BREAK;}
"for"           {printf("(FOR,      %s)\n", yytext);return FOR;}
"return"        {printf("(RETURN,   %s)\n", yytext);return RETURN;}
"if"            {printf("(IF,       %s)\n", yytext);return IF;}
"else"          {printf("(ELSE,     %s)\n", yytext);return ELSE;}
"while"         {printf("(WHILE,    %s)\n", yytext);return WHILE;}
"continue"      {printf("(CONTINUE, %s)\n", yytext);return CONTINUE;}

{wrongid}       {printf("Error identifier that starts with a number: %s\n", yytext);}
{id}            {printf("(ID,       %s)\n", yytext);strcpy(yylval.type_id,yytext);return ID;}
{comment}       {printf("This is a comment.\n");}
";"             {printf("(SEMI,     %s)\n", yytext);return SEMI;}
","             {printf("(COMMA,    %s)\n", yytext);return COMMA;}
{relop}         {printf("(RELOP,    %s)\n", yytext);strcpy(yylval.type_id,yytext);return RELOP;}
"+="            {printf("(ADDASS,   %s)\n", yytext);return ADDASS;}
"-="            {printf("(MINUSASS, %s)\n", yytext);return MINUSASS;}
"="             {printf("(ASSIGNOP, %s)\n", yytext);return ASSIGNOP;}
"++"            {printf("(SELFADD,  %s)\n", yytext);return SELFADD;}
"--"            {printf("(SELFMINUS,%s)\n", yytext);return SELFMINUS;}
"+"             {printf("(PLUS,     %s)\n", yytext);return PLUS;}
"-"             {printf("(MINUS,    %s)\n", yytext);return MINUS;}
"*"             {printf("(MUL,      %s)\n", yytext);return MUL;}
"/"             {printf("(DIV,      %s)\n", yytext);return DIV;}
"&&"            {printf("(AND,      %s)\n", yytext);return AND;}
"||"            {printf("(OR,       %s)\n", yytext);return OR;}
"!"             {printf("(NOT,      %s)\n", yytext);return NOT;}
"("             {printf("(LP,       %s)\n", yytext);return LP;}
")"             {printf("(RP,       %s)\n", yytext);return RP;}
"{"             {printf("(LC,       %s)\n", yytext);return LC;}
"}"             {printf("(RC,       %s)\n", yytext);return RC;}
"["             {printf("(LB,       %s)\n", yytext);return LB;}
"]"             {printf("(RB,       %s)\n", yytext);return RB;}
\n              {yycolumn=1;yylineno++;}
[ \r\t]         {}
.               {printf("Error: Mysterious character \"%s\"\n\t\n", yytext);}
%%
int yywrap()
{
return 1;
}