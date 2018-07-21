#include <unistd.h>

typedef struct {
    int success;
    int ret;
    char * stdout;
    size_t stdout_len;
    size_t stdout_buf_len;
    char * stderr;
    size_t stderr_len;
    size_t stderr_buf_len;
} cmd_ret_t;

/*
    Read data into a buffer

    Resizes the buffer as necessary
    fd:
        the fd to read from
    buf:
        pointer to the char* buffer to read into
    buf_len:
        pointer to the length of data in buf
    buf_buf_len:
        pointer to the actual length of buf
*/
ssize_t readsome(int fd, char ** buf, size_t * buf_len, size_t * buf_buf_len);

/*
    Run a shell command and return stdout/stderr/retcode
*/
cmd_ret_t run_cmd(char * command_hint, char * command_arg_zero, char * command, long cmd_timeout);
