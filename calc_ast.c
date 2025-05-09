
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "bison-calc.h"

/* Funções em C para AST */

/* Função hashing */
static unsigned symhash(char *sym)
{
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++) {
        hash = hash * 9 ^ c;
    }

    return hash;
}

struct symbol *lookup(char *sym)
{
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH;

    while (--scount >= 0) {
        if (sp->name && !strcasecmp(sp->name, sym))
            return sp;

        if (!sp->name) { /* nova entrada na TS */
            sp->name = strdup(sym);
            sp->value = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }

        if (++sp >= symtab + NHASH)
            sp = symtab; /* tenta a próxima entrada */
    }
    yyerror("overflow na tabela de símbolos
");
    abort(); /* tabela está cheia */
}

struct ast *newast(int nodetype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newnum(double d)
{
    struct numval *a = malloc(sizeof(struct numval));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = 'K';
    a->number = d;
    return (struct ast *)a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

struct ast *newfunc(int functype, struct ast *l)
{
    struct fncall *a = malloc(sizeof(struct fncall));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    return (struct ast *)a;
}

struct ast *newcall(struct symbol *s, struct ast *l)
{
    struct ufncall *a = malloc(sizeof(struct ufncall));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = 'C';
    a->l = l;
    a->s = s;
    return (struct ast *)a;
}

struct ast *newref(struct symbol *s)
{
    struct symref *a = malloc(sizeof(struct symref));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = 'N';
    a->s = s;
    return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v)
{
    struct symasgn *a = malloc(sizeof(struct symasgn));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
    struct flow *a = malloc(sizeof(struct flow));

    if (!a) {
        yyerror("sem espaço");
        exit(0);
    }
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;
    return (struct ast *)a;
}

/* Libera uma árvore de AST */
void treefree(struct ast *a)
{
    switch (a->nodetype) {
    /* duas subárvores */
    case '+': case '-': case '*': case '/':
    case '1': case '2': case '3': case '4': case '5': case '6':
    case 'L':
        treefree(a->r);
    /* uma subárvore */
    case 'C': case 'F':
        treefree(a->l);
    /* sem subárvores */
    case 'K': case 'N':
        break;
    case '=':
        free(((struct symasgn *)a)->v);
        break;
    /* acima de 3 subárvores */
    case 'I': case 'W':
        free(((struct flow *)a)->cond);
        if (((struct flow *)a)->tl)
            treefree(((struct flow *)a)->tl);
        if (((struct flow *)a)->el)
            treefree(((struct flow *)a)->el);
        break;
    default:
        printf("erro interno: free bad node %c
", a->nodetype);
    }
    free(a);
}

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
    struct symlist *sl = malloc(sizeof(struct symlist));

    if (!sl) {
        yyerror("sem espaço");
        exit(0);
    }
    sl->sym = sym;
    sl->next = next;
    return sl;
}

/* Libera uma lista de símbolos */
void symlistfree(struct symlist *sl)
{
    struct symlist *nsl;

    while (sl) {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}
