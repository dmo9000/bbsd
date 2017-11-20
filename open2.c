#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
 
/* since pipes are unidirectional, we need two pipes.
   one for data to flow from parent's stdout to child's
   stdin and the other for child's stdout to flow to
   parent's stdin */
 
#define NUM_PIPES          2
 
#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1
 
int pipes[NUM_PIPES][2];
 
/* always in a pipe[], pipe[0] is for read and 
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1
 
#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )

char buf_out[65536];
 
void
main()
{
    int outfd[2];
    int infd[2];
		int r = 0;
		int flags = 0;
     
    // pipes for parent to write and read
    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);
     
    if(!fork()) {
//        char *argv[]={ "/usr/bin/bc", "-q", 0};
        char *argv[]={ "/bin/bash", NULL };
 
        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);
 
        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);
          
        execv(argv[0], argv);
    } else {
        char buffer[100];
        int count;
 
        /* close fds not required by parent */       
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);

				// read from child's stdout

		flags = fcntl(PARENT_READ_FD, F_GETFL, 0);
		fcntl(PARENT_READ_FD, F_SETFL, flags | O_NONBLOCK);

    memset(&buf_out, 0, 65536);
    r = read(PARENT_READ_FD, &buf_out, 65536);

		while (r != 0) {
    if (r > 0) {
    	  printf("[read %d bytes]\n", r);
     		write(0, &buf_out, r);
      	} else {
				//printf("r = %d\n", r);
				}
    memset(&buf_out, 0, 65536);
    r = read(PARENT_READ_FD, &buf_out, 65536);
		}

        // Write to child’s stdin
        write(PARENT_WRITE_FD, "echo \"1 + 1\" | bc\n", 18);
  
        // Read from child’s stdout
        count = read(PARENT_READ_FD, buffer, sizeof(buffer)-1);
        if (count >= 0) {
            buffer[count] = 0;
            printf("%s", buffer);
        } else {
            printf("IO Error\n");
        }
    }
}
