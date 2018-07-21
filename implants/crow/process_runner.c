#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "process_runner.h"

#include "logc/src/log.h"


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


ssize_t readsome(int fd, char ** buf, size_t * buf_len, size_t * buf_buf_len) {
    size_t remaining = *buf_buf_len - *buf_len;
    if(remaining == 0) {
        void * newloc = realloc((void *)(*buf), *buf_buf_len * 2);
        if(!newloc) {
            log_error("Couldn't resize buffer");
            return (ssize_t) -1;
        }
        *buf = newloc;
        *buf_buf_len *= 2;
        remaining = *buf_buf_len - *buf_len;
        log_trace("Expanded to %d", *buf_buf_len);
    }
    log_trace("Trying to read %d from %d", remaining, fd);
    ssize_t numread = read(fd, (void *)(*buf) + *buf_len, remaining);
    log_trace("\tRead %d from %d", numread, fd);
    if(numread == -1) {
        return (ssize_t) -1;
    }
    *buf_len += numread;
    return numread;
}


cmd_ret_t run_cmd(char * command_hint, char * command_arg_zero, char * command, long cmd_timeout) {

    
    cmd_ret_t cmd_ret = {
        0,
        0,
        (char*)malloc(32),
        0,
        32,
        (char*)malloc(32),
        0,
        32
    };

    int stdout_pipe[2];
    int stderr_pipe[2];
    if(pipe(stdout_pipe) || pipe(stderr_pipe)) {
        log_error("Couldn't open pipes");
        return cmd_ret;
    }

    pid_t cpid = fork();
    if(cpid == -1) {
        log_error("Failed to fork");
        return cmd_ret;
    }
    if(cpid == 0) {
        // child
        log_trace("-Child running");

        // close read ends
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        alarm(cmd_timeout);
        log_trace("-Alarm set");
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        log_trace("-Duplicated");
        execl(command_hint, command_hint, command_arg_zero, command, (char *)0);

        // close write ends
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
    }
    else {
        // parent
        log_trace("Parent running");
    
        // close write ends
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        log_trace("Closed write pipes");

        struct timeval timeout = {cmd_timeout, 0};



        int still_reading = 1;
        int status;
        ssize_t num_read = 0;
        fd_set readfds;
        int nfds = max(stdout_pipe[0], stderr_pipe[0]) + 1;
        if(nfds > FD_SETSIZE) {
            log_fatal("File descriptors are too large for select");
            return cmd_ret;
        }
        while(still_reading) {
            log_trace("Reading...");
            waitpid(cpid, &status, WNOHANG);
            if(WIFEXITED(status) || WIFSIGNALED(status)) {
                still_reading = 0;
                log_trace("Child has exited or signaled");
            }

            FD_ZERO(&readfds);
            FD_SET(stdout_pipe[0], &readfds);
            FD_SET(stderr_pipe[0], &readfds);
            
            int ready = select(nfds, &readfds, NULL, NULL, &timeout);
            if(ready == -1) {
                log_error("Select failure");
                return cmd_ret;
            }

            if(ready) {
                if(FD_ISSET(stdout_pipe[0], &readfds)) {
                    num_read = readsome(stdout_pipe[0], &cmd_ret.stdout, &cmd_ret.stdout_len, &cmd_ret.stdout_buf_len);
                    if(num_read > 0) {
                        still_reading = 1;
                    }
                }
                if(FD_ISSET(stderr_pipe[0], &readfds)) {
                    num_read = readsome(stderr_pipe[0], &cmd_ret.stderr, &cmd_ret.stderr_len, &cmd_ret.stderr_buf_len);
                    if(num_read > 0) {
                        still_reading = 1;
                    }
                }

            }
        }

        log_trace("Waiting for child to finish");
        waitpid(cpid, &status, 0);
        cmd_ret.ret = WEXITSTATUS(status);
        cmd_ret.success = 1;
    }

    return cmd_ret;

}

void print_cmd_ret(cmd_ret_t cmd_ret) {
    if(!cmd_ret.success) {
        puts("!!! Command failed");
    }
    else {
        printf("### Command exited with value %d\n", cmd_ret.ret);
        puts("### Stdout:");
        fwrite(cmd_ret.stdout, 1, cmd_ret.stdout_len, stdout);
        puts("\n### Stderr:");
        fwrite(cmd_ret.stderr, 1, cmd_ret.stderr_len, stderr);
        puts("\n### END");
    }
}


#ifdef TESTING

/*
    Read from stdin into a cmd_ret and print progress
*/
void test_readsome() {
    cmd_ret_t cmd_ret = {
        0,
        0,
        (char*)malloc(1),
        0,
        1,
        (char*)malloc(1),
        0,
        1
    };

    while(-1 != readsome(0, &cmd_ret.stdout, &cmd_ret.stdout_len, &cmd_ret.stdout_buf_len)) {
        printf("Read %d: %s\n", (int)cmd_ret.stdout_len, cmd_ret.stdout);
    }

}

void test_run_cmd(char * cmd, long timeout) {
    log_trace("Running command %s with timeout %ld", cmd, timeout);
    cmd_ret_t cmd_ret = run_cmd("/bin/sh", "-c", cmd, timeout);
    log_trace("Printing output");
    print_cmd_ret(cmd_ret);
}

int main() {
    log_set_level(LOG_TRACE);
    // test_readsome();
    test_run_cmd("ls -lah", 1);
    // test_run_cmd("for i in 1 2 3 4 5 6 7 8 9 0; do sleep 1; echo hello; done", 5);
    // test_run_cmd("echo 'this is stdout' && echo 'this is stderr' 1>&2", 1);
    return 0;
}



#endif