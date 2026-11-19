
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
// C program to demonstrate use of fork() and pipe()
// with a 2-way pipe (two pipes, one in each direction).
//
// P1 (parent) reads a string and sends it to P2 (child).
// P2 adds "howard.edu", prints it, asks for a second string, adds that,
// and sends the result back to P1.
// P1 adds "gobison.org" and prints the final string.

int main()
{
    int fd1[2]; // pipe 1: P1 -> P2
    int fd2[2]; // pipe 2: P2 -> P1

    char fixed_str[] = "howard.edu";
    char second_fixed[] = "gobison.org";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if (pipe(fd2) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    printf("Input : ");
    fflush(stdout);
    scanf("%99s", input_str);

    p = fork();

    if (p < 0)
    {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // Parent process (P1)
    else if (p > 0)
    {
        // P1 only writes to pipe 1 and only reads from pipe 2
        close(fd1[0]);
        close(fd2[1]);

        // Send the first string to P2
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Wait for P2 to send the longer string back
        char final_str[300];
        read(fd2[0], final_str, sizeof(final_str));
        close(fd2[0]);

        // Wait for P2 to finish
        wait(NULL);

        // Add the last fixed string and print
        strcat(final_str, second_fixed);
        printf("Output : %s\n", final_str);
    }

    // Child process (P2)
    else
    {
        // P2 only reads from pipe 1 and only writes to pipe 2
        close(fd1[1]);
        close(fd2[0]);

        // Read the string from P1
        char concat_str[300];
        read(fd1[0], concat_str, 100);
        close(fd1[0]);

        // Add "howard.edu" and print it
        strcat(concat_str, fixed_str);
        printf("Output : %s\n", concat_str);

        // Ask for a second string and add it
        char second_input[100];
        printf("Input : ");
        fflush(stdout);
        scanf("%99s", second_input);
        strcat(concat_str, second_input);

        // Send the string back to P1
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}