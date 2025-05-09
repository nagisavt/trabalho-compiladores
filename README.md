# Trabalho de Compiladores

Implementação de uma calculadora com analisador léxico (Flex) e sintático (Bison), suportando operadores lógicos `&&`, `||` e estrutura de repetição `for`.

## Arquivos

- `calc.l`: analisador léxico
- `calc.y`: analisador sintático
- `calc_ast.c`: implementação da árvore sintática abstrata (AST)

## Compilação

```bash
bison -d calc.y
flex calc.l
gcc -o calc calc_ast.c calc.tab.c lex.yy.c -lm
```

## Execução

```bash
./calc
```

Digite expressões diretamente no terminal. Exemplo:

```
for(i = 0; i < 3; i = i + 1) i && 1
```

---

Desenvolvido para a disciplina de **Compiladores**.
