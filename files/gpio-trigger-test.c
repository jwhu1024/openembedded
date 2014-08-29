#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 
#define SIG_TEST	40

#define GPIO_SYSFS	"/sys/class/gpio-trigger/pid"

int trigger_count = 0;

void receiveData(int n, siginfo_t *info, void *unused) {
	printf("%s - button %s\n", __FUNCTION__
				 , (info->si_int == 0) ? "Pressed" : "Released");

	if (trigger_count++ == 10) {
		exit(1);
	}
}

void register_gpio_event() {
	char cmd[128] = {0};
	sprintf(cmd, "echo %d > %s", (int) getpid(), GPIO_SYSFS);
	system(cmd);
}

int main ( int argc, char **argv )
{
	// 1. set pid to kernel module
	register_gpio_event();

	// 2. register event with SIG_TEST
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIG_TEST, &sig, NULL);

	for (;;)
        	pause();

	return 0;
}