
/*
 * Declarações para uma calculadora avançada
 */

/* Interface com o lexer */
extern int yylineno;
void yyerror(char *s, ...);

/* Tabela de símbolos */
struct symbol {           /* um nome de variável */
    char *name;
    double value;
    struct ast *func;     /* stmt para função */
    struct symlist *syms; /* lista de argumentos */
};

/* Tabela de símbolos de tamanho fixo */
#define NHASH 9997
extern struct symbol symtab[NHASH];

struct symbol *lookup(char *);

/* Lista de símbolos, para uma lista de argumentos */
struct symlist {
    struct symbol *sym;
    struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

/* Tipos de nós */
/* 
    + - * / 
    0-7 operadores de comparação, 04 igual, 02 menor que, 01 maior que
    L expressão ou lista de comandos
    I comando IF
    W comando WHILE
    N symbol de referência
    = atribuição
    S lista de símbolos
    F chamada de função pré-definida
    C chamada de função definida pelo usuário
*/

/* Funções pré-definidas */
enum bifs { /* Funções pré-definidas */
    B_sqrt = 1,
    B_exp,
    B_log,
    B_print
};

/* Nós na AST */
/* Todos têm o "nodetype" inicial em comum */
struct ast {
    int nodetype;
    struct ast *l;
    struct ast *r;
};

struct fncall { /* Funções pré-definidas */
    int nodetype;      /* Tipo F */
    struct ast *l;
    enum bifs functype;
};

struct ufncall { /* Funções do usuário */
    int nodetype;      /* Tipo C */
    struct ast *l;     /* Lista de argumentos */
    struct symbol *s;
};

struct flow { /* Tipo I ou W */
    int nodetype;
    struct ast *cond;  /* Condição */
    struct ast *tl;    /* Ramo "then" ou lista "do" */
    struct ast *el;    /* Ramo opcional "else" */
    struct ast *init;  /* Inicialização (somente para for) */
    struct ast *inc;   /* Incremento (somente para for) */
    
};

struct numval { /* Tipo K */
    int nodetype;
    double number;
};

struct symref { /* Tipo N */
    int nodetype;
    struct symbol *s;
};

struct symasgn { /* Tipo = */
    int nodetype;
    struct symbol *s;
    struct ast *v;     /* Valor a ser atribuído */
};

/* Construção de uma AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(double d);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el, struct ast *init, struct ast *inc);

/* Definição de uma função */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* Avaliação de uma AST */
double eval(struct ast *);

/* Deletar e liberar uma AST */
void treefree(struct ast *);
