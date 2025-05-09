
calc: calc.l calc.y calc.h calc_ast.c
	@bison -d calc.y
	@flex -o calc.lex.c calc.l
	@gcc -o $@ calc.tab.c calc.lex.c calc_ast.c -lm
	@echo "Parser da Calculadora com Cmds, Funções, ... está pronto!"
