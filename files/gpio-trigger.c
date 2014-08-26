#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/sysfs.h>
#include <linux/device.h>

#define DRIVER_AUTHOR			"Lester Hu <lester_hu@bandrich.com>"
#define DRIVER_DESC			"GPIO Interrupt Test"

// use pin 85 on mdm9x30 (fastboot)
#define GPIO_ANY_GPIO			85

// text below will be seen in 'cat /proc/interrupt' command
#define GPIO_ANY_GPIO_DESC		"Some gpio pin description"

// below is optional
#define GPIO_ANY_GPIO_DEVICE_DESC 	"some_device"

/****************************************************************************/
/* Interrupts variables block						    */
/****************************************************************************/
short int irq_any_gpio = 0;

/****************************************************************************/
/* Sysfs block						    		    */
/****************************************************************************/
static u16 pid = -1;

/****************************************************************************/
/* IRQ handler - fired on interrupt					    */
/****************************************************************************/
static irqreturn_t gpio_irq_handler (int irq, void *dev_id, struct pt_regs *regs) {
	int val = -1;
	unsigned long flags;

	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);

	val = gpio_get_value(GPIO_ANY_GPIO);
	printk("Interrupt [%d] for device %s was triggered ! (%d)\n",
			irq, (char *) dev_id, val);

	// restore hard interrupts
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

/****************************************************************************/
/* This function configures interrupts.					    */
/****************************************************************************/
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

	printk("Mapped int %d\n", irq_any_gpio);

	if (request_irq(irq_any_gpio,
			(irq_handler_t ) gpio_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			GPIO_ANY_GPIO_DESC,
			GPIO_ANY_GPIO_DEVICE_DESC)) {
		printk("Irq Request failure\n");
		return;
	}
	return;
}

/****************************************************************************/
/* This function releases interrupts.					    */
/****************************************************************************/
void gpio_int_release (void) {
	free_irq(irq_any_gpio, GPIO_ANY_GPIO_DEVICE_DESC);
	gpio_free(GPIO_ANY_GPIO);
	return;
}

/****************************************************************************/
/* Sysfs attribute and functions.					    */
/****************************************************************************/
static ssize_t
pid_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%d\n", pid);
}

static ssize_t
pid_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n) {
	pid = (u16) simple_strtol (buf, NULL, 10);
	printk("pid = %d\n", pid);
	return n;
}

/****************************************************************************/
/* Class attribute.							    */
/****************************************************************************/
static struct class_attribute sysfs_attrs[] = {
	__ATTR(pid, S_IWUSR | S_IRUGO, pid_show, pid_store),
	__ATTR_NULL,
};

/****************************************************************************/
/* Device model classes.						    */
/****************************************************************************/
static struct class sysfs_class = {
	.name		= "gpio-trigger",
	.owner		= THIS_MODULE,
	.class_attrs	= sysfs_attrs,
};

/****************************************************************************/
/* This function initialize Sysfs					    */
/****************************************************************************/
int sysfs_config (void) {
	printk("sysfs_config !\n");
	class_register(&sysfs_class);
	return 1;
}

/****************************************************************************/
/* This function releases Sysfs.					    */
/****************************************************************************/
void sysfs_release (void) {
	printk("sysfs_release !\n");
	class_unregister(&sysfs_class);
	return;
}

/****************************************************************************/
/* Module init / cleanup block.						    */
/****************************************************************************/
int gpio_init (void) {
	printk("Hello, gpio-trigger!\n");
	gpio_int_config();
	sysfs_config();
	return 0;
}

void gpio_cleanup (void) {
	printk("Goodbye, gpio-trigger\n");
	gpio_int_release();
	sysfs_release();
	return;
}

module_init(gpio_init);
module_exit(gpio_cleanup);

/****************************************************************************/
/* Module licensing/description block.					    */
/****************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);