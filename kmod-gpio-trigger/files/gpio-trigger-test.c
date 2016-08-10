#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "debug.h"

#define SIG_WPS_TRIGGER			40				/* define our own signal */
#define GPIO_SYSFS			"/sys/class/gpio-trigger/pid"	/* define the path that kernel module registered */
#define SECONDS 			1000				/* 1 seconds */
#define FIVE_SECONDS 			5 * SECONDS			/* 5 seconds */
#define TRIGGER_THRESHOLD		FIVE_SECONDS			/* Threshold */

typedef enum {
	BTN_PRESS = 0,
	BTN_RELEASE
} BTN_STATE_E;

static BTN_STATE_E current_btn_state 	= BTN_RELEASE;
static short worker_stop 		= 0;
static useconds_t worker_delay 		= 200000;			/* microseconds for usleep (0.2 seconds)*/
static pthread_mutex_t btn_state_lock;

/******************************************
* SIGINT handler
******************************************/
void sigint_handler (int s) {
	debug("Caught signal %d\n", s);
	worker_stop = 1;
}

/******************************************
* SIG_WPS_TRIGGER handler
******************************************/
void gpio_event_cb (int n, siginfo_t *info, void *unused) {
	debug("Button %s\n", (info->si_int == 0) ? "Pressed" : "Released");

	pthread_mutex_lock(&btn_state_lock);
	current_btn_state = (BTN_STATE_E) info->si_int;
	pthread_mutex_unlock(&btn_state_lock);
}

/******************************************
* Send PID to kernel module
******************************************/
int send_pid_to_kmod () {
	int fd;
	if ((fd = open (GPIO_SYSFS, O_WRONLY) ) == -1) {
		debug("Cannot open output file\n");
		return -1;
	}

	int pid_len = sizeof(pid_t);
	char *pid_str = (char *) malloc (pid_len * sizeof (char));

	// make sure pointer is valid, otherwise return -1 directly
	check_mem(pid_str);

	// initialize memory
	memset(pid_str, 0, pid_len);

	// assign current process id to pid_str
	sprintf(pid_str, "%d", getpid());

	// write to file
	int bytes_write = write (fd, pid_str, strlen(pid_str));

	// uninitialize
	close(fd);
	free(pid_str);
	
	return (bytes_write == -1) ? -1 : 1;
}

/******************************************
* Get timestamp
******************************************/
long long get_system_time () {
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}

long long get_timeuse (long long start, long long end) {
	return end - start;
}

/******************************************
* Worker thread
******************************************/
void * work_func (void *argu) {

#ifndef NDEBUG
	char cmd[128] = {0};
	system("touch /tmp/gpio.log");
#endif

	long long start = 0, end = 0, timeuse = 0;
	static BTN_STATE_E prev_btn_state = BTN_RELEASE;

	while (worker_stop == 0) {
		debug("prev_btn_state : %d\tcurrent_btn_state : %d", (int) prev_btn_state, (int) current_btn_state);

		// lock mutex
		pthread_mutex_lock(&btn_state_lock);

		if (prev_btn_state != current_btn_state) {
			switch (current_btn_state) {
				case BTN_PRESS:
				{
					start = get_system_time();
					debug("start time : %lld", start);
					break;
				}
				case BTN_RELEASE:
				{
					end = get_system_time();
					debug("end time : %lld", end);
					break;
				}
				default:
					debug("Unknown Button State");
					break;
			}
		}

		// store previous state
		prev_btn_state = current_btn_state;

		// unlock mutex
		pthread_mutex_unlock(&btn_state_lock);

		if (start != 0 && end != 0) {
			timeuse = get_timeuse (start, end);
			debug("\x1B[32mTimeuse : %lld \033[0m", timeuse);

			if (timeuse >= TRIGGER_THRESHOLD) {
				// do some stuff here [TBD]
				debug("###### Threshold Arrival ######");
			}

#ifndef NDEBUG
			sprintf(cmd, "echo %d >> /tmp/gpio.log", timeuse);
			system(cmd);
			memset(cmd, 0, 128);
#endif
			// reset timers
			start = end = timeuse = 0;
		}

		// sleep 0.20 seconds
		usleep(worker_delay);
	}
	return NULL;
}

/******************************************
* Entry point
******************************************/
int main (int argc, char **argv) {
	pthread_t work_thread;

	// 1. send pid to kernel module
	if ( send_pid_to_kmod() == -1 ) {
		debug("send_pid_to_kmod failed");
		exit(EXIT_FAILURE);
	}

	debug("send_pid_to_kmod Success");

	// init mutex
	pthread_mutex_init(&btn_state_lock, NULL);

	// 2. register event with SIG_WPS_TRIGGER
	struct sigaction sig;
	sig.sa_sigaction = gpio_event_cb;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIG_WPS_TRIGGER, &sig, NULL);

	// 2-1. register SIGINT handler
	memset(&sig, 0, sizeof(sig));
	sig.sa_handler = sigint_handler;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sigaction(SIGINT, &sig, NULL);

	debug("Event Register Success");

	// 3. create thread to count the time during key press
	pthread_create(&work_thread, NULL, &work_func, NULL);

	// 4. waiting thread terminate
	pthread_join(work_thread, NULL);

	pthread_mutex_destroy(&btn_state_lock);
	debug("Process Return");
	return 0;
}