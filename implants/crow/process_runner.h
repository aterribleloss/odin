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

    Resizes the buffer as necessary (doubles)
    fd:
        the fd to read from
    buf:
        pointer to the char* buffer to read into
    buf_len:
        pointer to the length of data in buf
    buf_buf_len:
        pointer to the actual length of buf

    Returns the number of bytes read or -1 on error

    THE USER IS RESPONSIBLE TO FREE THE BUFFERS
*/
ssize_t readsome(int fd, char ** buf, size_t * buf_len, size_t * buf_buf_len);

/*
    Run a shell command and return stdout/stderr/retcode
*/
cmd_ret_t run_cmd(char * command_hint, char * command_arg_zero, char * command, long cmd_timeout);

/*
    Print a cmd_ret structure in a nice way for debugging
*/
void print_cmd_ret(cmd_ret_t cmd_ret);
