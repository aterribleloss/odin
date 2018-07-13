typedef enum {
	NOTASK = 0,
	CMD,
	GET
} task_type;

char * task_names[] = {
	[NOTASK] = NULL,
	[CMD] = "cmd",
	[GET] = "get"
};