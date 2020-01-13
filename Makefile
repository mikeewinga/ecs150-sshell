FLAGS = -Wall -Wextra -Werror

all: sshell

sshell: sshell.c
	gcc $(FLAGS) sshell.c -o sshell 

rmvfiles = sshell

clean:
	rm -rf $(rmvfiles) *.o
	rm -rf *.DS_Store
	rm -rf core
