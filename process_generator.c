#include "headers.h"

void clear_resources(int);

int read_processes(pri_queue* processes, int* count, char* filename);

int fork_clk(/* out */ pid_t* clkPid);
int fork_scheduler(int algorithm, int quantum, int procCount, /* out */ pid_t* schedPid);

int initialize_message_queue();
int process_loop(pri_queue* processes, key_t clk_child);

key_t process_msgq_id;

int get_arg(int argc, char* argv[], const char* name, char** output) {
	if (!output) return 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], name) == 0) {
			if (i + 1 >= argc) return 0;

			*output = argv[i + 1];
			return 1;
		}
	}

	return 0;
}

int main(int argc, char* argv[]) {
	// set interrupt handler
	signal(SIGINT, clear_resources);

	// read args
	if (argc < 2) {
		printf("Usage: %s testcasefilename.txt -sch schedType [-q quantumNumber]\n", argv[0]);
		return 1;
	}

	int schedAlgo = -1;
	int quantum = -1;

	char* processesFile = argv[1];
	char* schedTypeStr;
	char* qStr;

	if (!get_arg(argc, argv, "-sch", &schedTypeStr)) {
		printf("Usage: %s testcasefilename.txt -sch schedType [-q quantumNumber]\n", argv[0]);
		return 1;
	}

	// set sched algo 0 indexed
	schedAlgo = atoi(schedTypeStr) - 1;

	if (get_arg(argc, argv, "-q", &qStr)) {
		// parse quantum
		quantum = atoi(qStr);
	}

	// process queue (priority = arrivalTime) incase processes.txt isnt sorted by AT
	pri_queue processesQueue;
	pri_queue_init(&processesQueue);

	int procsCount;
	int readProcResult;
	if (!(readProcResult = read_processes(&processesQueue, &procsCount, processesFile))) {
		printf("Cannot read processes result=%d\n", readProcResult);
		goto exit;
	}

	// fork scheduler
	pid_t schedulerPid;
	if (!fork_scheduler(schedAlgo, quantum, procsCount, &schedulerPid)) {
		// error msg is printed inside
		goto exit;
	}

	pid_t clkPid;
	if (!fork_clk(&clkPid)) {
		goto exit;
	}

	initClk();

	if (!process_loop(&processesQueue, clkPid)) {
		perror("Error in process loop");
		goto exit;
	}

	wait(NULL);

exit:
	pri_queue_free(&processesQueue, 0);
	
	// invoke our own handler for now?
	raise(SIGINT);

	return 0;
}

void clear_resources(int signum) {
	printf("[ProcGen] Cleaning up...\n");

	msgctl(process_msgq_id, IPC_RMID, (struct msqid_ds*)0);
	destroyClk(true);
	exit(0);
}

int read_processes(pri_queue* processes, int* count, char* filename) {
	if (count) {
		*count = 0;
	}

	if (!processes)
		return 0;

	FILE* f = fopen(filename, "r");
	if (!f)
	{
		// invalid file?
		return 0;
	}

	char* line = 0;
	size_t lineLen = 0;
	while (getline(&line, &lineLen, f) != EOF)
	{
		// we have a line :P
		// ignore empty lines or lines that start with #
		if (lineLen == 0 ||
			strlen(line) == 0 ||
			line[0] == '#')
			continue;

		// allocate process
		struct process_data* p = malloc(sizeof(process_data));
		memset(p, 0, sizeof(process_data));

		// read proc data
		sscanf(line, "%d%d%d%d%d", &p->id, &p->arrival_time, &p->running_time, &p->priority, &p->mem_size);

		// insert in queue
		pri_queue_enqueue(processes, p->arrival_time, p);
		printf("Process with id %d, arrivaltime %d, remainingtime %d, priority %d memsize %d\n", p->id, p->arrival_time, p->running_time, p->priority, p->mem_size);

		if (count) {
			(*count)++;
		}
	}

	// close file
	fclose(f);

	// free line
	if (line)
	{
		free(line);
	}

	return 1;
}

int fork_clk(/* out */ pid_t* clkPid) {
	pid_t child = fork();
	if (child == -1) {
		perror("Failed to fork clk");
		return 0;
	}
	else if (child == 0) {
		execl("./clk.out", "clk.out", NULL);
	}

	*clkPid = child;
	return 1;
}

int fork_scheduler(int algorithm, int quantum, int procCount, /* out */ pid_t* schedPid) {
	pid_t child = fork();
	if (child == -1) {
		perror("Failed to fork scheduler");
		return 0;
	}
	else if (child == 0) {
		// alloc params
		char params[3][50];
		sprintf(params[0], "%d", algorithm);
		sprintf(params[1], "%d", quantum);
		sprintf(params[2], "%d", procCount);

		execl("./scheduler.out", "scheduler.out", params[0], params[1], params[2], NULL);
	}

	// parent

	*schedPid = child;
	return 1;
}

int initialize_message_queue() {
	process_msgq_id = msgget(MSGKEY, 0666 | IPC_CREAT);
	if (process_msgq_id == -1) {
		perror("Error in create Message Queue");
		return 0;
	}

	return 1;
}

int process_loop(pri_queue* processes, key_t clk_child) {
	// Message Queue Generation to send the process data to the scheduler
	if (!initialize_message_queue()) {
		// error msg already printed
		return 0;
	}

	process_message_buffer msgBuffer;
	msgBuffer.type = 1;

	process_data* proc = 0;
	while (pri_queue_dequeue(processes, (void**)&proc))
	{
		// keep waiting
		while (proc->arrival_time > getClk())
		{
			//printf("waiting for %d\n", proc->arrival_time - getClk());

			// sleep for 200ms
			usleep(200 * 1000);
		}

		printf("[ProcGen] %d - sending process with id %d and running time %d and arrivaltime  %d and priority %d\n", getClk(), proc->id, proc->running_time, proc->arrival_time, proc->priority);

		// send via msgq
		msgBuffer.data = *proc;

		if (msgsnd(process_msgq_id, &msgBuffer, sizeof(msgBuffer.data), !IPC_NOWAIT) == -1) {
			perror("Error in send");
			return 0;
		}
	}

	return 1;
}