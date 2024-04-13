#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int openfile(char *buf)
{
	int fdd;
	int size = 0;

	memset(buf, 0x00, sizeof(buf));

	if ((fdd = open("./.myshell", O_RDONLY)) < 0)
	{
		fprintf(stderr, "Error\n");
		fflush(stderr);
		return -1;
	}
	size = read(fdd, buf, 1024);
	if (size == 0)
	{
		return -1;
	}
}

int ac(char **command1, char *buf)
{
	char tmp[1024];
	char buf2[1024];
	char *path[5];
	char *ptr;
	int i = 0;
	int count = 0;

	memset(tmp, 0x00, sizeof(tmp));
	memset(path, 0x00, sizeof(path));

	strcpy(buf2, buf);

	ptr = strtok(buf2, "=");
	while (ptr != NULL)
	{
		path[i] = ptr;
		ptr = strtok(NULL, ":,\n");
		i++;
	}
	path[i] = '\0';
	for (int k = 1; k < 4; k++)
	{
		strcpy(tmp, path[k]);
		strcat(tmp, command1[0]);
		if (access(tmp, F_OK) != 0)
		{
			count++;
		}
	}
	if (count == 3)
	{
		printf("Command Not Found\n");
		return -1;
	}
	else
	{
		return 0;
	}
}

int ac2(char **command1, char **command2, char *buf)
{
	if (ac(command1, buf) || ac(command2, buf) == -1)
	{
		fprintf(stderr, "Command Not Found\n");
		fflush(stderr);
		return -1;
	}
	else
		return 0;
}

void set(char **retext1, char **retext2, char *str, char *now)
{
	memset(str, 0x00, sizeof(str));
	memset(retext1, 0x00, sizeof(retext1));
	memset(retext2, 0x00, sizeof(retext2));

	printf("%c[1;32m", 27);
	printf("%s@Linux:", getlogin());
	printf("%c[1;34m", 27);
	printf("%s>", now);
	printf("%c[0m", 27);
	printf("$");
}

void pro4(char **retext1){
	if (strcmp(retext1[0], "logout") == 0)
	{
		exit(0);
	}
	else if (strcmp(retext1[0], "cd") == 0)
	{
		chdir(retext1[1]);
	}
}

void pro3(char **retext1)
{

	if (fork() == 0)
	{
		execvp(retext1[0], retext1);
		printf("command not found \n");
		exit(0);
	}
	wait(NULL);
}

void pro2(int fdr, char **retext1, char **retext2)
{
	switch (fork())
	{
		case -1:
			perror("fork");
			break;
		case 0:
			fdr = open(retext2[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fdr == -1)
			{
				perror("파일 새로생성 오류");
				exit(1);
			}
			if (dup2(fdr, 1) == -1)
			{
				perror("fdr dup error");
			}
			close(fdr);
			execvp(retext1[0], retext1);
			printf("command not found \n");
			exit(0);
			break;
		default:
			wait(NULL);
	}
}

void pro1(int *fd, char **retext1, char **retext2)
{
	switch (fork())
	{
		case -1:
			perror("fork error");
			break;
		case 0:
			if (close(1) == -1)
				perror("close1");
			if (dup(fd[1]) != 0);
			if (close(fd[0]) == -1 || close(fd[1]) == -1)
			{
				perror("close2");
			}
			execvp(retext1[0], retext1);
			printf("command not found \n");
			exit(0);
	}

	switch (fork())
	{
		case -1:
			perror("fork error");
			break;
		case 0:
			if (close(0) == -1)
				perror("close3");
			if (dup(fd[0]) != 0)
				;
			if (close(fd[0]) == -1 || close(fd[1]) == -1)
				perror("close4");
			execvp(retext2[0], retext2);
			printf("command not found \n");
			exit(0);
	}
}

void tok1(char **retext1, char *s, int i)
{
	char *p;
	s[strlen(s)] = '\0';

	i = 0;
	p = strtok(s, " ");
	while (p != NULL && i < 3)
	{
		retext1[i] = p;
		p = strtok(NULL, " ");
		i++;
	}

	retext1[i] = (char *)0;
}

void tok2(char *text1, char *text2, char **retext1, char **retext2, char *s, int i, char *Val)
{
	char *p;
	text1 = strtok(s, Val);
	text2 = strtok(NULL, Val);

	strcat(text1, "\0");
	strcat(text2, "\0");

	i = 0;
	p = strtok(text1, " ");

	while (p != NULL && i < 3)
	{
		retext1[i] = p;
		p = strtok(NULL, " ");
		i++;
	}
	retext1[i] = (char *)0;

	i = 0;
	p = strtok(text2, " ");

	while (p != NULL && i < 3)
	{
		retext2[i] = p;
		p = strtok(NULL, " ");
		i++;
	}
	retext2[i] = (char *)0;
}

int main()
{
	char str[1024];
	char now[1024];
	char buf[1024];
	char *text1 = NULL;
	char *text2 = NULL;
	char *command1[4];
	char *command2[5];
	char *pch;

	int fd[2];
	int fdr;
	int i;

	sigset_t blockset;
	sigemptyset(&blockset);
	sigaddset(&blockset, SIGINT);
	sigaddset(&blockset, SIGQUIT);
	sigprocmask(SIG_BLOCK, &blockset, NULL);

	openfile(buf);

	while (1)
	{
		getcwd(now, sizeof(now));
		set(command1, command2, str, now);

		fgets(str, sizeof(str), stdin);
		str[strlen(str) - 1] = '\0';

		if (feof(stdin))
		{
			printf("Ctrl+D exit \n");
			exit(0);
		}

		fflush(stdin);

		if (strchr(str, '|') != NULL)
		{
			tok2(text1, text2, command1, command2, str, i, "|");
			if (ac2(command1, command2, buf) == -1)
				continue;
			if (pipe(fd) == -1)
			{
				printf("fail to call pipe()\n");
				exit(1);
			}
			pro1(fd, command1, command2);
			if (close(fd[0]) == -1 || close(fd[1]) == -1)
			{
				perror("close5");
			}
			while (wait(NULL) != -1)
				;
		}

		else if (strchr(str, '>') != NULL)
		{
			tok2(text1, text2, command1, command2, str, i, ">");
			if (ac(command1, buf) == -1)
				continue;
			pro2(fdr, command1, command2);
		}

		else if (strchr(str, '<') != NULL)
		{
			tok2(text1, text2, command1, command2, str, i, "<");
			if (ac(command1, buf) == -1)
				continue;
			pro2(fdr, command2, command1);
		}

		else
		{
			tok1(command1, str, i);
			if (command1[0] != NULL)
			{
				pro4(command1);
				if (ac(command1, buf) == -1)
					continue;
				pro3(command1);
			}
		}
	}
	return 0;
}
