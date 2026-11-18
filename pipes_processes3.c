#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// This program runs: cat scores | grep <arg> | sort
// P1 (parent) runs: cat scores
// P2 (child) runs: grep <arg>
// P3 (child's child) runs: sort
// Each process turns into its program with exec, so none of them
// can call wait(). The shell waits for P1 and the system cleans up the rest.
 

int main(int argc, char **argv)
{
  int cat_to_grep[2]; 
  int grep_to_sort[2];
  int pid;
  int pid2;

  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <grep argument>\n", argv[0]);
    return 1;
  }
  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  // make both pipes before forking so every process inherits both

  if (pipe(cat_to_grep) == -1 || pipe(grep_to_sort) == -1)
  {
    perror("pipe");
    return 1;
  }
  pid = fork();

  if (pid < 0)
  {
    perror("fork");
    return 1;
  }
  if (pid == 0)
  {
    //P2 (child): make P3 for sort, then turn into grep

    pid2 = fork();
    
    if (pid2 < 0)
    {
      perror("fork");
      return 1;
    }
    if (pid2 == 0)
    {
      // P3 handles  runs sort"

      //Read input from the grep_to_sort pipe
      dup2(grep_to_sort[0], 0);

      //close every pipe. Input is already set up, so we don't need them.

      close(cat_to_grep[0]);
      close(cat_to_grep[1]);
      close(grep_to_sort[0]);
      close(grep_to_sort[1]);

      execvp("sort", sort_args);
      perror("execvp sort");
      exit(1);
    }
    else
    {
      // P2 runs "grep <arg>"
      dup2(cat_to_grep[0], 0);
      dup2(grep_to_sort[1], 1);

      //close every pipe end so sort sees EOF when grep exits
      close(cat_to_grep[0]);
      close(cat_to_grep[1]);
      close(grep_to_sort[0]);
      close(grep_to_sort[1]);

      execvp("grep", grep_args);
      perror("execvp grep");
      exit(1);
    }
  }
  else
  {
    //P1 handles the cat scores

    dup2(cat_to_grep[1], 1);

    //Close all pipe so grep knows when the input is done

    close(cat_to_grep[0]);
    close(cat_to_grep[1]);
    close(grep_to_sort[0]);
    close(grep_to_sort[1]);

    execvp("cat", cat_args);
    perror("execvp cat");
    exit(1);
  }
  return 0;
}