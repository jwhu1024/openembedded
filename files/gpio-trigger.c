#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>	//
#include <linux/gpio.h>		//
#include <linux/sysfs.h>	//
#include <linux/device.h>
// #include <asm/siginfo.h>
#include <linux/rcupdate.h>	//
// #include <linux/uaccess.h>
// #include <linux/slab.h>
#include <linux/sched.h>	//
// #include <linux/spinlock.h>

#define DRIVER_AUTHOR			"Lester Hu <lester_hu@bandrich.com>"
#define DRIVER_DESC			"GPIO Interrupt Test"

// real-time signals are in the range of 33 to 64
#define SIG_TEST			40

// use pin 85 on mdm9x30 (fastboot)
#define GPIO_ANY_GPIO			85

// text below will be seen in 'cat /proc/interrupt' command
#define GPIO_ANY_GPIO_DESC		"Some gpio pin description"

// below is optional
#define GPIO_ANY_GPIO_DEVICE_DESC 	"some_device"

/************************************************/
/* Interrupts block				*/
/************************************************/
short int irq_any_gpio = 0;
int current_irq = -1;

/************************************************/
/* Sysfs block					*/
/************************************************/
static int pid = -1;

/************************************************/
/* send_signal_to_usr				*/
/************************************************/
static void send_signal_to_usr (int _val) {
	printk("Ready to Send %d to process(pid:%d)\n", _val, pid);

	struct siginfo info;
	struct task_struct *t;
	memset(&info, 0, sizeof(struct siginfo));

	info.si_signo	= SIG_TEST;	// custom signal
	info.si_code	= SI_QUEUE;	// real-time signal
	info.si_int	= _val;		// assign gpio value

	rcu_read_lock();
	t = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
	if(t == NULL){
		printk("no such pid\n");
		pid = -1;
		rcu_read_unlock();
		return -ENODEV;
	}
	rcu_read_unlock();

	//send the signal with data
	if (send_sig_info(SIG_TEST, &info, t) < 0) {
		printk("error sending signal\n");
	}
	return;
}

/************************************************/
/* Into this function when gpio85 press/release	*/
/************************************************/
void btn_press_handler (unsigned long data) {
	printk("btn_press_handler.\n");
	printk("current_irq : %d\n", current_irq);
	printk("irq_any_gpio : %d\n", irq_any_gpio);

#ifdef _DEBUG_
	if (current_irq > 0) {
		printk("%s\n", (current_irq == irq_any_gpio)
				? "btn_press_handler performed"
				: "Unknown IRQ");
	}
#endif

	if (current_irq > 0		&&	// valid irq number
	    current_irq == irq_any_gpio	&&	// match our irq number
	    pid != -1) {			// pid setting before using this module
		printk("send_signal_to_usr (%d)\n", gpio_get_value(GPIO_ANY_GPIO));
		send_signal_to_usr(gpio_get_value(GPIO_ANY_GPIO));
	} else {
		printk("Do nothing in btn_press_handler, pid (%d)\n", pid);
	}
}

DECLARE_TASKLET(tsklt, btn_press_handler, 0);

/************************************************/
/* IRQ handler - fired on interrupt		*/
/************************************************/
static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
	printk("IRQ number is %d\n", irq);
	current_irq = irq;
	tasklet_schedule(&tsklt);
	return IRQ_HANDLED;
}

/************************************************/
/* This function configures interrupts.		*/
/************************************************/
void gpio_int_config (void) {
	if (gpio_is_valid(GPIO_ANY_GPIO)) {
		if (gpio_request(GPIO_ANY_GPIO, GPIO_ANY_GPIO_DESC)) {
			printk("GPIO request faiure: %s\n", GPIO_ANY_GPIO_DESC);
			return;
		}	
	}

	if ((irq_any_gpio = gpio_to_irq(GPIO_ANY_GPIO)) < 0) {
		printk("GPIO to IRQ mapping faiure %s\n", GPIO_ANY_GPIO_DESC);
		return;
	}

	printk("Mapped interrupt %d\n", irq_any_gpio);

	if (request_irq(irq_any_gpio,
			gpio_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			GPIO_ANY_GPIO_DESC,
			GPIO_ANY_GPIO_DEVICE_DESC)) {
		printk("IRQ Request failure\n");
	}
	return;	
}

/************************************************/
/* This function releases interrupts.		*/
/************************************************/
void gpio_int_release (void) {
	disable_irq(irq_any_gpio);
	free_irq(irq_any_gpio, GPIO_ANY_GPIO_DEVICE_DESC);
	gpio_free(GPIO_ANY_GPIO);
	tasklet_kill(&tsklt);
	printk("Module Unloaded\n");
	return;
}

/************************************************/
/* Sysfs attribute and functions.		*/
/************************************************/
static ssize_t
pid_show (struct class *class, struct class_attribute *attr, char *buf) {
	return sprintf(buf, "%d\n", pid);
}

static ssize_t
pid_store (struct class *class, struct class_attribute *attr, const char *buf, size_t count) {
	pid = (u16) simple_strtol (buf, NULL, 10);
	printk("pid = %d\n", pid);
	return count;
}

/************************************************/
/* Class attribute.				*/
/************************************************/
static struct class_attribute sysfs_attrs[] = {
	__ATTR(pid, S_IWUSR | S_IRUGO, pid_show, pid_store),
	__ATTR_NULL,
};

/************************************************/
/* Device model classes.			*/
/************************************************/
static struct class sysfs_class = {
	.name		= "gpio-trigger",
	.owner		= THIS_MODULE,
	.class_attrs	= sysfs_attrs,
};

/************************************************/
/* This function initialize Sysfs		*/
/************************************************/
int sysfs_config (void) {
	printk("sysfs_config !\n");
	class_register(&sysfs_class);
	return 1;
}

/************************************************/
/* This function releases Sysfs.		*/
/************************************************/
void sysfs_release (void) {
	printk("sysfs_release !\n");
	class_unregister(&sysfs_class);
	return;
}

/************************************************/
/* Module init / cleanup block.			*/
/************************************************/
static int __init gpio_init (void) {
	printk("Hello, gpio-trigger!\n");
	gpio_int_config();
	sysfs_config();
	return 0;
}

static void __exit gpio_cleanup (void) {
	printk("Goodbye, gpio-trigger\n");
	gpio_int_release();
	sysfs_release();
	return;
}

module_init(gpio_init);
module_exit(gpio_cleanup);

/************************************************/
/* Module licensing/description block.		*/
/************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);