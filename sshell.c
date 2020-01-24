#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h> 

#define CMDLINE_MAX 512
#define ARG_MAX 17
#define CHAR_MAX 32
typedef struct process Process;
typedef struct task Task;

struct process {
	char* args[ARG_MAX];
	char opp;
	char opp_err;
};

struct task {
	Process processes[ARG_MAX];
	int num_pros;
	int num_pipes;
};

struct StackNode {
	char* nodeValue;
	struct StackNode *nextNode;
};

struct StackNode *topPtr = (struct StackNode*)NULL;

int CALL_CD(const char* path, const int retval) {
	if (!strcmp(path, "..")) {
		char* cwd = malloc(CMDLINE_MAX * sizeof(char));
		getcwd(cwd, CMDLINE_MAX);
		int slash_index;
		for (int k = 0; k < ((int)strlen(cwd)) - 1; k++) {
			if (cwd[k] == '/') {
				slash_index = k;
			}
		}
		char* newPath = malloc(CHAR_MAX * sizeof(char));
		newPath = strncpy(newPath, cwd, slash_index);
		chdir(newPath);
	}
	else {
		if (chdir(path) < 0) {
			fprintf(stderr, "Error: no such directory\n");
			return(-1);
		}
	}
	return(retval);
}

int CALL_PWD(const int retval) {
	char buf[CMDLINE_MAX];
	printf("%s\n", getcwd(buf, CMDLINE_MAX));
	return(retval);
}

void CALL_EXIT() {
	fprintf(stderr, "Bye...\n");
	fprintf(stdout, "+ completed 'exit' [0]\n");
	exit(0);
}

bool isEmpty() {
	bool stackEmpty;
	if (!topPtr) {
		stackEmpty = true;
	}
	else {
		stackEmpty = false;
	}
	return stackEmpty;
}

void PushStack(char* strings) {
	struct StackNode *newNode = malloc(sizeof(struct StackNode));
	newNode->nodeValue = malloc(CHAR_MAX * sizeof(char));
	memcpy(newNode->nodeValue, strings, CHAR_MAX);
	newNode->nextNode = topPtr;
	topPtr = newNode;
	if (isEmpty()) {
		topPtr = newNode;
		newNode->nextNode = NULL;
	}
	else {
		newNode->nextNode = topPtr->nextNode;
		topPtr = newNode;
	}
}

void PopStack() {
	struct StackNode *tempNode;
	char* strings;
	if (isEmpty()) {
		fprintf(stderr, "Error: directory stack empty\n");
	}
	else {
		strings = topPtr->nodeValue;
		tempNode = topPtr->nextNode;
		CALL_CD(strings, 0);
		free(topPtr);
		topPtr = tempNode;
	}
}

void display() {
	struct StackNode *pointer = NULL;
	pointer = topPtr;
	char buf[CMDLINE_MAX];
	getcwd(buf, CMDLINE_MAX);
	printf("%s\n", buf);
	if (isEmpty())
	{
		fprintf(stderr, "\nError: directory stack empty");
	}
	while (pointer != NULL) {
		printf("%s\n", pointer->nodeValue);
		pointer = pointer->nextNode;
	}
}

void PARSE_CMD(const char* cmd, Task* t) {
	//create process to store the individual commands
	Process p;
	//create temp argument to store individual words
	char temp_arg[CHAR_MAX];
	memset(temp_arg, 0, CHAR_MAX);
	//counts the number of processes in the Task
	int process_counter = 0;
	//counts number of arguements in each process
	int arg_counter = 0;
	//character counter so that the for-loop matches with the arguments
	int char_counter = 0;
	for (int i = 0; i < CMDLINE_MAX; i++) {
		//check for spaces
		if (cmd[i] == ' ') {
			if (cmd[i - 1] == ' ' || char_counter == 0) {
				continue;
			}
			//if space follows a word, store the word in p
			else {
				p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
				memcpy(p.args[arg_counter], temp_arg, CHAR_MAX);
				memset(temp_arg, 0, CHAR_MAX);
				arg_counter += 1;
				char_counter = 0;
				continue;
			}
		}
		else if (cmd[i] == '|' || cmd[i] == '>') {
			p.opp = cmd[i];
			if (cmd[i] == '|') {
				t->num_pipes += 1;
			}
			if (cmd[i + 1] == '&') {
				p.opp_err = cmd[i + 1];
			}
			if (cmd[i - 1] != ' ') {
				p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char));
				memcpy(p.args[arg_counter], temp_arg, CHAR_MAX);
				memset(temp_arg, 0, CHAR_MAX);
				arg_counter += 1;
				char_counter = 0;
			}
			p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char*));
			p.args[arg_counter] = (char*)NULL;
			t->processes[process_counter] = p;
			t->num_pros += 1;
			process_counter += 1;
			arg_counter = 0;
			continue;
		}
		else if (cmd[i] == '\0' || cmd[i] == '\n') {
			if (cmd[i - 1] != ' ') {
				p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char*));
				memcpy(p.args[arg_counter], temp_arg, CHAR_MAX);
				memset(temp_arg, 0, CHAR_MAX);
				arg_counter += 1;
				char_counter = 0;
			}
			p.args[arg_counter] = malloc(CHAR_MAX * sizeof(char*));
			p.args[arg_counter] = (char*)NULL;
			t->processes[process_counter] = p;
			t->num_pros += 1;
			process_counter += 1;
			arg_counter = 0;
			break;
		}
		else {
			temp_arg[char_counter] = cmd[i];
			char_counter += 1;
		}
	}
}

int PIPE_CMD(Task* t) {
	pid_t _pid;
	int fd_in = (-1);
	int fd_in_prev = (-1);
	int fd[2];
	for (int p = 0; p < t->num_pros; p++) {
		pipe(fd);
		_pid = fork();
		if (_pid > 0) {
			// Parent
			if (t->processes[p].opp == '|' && fd_in_prev != -1) {
				fd_in = fd[0];
				close(fd[1]);
			}
			else if (t->processes[p].opp == '|') {
				fd_in = fd[0];
				close(fd[1]);
			}
			else if (fd_in_prev != -1) {
				fd_in = (-1);
				close(fd[0]);
				close(fd[1]);
			}
			fd_in_prev = fd_in;
			fd_in = (-1);
			int status;
			waitpid(_pid, &status, 0);
		}
		else if (_pid == 0) {
			// Child
			if (t->processes[p].opp == '|' && fd_in_prev != -1) {
				dup2(fd_in_prev, STDIN_FILENO);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
			}
			else if (t->processes[p].opp == '|') {
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
			}
			else if (fd_in_prev != -1) {
				dup2(fd_in_prev, STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);
			}
			execvp(t->processes[p].args[0], t->processes[p].args);
			perror("bad exec");
			return(-1);
		}
		else {
			perror("bad fork");
			return(-1);
		}
	}
	return 0;
}

int main(void) {
	char cmd[CMDLINE_MAX];
	while (1) {
		Task Run;
		Run.num_pipes = 0;
		Run.num_pros = 0;
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
		PARSE_CMD(cmd, &Run);
		char* cmd = Run.processes[0].args[0];
		if (!strcmp(cmd, "exit")) {
			CALL_EXIT();
		}
		else if (!strcmp(cmd, "pwd")) {
			retval = CALL_PWD(retval);
			fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
		}
		else if (!strcmp(cmd, "cd")) {
			retval = CALL_CD(Run.processes[0].args[1], retval);
			fprintf(stdout, "+ completed '%s %s' [%d]\n", cmd, Run.processes[0].args[1], retval);
		}
		else if (!strcmp(cmd, "pushd")) {
			char buf[CMDLINE_MAX];
			getcwd(buf, CMDLINE_MAX);
			PushStack(buf);
			fprintf(stdout, "+ completed '%s' [0]\n", cmd);
		}
		else if (!strcmp(cmd, "popd")) {
			PopStack();
			fprintf(stdout, "+ completed '%s' [0]\n", cmd);
		}
		else if (!strcmp(cmd, "dirs")) {
			display();
			fprintf(stdout, "+ completed '%s' [0]\n", cmd);
		}
		else {
			retval = PIPE_CMD(&Run);
			if (retval != 0) {
				fprintf(stderr, "Error: command not found\n");
			}
			fprintf(stdout, "+ completed '%s' [%d]\n", cmd, retval);
		}
	}
	return EXIT_SUCCESS;
}