#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <sys/wait.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

using namespace std;

int initPipe(int pipe_array[])
{
  if (pipe(pipe_array) < 0)
  {
    close(pipe_array[PIPE_READ]);
    close(pipe_array[PIPE_WRITE]);
    perror("allocating pipe for child ");
    exit(errno);
  }
}

int main()
{
  int aStdinPipe[2];
  int aStdoutPipe[2];
  int nChild;

  initPipe(aStdinPipe);
  initPipe(aStdoutPipe);

  nChild = fork();
  if (0 == nChild)
  {
      if (dup2(aStdinPipe[PIPE_READ], STDIN_FILENO) == -1)
    {
      exit(errno);
    }
    // redirect stdout
    if (dup2(aStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1)
    {
      exit(11);
    }
    close(aStdoutPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
#if 0
    // redirect stderr
    if (dup2(aStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1)
    {
      exit(errno);
    }
#endif
    cout << "TEST" << endl;

    char *const args[] = {"echo", "foo", NULL};
    int nResult = execvp("echo", args);
    exit(nResult);
  }
  else
  {
    //char *szMessage = new char(sizeof("setboard 8/8/8/1p6/4P3/k3P3/8/3K4 w - - 0 1"));
    //write(aStdinPipe[PIPE_WRITE], szMessage, strlen(szMessage));
    int status;

    cout << endl;
    char bufr[1];
    while (read(aStdoutPipe[PIPE_READ], bufr, 1))
    {
      printf("char: %c\n", *bufr);
    }
    int ret_code = waitpid(nChild, &status, 0);
    printf("status : %d, retcode: %d",WEXITSTATUS(status),ret_code);
  }
}
