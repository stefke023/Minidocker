
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <string.h>
#include <sys/mount.h>

#define errP(err) do { \
    perror(err);\
    exit(EXIT_FAILURE); \
} while (0)

#define STACK_SIZE (1024 * 1024)

char uts_stack[STACK_SIZE];

int uts_func(void *arg) {
    pid_t pid;
    struct utsname uts;
    char **args = (char **)arg;

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        errP("make private failed\n");
    }

    int rv = sethostname(args[0], strlen(args[0]));

    if (rv != 0) {
        errP("uts set failure");
    }

    if (uname(&uts) == -1) {
        errP("uts get failure");
    }

    pid = getpid();

    printf("Nodename in child is %s\n", uts.nodename);
    printf("Process id of the child in his namespace is %d\n", pid);

    mount("proc", "/proc", "proc", 0, NULL);

    if(rv != 0) {
        errP("proc mount failure\n");
    }


    printf("\n\n***************************\n");
    execvp(args[1], &args[1]);

    errP("command call failure");

    return 0;
}


int main(int argc, char **argv) {
    pid_t child_pid;
    struct utsname uts;

    if (argc < 3) {
        errP("hostname or command not given");
    }

    child_pid = clone(uts_func, uts_stack + STACK_SIZE, SIGCHLD | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS, &argv[1]);

    if (child_pid == -1) {
        errP("clone failure");
    }

    printf("Pid of a child process created by clone is %d\n", child_pid);

    if (uname(&uts) == -1) {
            errP("uts get failure");
    }

    printf("Nodename in parent is %s\n", uts.nodename);


    if (waitpid(child_pid, NULL, 0) == -1) {
        errP("wait failure");
    }

    printf("\nHERE IS OUR CONTAINER PROCESS DONE\n");
    printf("***************************\n");

    return 0;
}