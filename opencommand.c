#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>


int main(int argc, char *argv[])
{
	printf("Starting ...\n");
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
         return pid;
    }
/* This section is never be reached without a goto */
  error_fork:
    close(err[0]);
    close(err[1]);
  error_err:
    close(out[0]);
    close(out[1]);
  error_out:
    close(in[0]);
    close(in[1]);
  error_in:
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
