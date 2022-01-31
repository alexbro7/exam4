#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
unsigned int	nbSeg(char** s, char* c){
	unsigned int r = 1;
	if (!s || !s[0])
		return 0;
	for (unsigned int i = 0; s[i]; i++)
		r += strcmp(s[i], c) ? 0 : 1;
	return r;
}
unsigned int nextOccur(char** s, char* c, unsigned int i){
	while (s && s[i] && strcmp(s[i], c))
		i++;
	return i;
}
char**	subTab(char** s, unsigned int a, unsigned int b){
	char** r;
	if (!(r = malloc(sizeof(char*) * (b - a + 1))))
		return NULL;
	for (unsigned int i = 0; a + i < b; i++)
		r[i] = s[a + i];
	r[b - a] = NULL;
	return r;
}
void	putErr(char *s){
	unsigned int i = 0;
	while (s && s[i])
		i++;
	write (2, s, i);
}
void	cd(char** av){
	if (!av[1] || av[2])
		return putErr("error: cd: bad arguments\n");
	if (chdir(av[1]) == -1){
		putErr("error: cd: cannot change directory to ");
		putErr(av[1]);
		write(2, "\n", 1);
	}
}
void	exec(char** av, char** env, int* fd, int last){
	int	pipefd[2];
	pid_t	pid;
	if (!strcmp(av[0], "cd"))
		return cd(av);
	if (pipe(pipefd) == -1 || (pid = fork()) == -1){
		putErr("error: fatal\n");
		exit(1);
	}
	if (!pid){
		close(pipefd[0]);
		dup2(*fd, STDIN_FILENO);
		if (!last)
			dup2(pipefd[1], STDOUT_FILENO);
		if (execve(av[0], av, env) == -1){
			putErr("error: cannot execute ");
			putErr(av[0]);
			write(2, "\n", 1);
		}
		close(pipefd[1]);
	}
	else{
		close(pipefd[1]);
		waitpid(pid, NULL, 0);
		kill(pid, SIGTERM);
		if (!last)
			dup2(pipefd[0], *fd);
		close(pipefd[0]);
	}
}
int	main(int ac, char** av, char** env){
	char** cmd, **pipe;
	unsigned int lCmd = 0, lPipe;
	int fd = 0;
	if (ac < 2)
		return 1;
	for (unsigned int i = 0; i < nbSeg(av, ";"); i++){
		if (!(cmd = subTab(av, lCmd + 1, nextOccur(av, ";", lCmd + 1)))){
			putErr("error: fatal\n");
			return 1;
		}
		lPipe = -1;
		for (unsigned int j = 0; j < nbSeg(cmd, "|"); j++){
			if (!(pipe = subTab(cmd, lPipe + 1, nextOccur(cmd, "|", lPipe + 1)))){
				free(cmd);
				putErr("error: fatal\n");
				return 1;
			}
			exec(pipe, env, &fd, j + 1 == nbSeg(cmd, "|"));
			free(pipe);
			lPipe = nextOccur(cmd, "|", lPipe + 1);
		}
		free(cmd);
		lCmd = nextOccur(av, ";", lCmd + 1);
	}
	return 0;
}
