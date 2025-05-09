
/*
 * Lexer para uma calculadora avançada
 */

/* Reconhecimento de tokens para a calculadora */

%option nowywrap nodefault yylineno

%{
#include "bison-calc.h"
#include "bison-calc.tab.h"
%}

/* Exponente float */
EXP     ([Ee][+-]?[0-9]+)

/* Operadores de caractere único */
"+"     { return yytext[0]; }
"-"     { return yytext[0]; }
"*"     { return yytext[0]; }
"/"     { return yytext[0]; }
"("     { return yytext[0]; }
")"     { return yytext[0]; }
","     { return yytext[0]; }
";"     { return yytext[0]; }

/* Operadores de comparação, todos são tokens CMP */
"<"     { yylval.fn = 1; return CMP; }
">"     { yylval.fn = 2; return CMP; }
"<="    { yylval.fn = 3; return CMP; }
">="    { yylval.fn = 4; return CMP; }
"=="    { yylval.fn = 5; return CMP; }
"!="    { yylval.fn = 6; return CMP; }

/* Palavras-chave */
"if"    { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"while" { return WHILE; }
"do"    { return DO; }
"let"   { return LET; }

/* Funções pré-definidas */
"sqrt"  { yylval.fn = B_sqrt; return FUNC; }
"exp"   { yylval.fn = B_exp; return FUNC; }
"log"   { yylval.fn = B_log; return FUNC; }
"print" { yylval.fn = B_print; return FUNC; }

/* Nomes */
[a-zA-Z_][a-zA-Z_0-9]* {
    yylval.s = lookup(yytext);
    return NAME;
}

/* Números */
[0-9]+(\.[0-9]*)?{EXP}? {
    yylval.d = atof(yytext);
    return NUMBER;
}

/* Ignora espaço em branco */
[ 	]    /* ignora espaço em branco */

/* Ignora continuação de linha */
\
     { printf(">>"); }

/* Nova linha */

       { return EOL; }

/* Caractere não reconhecido */
.        { yyerror("Caractere desconhecido: %c
", *yytext); }
%%
