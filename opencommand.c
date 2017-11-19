#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>

static char buf_out[65536];

int main(int argc, char *argv[])
{
	int filedes[2];
	pid_t p = 0;
	int r = 0;
	argv[0] = "/usr/bin/fortune";
	argv[1] = "-l";
	argv[2] = NULL;
	p = opencmd(&filedes, "/usr/bin/fortune", argv); 
	printf("started process with pid %u\n", p);
	memset(&buf_out, 0, 65536);
	printf("filedes[1] = %d\n", filedes[1]);
	r = read(filedes[1], &buf_out, 65536);
	while (r > 0) {
		printf("[read %d bytes]\n", r);
		write(0, &buf_out, r);
		memset(&buf_out, 0, 65536);
		r = read(filedes[1], &buf_out, 65536);
		}

	exit(0);
}

pid_t opencmd(int *pipes, const char *path, char *const *const argv)
{
    if (!pipes || !path || !argv)
        goto error_in;

    int in[2];
    int out[2];
    int err[2];
    pid_t pid;

    /*
     * Creating pipes in between ...
     */
    if (pipe(in))
        goto error_in;
    if (pipe(out))
        goto error_out;
    if (pipe(err))
        goto error_err;

	printf("in[0] = %d\n", in[0]);
	printf("in[1] = %d\n", in[1]);
	printf("out[0] = %d\n", out[0]);
	printf("out[1] = %d\n", out[1]);

    /*
     * Starting actual processing ...
     */
    pid = fork();
    switch (pid) {
     case -1:       /* Error */
         goto error_fork;
     case 0:        /* Child */
         close(in[1]);
         close(out[0]);
         close(err[0]);

         dup2(in[0], 0);    /* redirect child stdin to in[0] */
         dup2(out[1], 1);   /* redirect child stdout to out[1] */
         dup2(err[1], 2);   /* redirect child stderr to err[1] */
         execv(path, argv); /* actual command execution */
         return -1; /* shall never be executed */
     default:       /* Parent */
         close(in[0]);  /* no need to read its stdin */
         close(out[1]); /* no need to write to its stdout */
         close(err[1]); /* no need to write to its stderr */

         pipes[0] = in[1];  /* Write to child stdin */
         pipes[1] = out[0]; /* Read child stdout */
         pipes[2] = err[0]; /* Read child stderr */
				printf("returing pid %d\n", pid);
				printf("pipes[0] = %d\n", pipes[0]);
				printf("pipes[1] = %d\n", pipes[1]);
				printf("pipes[2] = %d\n", pipes[2]);
         return pid;
    }
/* This section is never be reached without a goto */
  error_fork:
		printf("Forking error.\n");
    close(err[0]);
    close(err[1]);
  error_err:
		printf("Error on connecting stderr.\n");
    close(out[0]);
    close(out[1]);
  error_out:
		printf("Error on connecting stdout.\n");
    close(in[0]);
    close(in[1]);
  error_in:
		printf("Error on connecting stdin.\n");
    return -1;
}

int closecmd(const pid_t pid, int *pipes)
{
    int status;
    waitpid(pid, &status, 0);
    close(pipes[0]);
    close(pipes[1]);
    close(pipes[2]);
    return status;
}
