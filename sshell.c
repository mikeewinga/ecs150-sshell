#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

int SYS_CALL(char* command) {	
	pid_t _pID; 
	_pID = fork();
	if (_pID == 0) {
		char* args[] = {command, (char*)NULL};
		execvp(args[0], args);
		return 0;
	}
	else if (_pID > 0) {
		int status;
		waitpid(_pID, &status, 0);
		return (status);
	}
	else {
		perror("bad fork");
		return (-1);
	}
	return 0;
}

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
			fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
                        break;
                }
		
		if (!strcmp(cmd, "pwd")) {
			retval = SYS_CALL("ps -p PID -o comm=");
			//fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
                }


                /* Regular command */
                retval = SYS_CALL(cmd);
                fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
        }

        return EXIT_SUCCESS;
}
