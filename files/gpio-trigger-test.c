#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "debug.h"

/* define our own signal */
#define SIG_WPS_TRIGGER		40

/* define the path that kernel module registered */
#define GPIO_SYSFS		"/sys/class/gpio-trigger/pid"

static short btn_state = 1;
static short worker_stop = 0;

void sigint_handler (int s) {
	log_info("Caught signal %d\n", s);
	worker_stop = 1;
}

void gpio_event_cb (int n, siginfo_t *info, void *unused) {
	log_info("Button %s\n", (info->si_int == 0) ? "Pressed" : "Released");
	btn_state = info->si_int;
}

int send_pid_to_kmod () {

	int fd;
	if ((fd = open (GPIO_SYSFS, O_WRONLY) ) == -1) {
		log_err("Cannot open output file\n");
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

void *work_func (void *argu) {
	while (worker_stop == 0) {
		log_info("%d\n", btn_state);
		sleep(1);
	}
	return NULL;
}

int main (int argc, char **argv) {

	pthread_t work_thread;

	// 1. send pid to kernel module
	if ( send_pid_to_kmod() == -1 ) {
		log_err("send_pid_to_kmod failed");
		exit(EXIT_FAILURE);
	}

	log_info("send_pid_to_kmod Success");

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

	log_info("Event Register Success");

	// 3. create thread to count the time during key press
	pthread_create(&work_thread, NULL, &work_func, NULL);

	// 4. waiting thread terminate
	pthread_join(work_thread, NULL);

	log_info("Process Return");
	return 0;
}