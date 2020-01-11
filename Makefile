FLAGS=-Wall -Wextra -Werror

all: sshell test

test: sshellTEST.c
	gcc $(FLAGS) sshellTEST.c -o test 

sshell: sshell.c
	gcc $(FLAGS) sshell.c -o sshell 

rmvfiles = sshell test

clean:
	rm -rf $(rmvfiles) *.o
	rm -rf *.DS_Store
