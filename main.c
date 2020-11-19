#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int bg = 0;

struct node {
	char *word;
	struct node *next;
};
typedef struct node Node;
typedef Node *List;

void shell_print() {
	printf(">>>");
	fflush(stdout);
}

void pushback(List *headptr, char *buffer) {
	List p = malloc(sizeof(Node));
	p -> word = buffer;
	p -> next = NULL;
	List t = *headptr;
	if(*headptr == NULL) {
		*headptr = p;
		return;
	}
	while(t -> next != NULL)
		t = t -> next;
	t -> next = p;
}

void printlist (List head) {
	List t = head;
	while(t -> next != NULL)
	{
		printf ("%s ", t->word);
		t = t -> next;
	}
	printf ("\n");
}

List buildlist(char *buffer, int wordcounter) {
	List head = NULL;
	int pos = 0;
	for (int i = 0; i <= wordcounter; i++) {
		pushback (&head, &buffer[pos]);
		pos += (strlen (&buffer[pos]) + 1);
	}
	return head;
}

void clearlist(List head) {
	List t;
	while(head != NULL) {
		t = head;
		head = head -> next;
		free(t);
	}
}
int filter(char **buffer) {
	char *old_buffer = *buffer;
	int new_buf_size = 16;
	char *new_buffer = malloc(new_buf_size * sizeof(char));

	int i = 0;
	int j = 0;
	int wordcounter = 0;

	int flag1 = 0;
	int flag2 = 0;
	while (old_buffer[i] != '\0') {
		//	Переполнение
		if (j > new_buf_size) {
			new_buf_size *= 2;
			new_buffer = realloc(new_buffer, new_buf_size);
		}

		switch (old_buffer[i]) {
		case '"':
			if(flag1 == 0) {
				flag1 = 1;
				i++;
				break;
			}
			else {
				flag1 = 0;
				flag2 = 1;
				i++;
				continue;
			}
		case ' ':
			if(flag1 == 1)
				break;
			if(flag2 == 1) {
				new_buffer[j++] = '\0';
				wordcounter++;
			}
			flag2 = 0;
			break;
		default:
			flag2 = 1;
			break;
		}
		if(flag1 == 1) {
			new_buffer[j++] = old_buffer[i++];
			continue;
		}
		if(flag1 == 0 && flag2 == 1) {
			new_buffer[j++] = old_buffer[i++];
			continue;
		}
		if(flag1 == 0 && flag2 == 0) {
			i++;
			continue;
		}
	}
	if(old_buffer[--i] == ' ') {
		wordcounter--;
		i++;
	}
	new_buffer[j] = '\0';
	*buffer = new_buffer;
	free(old_buffer);
	return ++wordcounter;
}

char **build_args(List head) {
	List t = head;
	int bufsize = 8, pos = 0;
	char **args = malloc(bufsize * sizeof(char*));
	if (!args)
	{
		fprintf(stderr, "shell: allocation error\n");
		exit(1);
	}
	while(t -> next != NULL) {
		args[pos] = t -> word;
		pos++;
		if (pos >= bufsize) {
			bufsize *= 2;
			args = realloc(args, bufsize * sizeof(char*));
			if (!args) {
				fprintf(stderr, "shell: allocation error\n");
				exit(1);
			}
		}
		t = t -> next;
	}
	args[pos] = NULL;
	return args;
}

int shell_launch(char **args) {
	int wstatus;
	pid_t pid = fork();
	if(!bg) {
		if(pid > 0)
			waitpid(pid, &wstatus, 0);
		else
		{
			if(execvp(args[0], args) < 0) {
				perror("exec failed");
				return 1;
			}
		}
	}
	else
		if(pid == 0) {
			bg = 0;
			if(execvp(args[0], args) < 0) {
				perror("exec failed");
				return 1;
			}
		}
	return 0;
}

int shell_cd(char **args)
{
	char *home = getenv("HOME");
	if (args[1] == NULL || strcmp(args[1] , "~") == 0) {
		chdir(home);
	} else {
		if(chdir(args[1]) != 0) {
			perror("shell");
		}
	}
	return 0;
}

int shell_execute(char **args) {
	if (strcmp(args[0], "cd") == 0) {
		return shell_cd(args);
	}

	return shell_launch(args);
}

int bufsize = 16, pos = 0;
int ch, pre_ch;
int ampersant = 0;
// bufsize = 16;
// pos = 0;

char *get_buf(char *buffer) {
	char ch = getchar();
	while ((ch != '\n')) {
		if (pos >= bufsize) {
			bufsize *= 2;
			buffer = realloc(buffer, bufsize);
		}
		buffer[pos] = ch;
		if(ch == '&')
		{
			ampersant = 1;
			// free(&buffer[pos]);
			// buffer[pos] = NULL;
		}
		pos++;
		pre_ch = ch;
		ch = getchar();
	}
	buffer[pos] = '\0';
	return buffer;
}

int main(int argc, char **argv) {
	char *buffer = malloc(bufsize * sizeof(char));
	shell_print();

	while (strcmp(get_buf(buffer), "exit") != 0) {

		if(pre_ch == '&' && ampersant)
		{

		}

		if(pos == 0)
		{
			free(buffer);
			shell_print();
			continue;
		}
//		printf("Source text: %s\n", buffer);
		int wordcounter = filter(&buffer);
		List head = NULL;
		head = buildlist(buffer, wordcounter);
//		printf("List: ");
//		printlist(head);
		char **args;
		args = build_args(head);
		shell_execute(args);
		free(buffer);
		int i = 0;
		pos = 0;
		free(args);
		clearlist(head);
		shell_print();
	}
	printf("\n");
	return 0;
}
