/*
 * Funções Auxiliares para uma calculadora avançada
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "calc.h"
#include "calc.tab.h"

/* Funções em C para TS */
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

struct symbol symtab[NHASH];

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
    yyerror("overflow na tabela de símbolos\n");
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

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el, struct ast *init, struct ast *inc)
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
    a->init = init; /* Adiocionado para leitura do FOR (valor de inicio) */
    a->inc = inc; /* Adicionado para leitura do FOR (valor de incremento) */
    return (struct ast *)a;
}

/* Libera uma árvore de AST */
void treefree(struct ast *a)
{
    switch (a->nodetype) {
    /* duas subárvores */
    case '+': 
    case '-': 
    case '*':
    case '/':
    case '1': case '2': case '3': case '4': case '5': case '6':
    case 'A': case 'O':
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
    case 'I': case 'W': case 'R':
        free(((struct flow *)a)->cond);
        if (((struct flow *)a)->tl) treefree(((struct flow *)a)->tl);
        if (((struct flow *)a)->el) treefree(((struct flow *)a)->el);
        if (((struct flow *)a)->init) treefree(((struct flow *)a)->init);
        if (((struct flow *)a)->inc) treefree(((struct flow *)a)->inc);
        break;
    default:
        printf("erro interno: free bad node %c\n", a->nodetype);
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

/* Etapa principal: avaliação de expressões, comandos, funções... */

static double callbuiltin(struct fncall *);
static double calluser(struct ufncall *);

double eval(struct ast *a)
{
    double v;
    if (!a) {
        yyerror("erro interno: null eval");
        return 0.0;
    }

    switch (a->nodetype) {
    /* Constante */
    case 'K': v = ((struct numval *)a)->number; break;

    /* Referência de nome */
    case 'N': v = ((struct symref *)a)->s->value; break;

    /* Atribuição */
    case '=': v = ((struct symasgn *)a)->s->value = eval(((struct symasgn *)a)->v); break;

    /* Expressões */
    case '+': v = eval(a->l) + eval(a->r); break;
    case '-': v = eval(a->l) - eval(a->r); break;
    case '*': v = eval(a->l) * eval(a->r); break;
    case '/': v = eval(a->l) / eval(a->r); break;

    /* Comparações */
    case '1': v = (eval(a->l) > eval(a->r)) ? 1 : 0; break;
    case '2': v = (eval(a->l) < eval(a->r)) ? 1 : 0; break;
    case '3': v = (eval(a->l) != eval(a->r)) ? 1 : 0; break;
    case '4': v = (eval(a->l) == eval(a->r)) ? 1 : 0; break;
    case '5': v = (eval(a->l) >= eval(a->r)) ? 1 : 0; break;
    case '6': v = (eval(a->l) <= eval(a->r)) ? 1 : 0; break;
    case 'A': v = (eval(a->l) != 0 && eval(a->r) != 0) ? 1 : 0; break;
    case 'O': v = (eval(a->l) != 0 || eval(a->r) != 0) ? 1 : 0; break;

    /* Controle de fluxo */
    case 'I': /* if/then/else */
        if (eval(((struct flow *)a)->cond) != 0) { /* verifica condição */
            if (((struct flow *)a)->tl) /* ramo verdadeiro */
                v = eval(((struct flow *)a)->tl);
            else
                v = 0.0; /* valor default */
        } else {
            if (((struct flow *)a)->el) /* ramo falso */
                v = eval(((struct flow *)a)->el);
            else
                v = 0.0; /* valor default */
        }
        break;

    case 'W': /* while/do */
        v = 0.0; /* valor default */

        if (((struct flow *)a)->tl) { /* verifica lista de comandos */
            while (eval(((struct flow *)a)->cond) != 0) { /* avalia condição */
                v = eval(((struct flow *)a)->tl); /* executa comandos */
            }
        }
        break;
      
    case 'R': { /* FOR */
      v = 0.0; /* valor default */
      eval(((struct flow *)a)->init); /* inicializa */
      
      if (((struct flow *)a)->tl) { /* verifica lista de comandos */ 
        while (eval(((struct flow *)a)->cond) != 0) { /* avalaia condicao */
          v = eval(((struct flow *)a)->tl);     /* executa comando */
          eval(((struct flow *)a)->inc);       /* incrementa */
          }
      }

    break;
    }

    /* Lista de comandos */
    case 'L': eval(a->l); v = eval(a->r); break;

    case 'F': v = callbuiltin((struct fncall *)a); break;

    case 'C': v = calluser((struct ufncall *)a); break;

    default:
        printf("erro interno: bad node %c\n", a->nodetype);
    }
    return v;
}

static double callbuiltin(struct fncall *f)
{
    enum bifs functype = f->functype;
    double v = eval(f->l);

    switch (functype) {
    case B_sqrt: 
      return sqrt(v);
    case B_exp: 
      return exp(v);
    case B_log: 
      return log(v);
    case B_print:
        printf("= %4.4g\n", v);
        return v;
    default:
        yyerror("Função pré-definida %d desconhecida\n", functype);
        return 0.0;
    }
}

/* Função definida por usuário */
void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
    if (name->syms) symlistfree(name->syms);
    if (name->func) treefree(name->func);
    name->syms = syms;
    name->func = func;
}

static double calluser(struct ufncall *f)
{
    struct symbol *fn = f->s; /* nome da função */
    struct symlist *sl;      /* argumentos originais da função */
    struct ast *args = f->l; /* argumentos usados na função */
    double *oldval, *newval; /* salvar valores de argumento */
    double v;
    int nargs;
    int i;

    if (!fn->func) {
        yyerror("chamada para função %s indefinida", fn->name);
        return 0;
    }

    /* Contar argumentos */
    sl = fn->syms;
    for (nargs = 0; sl; sl = sl->next)
        nargs++;

    /* Preparar o espaço para salvar argumentos */
    oldval = (double *)malloc(nargs * sizeof(double));
    newval = (double *)malloc(nargs * sizeof(double));

    if (!oldval || !newval) {
        yyerror("Sem espaço em %s", fn->name);
        return 0.0;
    }

    /* Avaliação de argumentos */
    sl = fn->syms;
    for (i = 0; i < nargs; i++) {
        if (!args) {
            yyerror("Poucos argumentos na chamada da função %s", fn->name);
            free(oldval); free(newval);
            return 0.0;
        }

        if (args->nodetype == 'L') { /* se é uma lista de nós */
            newval[i] = eval(args->l);
            args = args->r;
        } else { /* se é o fim da lista */
            newval[i] = eval(args);
            args = NULL;
        }
    }

    /* Salvar valores originais dos argumentos e atribuir novos valores */
    sl = fn->syms;
    for (i = 0; i < nargs; i++) {
        struct symbol *s = sl->sym;
        oldval[i] = s->value;
        s->value = newval[i];
        sl = sl->next;
    }

    free(newval);

    /* Avaliação da função */
    v = eval(fn->func);

    /* Recolocar os valores originais */
    sl = fn->syms;
    for (i = 0; i < nargs; i++) {
        struct symbol *s = sl->sym;
        s->value = oldval[i];
        sl = sl->next;
    }

    free(oldval);
    return v;
}

void yyerror(char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

extern FILE *yyin;

int main(int argc, char **argv) {
   
    if (argc > 1) {
        FILE *inputFile = fopen(argv[1], "r"); 
        if (!inputFile) {
            perror(argv[1]); 
            return 1;        
        }

        yyin = inputFile; 
        printf("> "); 
        yyparse(); 
        fclose(inputFile);
        
    } else {
        printf("Usage: %s <input-file>\n", argv[0]); 
        return 1; 
    }

    return 0;
}
