shell: shell.c
	clang -Wall -g -Wpedantic -Wextra -Werror shell.c -o shell
clean:
	rm -rf shell shell.o shell.dSYM