/* Dare_Dev
 * Sambeau, Beachm, Wasimk95
 * EC 535, Spring 2016
 * Lab 5, 4/11/16
 * Source code for kernel module GPIO & LED counter
 */

#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/jiffies.h> /* jiffies */
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pxafb.h>
#include <asm/arch/pxa-regs.h>
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm-arm/arch/hardware.h>

// Declare Buttons and LEDs
static unsigned int B0 = 101;
static unsigned int B1 = 29;
static unsigned int B2 = 30;
static unsigned int gpioLED0 = 31;
static unsigned int gpioLED1 = 113;
static unsigned int gpioLED2 = 9;
static unsigned int gpioLED3 = 28;

// Function Preprocessors
static ssize_t counter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static ssize_t counter_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void timer_callback(unsigned long data);

// Structure that declares the usual file access functions
struct file_operations counter_fops = {
	write	: counter_write,
	read	: counter_read
};

// Basic counter variables
static struct timer_list *counter;
int value = 15;
int period = 500;
int THRESHOLD = 100;	// Time between button presses for reset to 15
int start_hold = 0; // 0 for hold, 1 for start
int up_down = 0; 	// 0 for up, 1 for down
int brightness = 0; // 0 for highest brightness, 1 for mid brightness, 2 for low brightness
int b0_time = 0;	// time, in jiffies, button 0 was pressed
int b1_time = 0; 	// time, in jiffies, button 1 was pressed

// Interrupt handler for B0: Start/Hold
irqreturn_t gpio_irq0(int irq0, void *dev_id, struct pt_regs *regs)
{
	int delay;
	
	// Get time between last B1 press and current B0 press
	b0_time = jiffies_to_msecs(jiffies);
	delay = (b0_time - b1_time > 0) ? b0_time - b1_time : b1_time - b0_time;

	// If within threshold, reset timer
	if(delay < THRESHOLD) 
	{
		value = 15;
		up_down = (up_down == 1) ? 0 : 1;
		pxa_gpio_set_value(gpioLED0, value & 0x01);
		pxa_gpio_set_value(gpioLED1, value & 0x02);
		pxa_gpio_set_value(gpioLED2, value & 0x04);
		pxa_gpio_set_value(gpioLED3, value & 0x08);
	}
	// Else, switch hold/start
	else
	{
		start_hold = (start_hold == 1) ? 0 : 1;
	}
	return IRQ_HANDLED;
}

// Interrupt handler for B1: Up/Down
irqreturn_t gpio_irq1(int irq1, void *dev_id, struct pt_regs *regs)
{
	int delay;

	// Get time between last B0 press and current B1 press
	b1_time = jiffies_to_msecs(jiffies);
	delay = (b0_time - b1_time > 0) ? b0_time - b1_time : b1_time - b0_time;

	// If within threshold, reset timer
	if(delay < THRESHOLD) 
	{
		value = 15;
		start_hold = (start_hold == 1) ? 0 : 1;
		pxa_gpio_set_value(gpioLED0, value & 0x01);
		pxa_gpio_set_value(gpioLED1, value & 0x02);
		pxa_gpio_set_value(gpioLED2, value & 0x04);
		pxa_gpio_set_value(gpioLED3, value & 0x08);
	}
	// Else, switch up/down mode
	else
	{
		up_down = (up_down == 1) ? 0 : 1;
	}


	if (brightness == 3) brightness = 0;
	return IRQ_HANDLED;
}

// Interrupt handler for B2: Brightness
irqreturn_t gpio_irq2(int irq2, void *dev_id, struct pt_regs *regs)
{
	// Update brightness value
	brightness++;
	brightness = (brightness == 3) ? 0 : brightness;

	// Update duty cycle to match brightness
	if (brightness == 0) PWM_PWDUTY0 = 0x010;
	else if(brightness == 1) PWM_PWDUTY0 = 0x0a0;
	else PWM_PWDUTY0 = 0x3ff;

	return IRQ_HANDLED;
}

void timer_callback(unsigned long data)
{
	// If counter is started, update timer
	if (start_hold == 1)
	{
		// Update value and print it
		(up_down == 0) ? value++ : value --;
		value = (value == 16) ? 0 : value;
		value = (value == -1) ? 15: value;
		pxa_gpio_set_value(gpioLED0, value & 0x01);
		pxa_gpio_set_value(gpioLED1, value & 0x02);
		pxa_gpio_set_value(gpioLED2, value & 0x04);
		pxa_gpio_set_value(gpioLED3, value & 0x08);
	}

	// Update timer
	mod_timer(counter, jiffies + msecs_to_jiffies(period));
}

static int init_counter(void)
{
	// Register the major number
	int result = register_chrdev(61, "mygpio", &counter_fops);
	
	// Assign interrupt number variables to GPIOs
	int irq0 = IRQ_GPIO(B0);
	int irq1 = IRQ_GPIO(B1);
	int irq2 = IRQ_GPIO(B2);

	if (result < 0)
		return result;

	// Initialize GPIOs as inputs
	pxa_gpio_mode(B0);
	pxa_gpio_mode(B1);
	pxa_gpio_mode(B2);
	pxa_gpio_mode(gpioLED0);
	pxa_gpio_mode(gpioLED1);
	pxa_gpio_mode(gpioLED2);
	pxa_gpio_mode(gpioLED3);

	// Set inputs
	gpio_direction_input(B0);
	gpio_direction_input(B1);
	gpio_direction_input(B2);

	// Set Outputs
	gpio_direction_output(gpioLED0, 1);
	gpio_direction_output(gpioLED1, 1);
	gpio_direction_output(gpioLED2, 1);
	gpio_direction_output(gpioLED3, 1);


	// Request interrupt numbers for variables and assign functions
	if (request_irq(irq0, &gpio_irq0, SA_INTERRUPT | SA_TRIGGER_RISING, "mygpio0", NULL) != 0)
	{
		printk("irq0 not acquired.\n");
		return -1;
	}
	if (request_irq(irq1, &gpio_irq1, SA_INTERRUPT | SA_TRIGGER_RISING, "mygpio1", NULL) != 0)
	{
		printk("irq1 not acquired.\n");
		return -1;
	}
	if (request_irq(irq2, &gpio_irq2, SA_INTERRUPT | SA_TRIGGER_RISING, "mygpio2", NULL) != 0)
	{
		printk("irq2 not acquired.\n");
		return -1;
	}

	// Initialize PWM
	pxa_set_cken(CKEN0_PWM0, 1);
	pxa_gpio_mode(GPIO16_PWM0_MD);
	PWM_PERVAL0 = 0x3ff;
	PWM_PWDUTY0 = 0x3ff;

	// Initialize times at least THRESHOLD apart
	b0_time = jiffies_to_msecs(jiffies);
	b1_time = jiffies_to_msecs(jiffies) - THRESHOLD;

	// Initialize timer with period of 1 second
	counter = kmalloc(sizeof(struct timer_list *), GFP_KERNEL);
	setup_timer(counter, timer_callback, 0);
	mod_timer(counter, jiffies + msecs_to_jiffies(period));


	printk("Module Initialized.\n");
	return 0;
}

static void clean_counter(void)
{
	// Free the major number
	unregister_chrdev(61, "mygpio");

	// Set the duty cycle to 0
	PWM_PWDUTY0 = 0;

	// Turn all LEDs off
	pxa_gpio_set_value(gpioLED0, 0);
	pxa_gpio_set_value(gpioLED1, 0);
	pxa_gpio_set_value(gpioLED2, 0);
	pxa_gpio_set_value(gpioLED3, 0);

	// Free requested interrupt numbers
	free_irq(IRQ_GPIO(B0), NULL);
	free_irq(IRQ_GPIO(B1), NULL);
	free_irq(IRQ_GPIO(B2), NULL);

	// Free all GPIO pins
	gpio_free(B0);
	gpio_free(B1);
	gpio_free(B2);
	gpio_free(gpioLED0);
	gpio_free(gpioLED1);
	gpio_free(gpioLED2);
	gpio_free(gpioLED3);

	// Delete Timer
	del_timer(counter);
	kfree(counter);

	printk("Module Exited\n");
}

// Write to module - used to modify count frequency, modify current value, and modify brightness
static ssize_t counter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char *temp;

	// Create a temp buffer for incoming data
	temp = kmalloc(128*sizeof(char), GFP_KERNEL);
	memset(temp, 0, 128);

	// Copy data from the user to temp buffer */
	if (copy_from_user(temp, buf, count))
		return -EFAULT;

	// Get new period from buffer
	if (temp[0] == 's' && temp[2] == '\n') 
	{
		if (temp[1] == '0') period = 4000;
		else if (temp[1] == '1') period = 2000;
		else if (temp[1] == '2') period = 500;		
	}

	// Get new value from buffer
	else if (temp[0] == 'v' && temp[2] == '\n')
	{
		if (temp[1] >= '0' && temp[1] < ':')
			value = temp[1]-'0';
		else if (temp[1] > '`' && temp[1] < 'g')
			value = temp[1]-'`'+9;
		pxa_gpio_set_value(gpioLED0, value & 0x01);
		pxa_gpio_set_value(gpioLED1, value & 0x02);
		pxa_gpio_set_value(gpioLED2, value & 0x04);
		pxa_gpio_set_value(gpioLED3, value & 0x08);
	}

	// Get new brightness from buffer
	else if (temp[0] == 'b' && temp[2] == '\n') 
	{
		if (temp[1] > '0' && temp[1] < '4')
			period = (temp[1]-48)-1;
	}

	return count;
}

// Read from module - read counter information 
static ssize_t counter_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) { 
	char *temp;

	temp = kmalloc(128*sizeof(char), GFP_KERNEL);
	
	sprintf(temp, "%d\n%d\n%d\n%d\n%d\n", value, period, start_hold, up_down, 			brightness);

	if (copy_to_user(buf, temp, count))
		return -EFAULT;

	return count;
}

module_init(init_counter);
module_exit(clean_counter);
