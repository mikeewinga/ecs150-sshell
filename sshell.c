#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define ARG_MAX 17
#define CHAR_MAX 32
typedef struct process Process;
typedef struct task Task;

struct process {
	char* args[ARG_MAX];
	char* next_arg[ARG_MAX];
	char opp;
	char opp_err;
	int pid;
	int fd[2];
	int complete;
};

struct task {
	Process processes[ARG_MAX];
};

void parse_command(const char* cmd, Task* t) {
	Process p;
	char temp_arg[CHAR_MAX];
	memset(temp_arg, 0, CHAR_MAX);
	int process_counter = 0;
	int arg_counter = 0;
	int char_counter = 0;
	for (int i = 0; i < CMDLINE_MAX; i++) {
		if (cmd[i] == ' ') {
			if (cmd[i - 1] == ' ' || char_counter == 0) {
				continue;
			}
			else {
				p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
				memcpy(p.args[arg_counter], temp_arg, CHAR_MAX);
				memset(temp_arg, 0, CHAR_MAX);
				arg_counter += 1;
				char_counter = 0;
				continue;
			}
		}
		else if (cmd[i] == '|' || cmd[i] == '>' || cmd[i] == '&') {
			p.opp = cmd[i];
			if (cmd[i - 1] != ' ') {
				p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
				memcpy(p.args[arg_counter], temp_arg, CHAR_MAX);
			}
			if (cmd[i] == '&') {
				i += 1;
				p.opp_err = cmd[i];
			}
			arg_counter += 1;
			p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
			p.args[arg_counter] = (char*)NULL;
			memset(temp_arg, 0, CHAR_MAX);
			t->processes[process_counter] = p;
			process_counter += 1;
			arg_counter = 0;
			char_counter = 0;
			continue;
		}
		else if (cmd[i] == '\0') {
			if (cmd[i - 1] != ' ') {
				p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
				memcpy(p.args[arg_counter], temp_arg, CHAR_MAX);
			}
			arg_counter += 1;
			p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
			p.args[arg_counter] = (char*)NULL;
			memset(temp_arg, 0, CHAR_MAX);
			t->processes[process_counter] = p;
			process_counter += 1;
			arg_counter = 0;
			char_counter = 0;
			break;
		}
		else {
			temp_arg[char_counter] = cmd[i];
			char_counter += 1;
		}
	}
}


int SYS_CALL(Task* t) {
	pid_t _pid;
	_pid = fork();
	if (_pid == 0) {
		execvp(t->processes[0].args[0], t->processes[0].args);
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

	while (1) {
		Task Run;
		char *nl;
		int retval = 0;

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
		if (nl) {
			*nl = '\0';
		}

		parse_command(cmd, &Run);

		/*
			for(int m=0;m<3;m++){
			  printf("Process %d:\n", m);
			  for(int n=0;n<5;n++){
				  printf("%s\n", Run.processes[m].args[n]);
			  }
			}
		*/

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

		if (!strcmp(Run.processes[0].args[0], "cd")) {
			char* path = Run.processes[0].args[2];
			if (!strcmp(path, "..")) {
				char* cwd = getcwd(cwd, CMDLINE_MAX);
				int slash_index;
				for (int k = 0; k < ((int)strlen(cwd)) - 1; k++) {
					if (cwd[k] == '/') {
						slash_index = k;
					}
				}
				char* newPath = malloc(32 * sizeof(char));
				newPath = strncpy(newPath, cwd, slash_index);
				chdir(newPath);
			}
			else {
				if (chdir(path) < 0) {
					fprintf(stderr, "Error: no such directory\n");
					continue;
				}
			}
			fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
			continue;
		}


		/* Regular command */
		retval = SYS_CALL(&Run);
		if (retval != 0) {
			fprintf(stderr, "Error: command not found\n");
		}
		fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
	}

	return EXIT_SUCCESS;
}