
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
#include <sys/stat.h>
#include <errno.h>
#include <sys/syscall.h>

#define errP(err) do { \
    perror(err);\
    exit(EXIT_FAILURE); \
} while (0)

#define STACK_SIZE (1024 * 1024)

char uts_stack[STACK_SIZE];

int pivot_root(const char *rootfs, const char *tmp_rootfs) {
    if (syscall(SYS_pivot_root, rootfs, tmp_rootfs) == -1)
        return -1;

    return chdir("/");
}

void usage(void) {
    printf("You didn't call program as you should\n");
    printf("sudo ./build/program.out <hostname> <rootfs_path> <command> [args...]\n");
}

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

    char *rootfs = args[1];

    if (mount(rootfs, rootfs, NULL, MS_REC | MS_BIND , NULL) == -1) {
        errP("binding new rootfs failed\n");
    }

    size_t tmp_size = strlen(rootfs) + strlen("/oldroot") + 1;
    char *rootfs_tmp = (char *)malloc(tmp_size);
    if (rootfs_tmp == NULL) {
        errP("malloc allocation failure");
    }
    snprintf(rootfs_tmp, tmp_size, "%s%s", rootfs, "/oldroot");

    if (mkdir(rootfs_tmp, 0755) == -1 && errno != EEXIST) {
        errP("binding tmp rootfs failed\n");
    }

    if (pivot_root(rootfs, rootfs_tmp) == -1) {
        errP("changing to a new root failed");
    }

    if (umount2("/oldroot", MNT_DETACH) == -1) {
        errP("umounting tmp rootfs failed\n");
    }

    free(rootfs_tmp);
    rootfs_tmp = NULL;

    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        errP("proc mount failure\n");
    }

    printf("\n\n***************************\n");
    execvp(args[2], &args[2]);

    errP("command call failure");

    return 0;
}


int main(int argc, char **argv) {
    pid_t child_pid;
    struct utsname uts;

    if (argc < 4) {
        usage();
        errP("invalid program call");
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