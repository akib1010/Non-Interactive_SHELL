shell: shell.c
	clang -Wall -g -Wpedantic -Wextra -Werror shell.c -o shell
clean:
	rm -rf shell shell.o shell.dSYM words touched.txt rand5words.txt first5words.txt nonvoluntaryswitches.txt randsort5words.txt