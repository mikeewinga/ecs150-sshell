#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

void parse_command(char* cmd, char** args) {
	char temp_cmd[32];
	memset(temp_cmd, 0, 32);
	int arg_counter = 0;
	int char_counter = 0;
	for (int i = 0; i < 512; i++) {
		if (cmd[i] == ' ') {
			if (cmd[i - 1] != ' ') {
				args[arg_counter] = (char*)malloc(32 * sizeof(char));
				memcpy(args[arg_counter], temp_cmd, 32);
				arg_counter += 1;
				char_counter = 0;
				memset(temp_cmd, 0, 32);
				continue;
			}
			else {
				continue;
			}
		}
		else if (cmd[i] == '\0' || cmd[i] == '\n') {
			args[arg_counter] = (char*)malloc(32 * sizeof(char));
			memcpy(args[arg_counter], temp_cmd, 32);
			arg_counter += 1;
			char_counter = 0;
			memset(temp_cmd, 0, 32);
			break;
		}
		else {
			temp_cmd[char_counter] = cmd[i];
			char_counter += 1;
		}
	}
	args[arg_counter] =(char*)NULL;
	arg_counter = 0;
}



int SYS_CALL(char** args) {	
	pid_t _pid; 
	_pid = fork();	
	if (_pid == 0) {
		execvp(args[0], args);
		exit(-1);
	}
	else if (_pid > 0) {
		int status;
		waitpid(_pid, &status, 0);
		return(status);
	}
	else {
		perror("bad fork");
		exit(-1);
	}
	return 0;
}


int main(void)
{
	char cmd[CMDLINE_MAX];
	char* commands[16];
	//free(commands);

	while (1) {
		char *nl;
		int retval=0;

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

		parse_command(cmd, commands);

		/* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl) {
			*nl = '\0';
		}


		/* Builtin command */
		if (!strcmp(cmd, "exit")) {
			fprintf(stderr, "Bye...\n");
			fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
			break;
		}

		if (!strcmp(cmd, "pwd")) {
			char buf[CMDLINE_MAX];
			printf("%s\n", getcwd(buf, CMDLINE_MAX));
			fprintf(stdout, "+ completed '%s' [0]\n", cmd);
			continue;
		}

		if (!strcmp(commands[0], "cd")) {
			char* path = commands[1];
			if(!strcmp(path, "..")) {
				char* cwd = getcwd(cwd, CMDLINE_MAX);
				int slash_index;
				for(int k=0; k<((int)strlen(cwd))-1; k++) {
					if (cwd[k] == '/') {
						slash_index = k;
					}
				}
				char* newPath = malloc(32*sizeof(char));
				newPath = strncpy(newPath, cwd, slash_index);
				chdir(newPath);			
			}
			else {
				chdir(path);
			}
			fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
			continue;
		}


		/* Regular command */
		retval = SYS_CALL(commands);
		if(retval != 0){
			fprintf(stderr, "Error: command not found\n");
		}
		fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
	}

	return EXIT_SUCCESS;
}