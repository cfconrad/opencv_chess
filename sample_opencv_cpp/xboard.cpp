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
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]);
    char arg1[] = "/usr/bin/xboard";
    char arg2[] = "-ncp";
    char arg3[] = "-lpf";
    char arg4[] = "/home/asmorodskyi/Downloads/test.fen";
    char *const args[] = {arg1, arg2, arg3,arg4,NULL};
    int nResult = execvp(arg1, args);
    exit(nResult);
  }
  else
  {
    char bufr[1];
    string str = string("");
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_WRITE]);
    while (read(aStdoutPipe[PIPE_READ], bufr, 1))
    {
      str += bufr[0];
    }
    cout << "Output: " << endl
         << str << endl;
    int status;
    waitpid(nChild, &status, 0);
  }
}
