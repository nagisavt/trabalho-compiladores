# Trabalho Final - Compiladores

Este projeto é uma calculadora avançada desenvolvida para a disciplina de **Compiladores**, utilizando **Flex** (analisador léxico), **Bison** (analisador sintático) e **C**. A linguagem aceita expressões matemáticas, operadores lógicos, controle de fluxo e definição de funções.

## Funcionalidades

- Operações aritméticas: `+`, `-`, `*`, `/`
- Operadores lógicos: `&&` (AND), `||` (OR)
- Comparações: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Comando condicional: `if`, `then`, `else`
- Laços: `while`, `for`
- Definição de funções: `let nome(param) = exp`
- Funções internas: `print`, `sqrt`, `log`, `exp`

## Estrutura de Arquivos

- `calc.l` — analisador léxico (Flex)
- `calc.y` — analisador sintático (Bison)
- `calc_ast.c` — construção e avaliação da AST
- `calc.h` — cabeçalho com definições de estruturas
- `Makefile` — automação da compilação
- Arquivos de entrada para testes:
  - `entradaSimples1.txt`
  - `entradaSimples2.txt`
  - `entradaSimples3.txt`
  - `entradaComplexa1.txt`
  - `entradaComplexa2.txt`
  - `entradaComplexa3.txt`

## Compilação

No terminal (Linux, WSL ou Git Bash):

```bash
make
```

Isso irá gerar o executável `calc`.

## Execução com arquivos de entrada

Execute passando o arquivo de entrada com redirecionamento:

```bash
./calc < entradaSimples1.txt
```

Você pode testar qualquer dos arquivos listados acima.

## Execução interativa

Também é possível usar a calculadora no modo interativo:

```bash
./calc
```

Digite comandos como:

```c
let inc(x) = x + 1;
for(i = 0; i < 10; i = inc(i)) if (i < 5) then print(i); else print(100);
```

## Limpeza

Para remover arquivos gerados:

```bash
make clean
```

---

Desenvolvido por [Seu Nome] — UTFPR

