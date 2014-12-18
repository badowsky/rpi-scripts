/* dht11km.c
 *
 * dht11km - Device driver for reading values from DHT11 temperature and humidity sensor.
 *
 *			 By default the DHT11 is connected to GPIO pin 0 (pin 3 on the GPIO connector)
 *           The Major version default is 80 but can be set via the command line.
 *			 Command line parameters: gpio_pin=X - a valid GPIO pin value
 *					                  driverno=X - value for Major driver number
 *									  format=X - format of the output from the sensor
 *
 * Usage:
 *        Load driver: 	insmod ./dht11km.ko <optional variables>
 *				i.e.   	insmod ./dht11km.ko gpio_pin=2 format=3
 *
 *		  Set up device file to read from (i.e.):
 *						mknod /dev/dht11 c 80 0
 *						mknod /dev/myfile c <driverno> 0	- to set the output to your own file and driver number
 *
 *		  To read the values from the sensor: cat /dev/dht11
 *
 * Copyright (C) 2012 Nigel Morton <nigel@ntpworld.co.uk>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>
#include <linux/irq.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>

#include <linux/fs.h>
#include <asm/uaccess.h>	// for put_user 

// include RPi harware specific constants 
#include <mach/hardware.h>

#define DRIVER_NAME "dht11"
#define RBUF_LEN 256
#define SUCCESS 0
#define BUF_LEN 80		// Max length of the message from the device 

// set GPIO pin g as input 
#define GPIO_DIR_INPUT(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
// set GPIO pin g as output 
#define GPIO_DIR_OUTPUT(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
// get logical value from gpio pin g 
#define GPIO_READ_PIN(g) (*(gpio+13) & (1<<(g))) && 1
// sets   bits which are 1 ignores bits which are 0 
#define GPIO_SET_PIN(g)	*(gpio+7) = 1<<g;
// clears bits which are 1 ignores bits which are 0 
#define GPIO_CLEAR_PIN(g) *(gpio+10) = 1<<g;
// Clear GPIO interrupt on the pin we use 
#define GPIO_INT_CLEAR(g) *(gpio+16) = (*(gpio+16) | (1<<g));
// GPREN0 GPIO Pin Rising Edge Detect Enable/Disable 
#define GPIO_INT_RISING(g,v) *(gpio+19) = v ? (*(gpio+19) | (1<<g)) : (*(gpio+19) ^ (1<<g))
// GPFEN0 GPIO Pin Falling Edge Detect Enable/Disable 
#define GPIO_INT_FALLING(g,v) *(gpio+22) = v ? (*(gpio+22) | (1<<g)) : (*(gpio+22) ^ (1<<g))

// module parameters 
static int sense = 0;
static struct timeval last_time = {0, 0};

static spinlock_t lock;

// Forward declarations
static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static void clear_interrupts(void);

void read_sensor(void);
int check_result(void);

// Global variables are declared as static, so are global within the file. 
static int Device_Open = 0;				// Is device open?  Used to prevent multiple access to device 
static char msg[BUF_LEN];				// The msg the device will give when asked 
static char *msg_Ptr;
static spinlock_t lock;
static unsigned int bitcount=0;
static unsigned int bytecount=0;
static unsigned int started=0;			//Indicate if we have started a read or not
static unsigned char dht[5];			// For result bytes
static int format = 0;		//Default result format
static int gpio_pin = 23;		//Default GPIO pin
static int driverno = 80;		//Default driver number

//Operations that can be performed on the device
static struct file_operations fops = {
	.read = device_read,
	.open = device_open,
	.release = device_close
};


volatile unsigned *gpio;

// IRQ handler - where the timing takes place
static irqreturn_t irq_handler(int i, void *blah, struct pt_regs *regs)
{
	struct timeval time;
	long delta_t;
	int data_time = 0;
	int pin_state;

	// use the GPIO pin_state level 
	pin_state = GPIO_READ_PIN(gpio_pin);

	/* reset interrupt */
	GPIO_INT_CLEAR(gpio_pin);

	if (sense != -1) {
		// get current time 
		do_gettimeofday(&time);
		// time since last interrupt in microseconds 
		delta_t = time.tv_sec-last_time.tv_sec;
			
		data_time = (int) (delta_t*1000000 + (time.tv_usec - last_time.tv_usec)); // ************************* przetestowac bez duplikowania
		last_time = time;	//Save last interrupt time
		
		if((pin_state == 1)&(data_time > 40))
			{
			started = 1;
			return IRQ_HANDLED;	
			}
			
		if((pin_state == 0)&(started==1))
			{
			if(data_time > 80)
				return IRQ_HANDLED;										//skip cause of Start/spurious? signal
			if(data_time < 15)
				return IRQ_HANDLED;										//skip cause of Spurious signal?
			if (data_time > 60)//55 
				dht[bytecount] = dht[bytecount] | (0x80 >> bitcount);	//Add a 1 to the data byte
			
			//Uncomment to log bits and durations - may affect performance and not be accurate!
			//printk("B:%d, d:%d, dt:%d\n", bytecount, bitcount, data);
			bitcount++;
			if(bitcount == 8)
				{
				bitcount = 0;
				bytecount++;
				}
			//if(bytecount == 5)
			//	printk(KERN_INFO DRIVER_NAME "Result: %d, %d, %d, %d, %d\n", dht[0], dht[1], dht[2], dht[3], dht[4]);
			}
		}
	return IRQ_HANDLED;
}

static int setup_interrupts(void)
{
	int result;
	unsigned long flags;

	result = request_irq(INTERRUPT_GPIO0, (irq_handler_t) irq_handler, 0, DRIVER_NAME, (void*) gpio);

	switch (result) {
	case -EBUSY:
		printk(KERN_ERR DRIVER_NAME ": IRQ %d is busy\n", INTERRUPT_GPIO0);
		return -EBUSY;
	case -EINVAL:
		printk(KERN_ERR DRIVER_NAME ": Bad irq number or handler\n");
		return -EINVAL;
	default:
		printk(KERN_INFO DRIVER_NAME	": Interrupt %04x obtained\n", INTERRUPT_GPIO0);
		break;
	};

	spin_lock_irqsave(&lock, flags);

	// GPREN0 GPIO Pin Rising Edge Detect Enable 
	GPIO_INT_RISING(gpio_pin, 1);
	// GPFEN0 GPIO Pin Falling Edge Detect Enable 
	GPIO_INT_FALLING(gpio_pin, 1);

	// clear interrupt flag 
	GPIO_INT_CLEAR(gpio_pin);

	spin_unlock_irqrestore(&lock, flags);

	return 0;
}

// Clear the GPIO edge detect interrupts
static void clear_interrupts(void)
{
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);

	// GPREN0 GPIO Pin Rising Edge Detect Disable 
	GPIO_INT_RISING(gpio_pin, 0);

	// GPFEN0 GPIO Pin Falling Edge Detect Disable 
	GPIO_INT_FALLING(gpio_pin, 0);

	spin_unlock_irqrestore(&lock, flags);

	free_irq(INTERRUPT_GPIO0, (void *) gpio);
}

// Initialise GPIO memory
static int init_port(void)
{
	// reserve GPIO memory region. 
	if (request_mem_region(GPIO_BASE, SZ_4K, DRIVER_NAME) == NULL) {
		printk(KERN_ERR DRIVER_NAME ": unable to obtain GPIO I/O memory address\n");
		return -EBUSY;
	}

	// remap the GPIO memory 
	if ((gpio = ioremap_nocache(GPIO_BASE, SZ_4K)) == NULL) {
		printk(KERN_ERR DRIVER_NAME ": failed to map GPIO I/O memory\n");
		return -EBUSY;
	}

	return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
	char result_validity[3];			//To say if the result is trustworthy or not
	int retry = 0;
	
	if (Device_Open)
		return -EBUSY;

	try_module_get(THIS_MODULE);		//Increase use count

	Device_Open++;

    //printk(KERN_INFO DRIVER_NAME " Start setup (device_open)\n");
    int i;
    for(i=0;i<5;i++){
        read_sensor();
        if (check_result() == 0){
            // If result is OK, notice it and break the loop
            sprintf(result_validity, "OK");
            break;
        }else{
            // If result is bad, notice it and prepare for another read
            sprintf(result_validity, "BAD");
            clear_interrupts();
            mdelay(1100);
        }
    }
	
	//Check if the read results are valid. If not then try again!
//	if((dht[0] + dht[1] + dht[2] + dht[3] == dht[4]) & (dht[4] > 0))
//		sprintf(result, "OK");
//	else
//		{
//		retry++;
//		sprintf(result, "BAD");
//		if(retry == 5)
//			goto return_result;		//We tried 5 times so bail out
//		clear_interrupts();
//		mdelay(1100);				//Can only read from sensor every 1 second so give it time to recover
//		goto start_read;
//		}

		//Return the result in various different formats
	switch(format){
		case 0:
			sprintf(msg, "Values: %d, %d, %d, %d, %d, %s\n", dht[0], dht[1], dht[2], dht[3], dht[4], result_validity);
			break;
		case 1:
			sprintf(msg, "%0X,%0X,%0X,%0X,%0X,%s\n", dht[0], dht[1], dht[2], dht[3], dht[4], result_validity);
			break;
		case 2:
			sprintf(msg, "%02X%02X%02X%02X%02X%s\n", dht[0], dht[1], dht[2], dht[3], dht[4], result_validity);
			break;
		case 3:
			sprintf(msg, "Temperature: %dC\nHumidity: %d%%\nResult:%s\n", dht[0], dht[2], result_validity);
			break;
		
	}
	msg_Ptr = msg;

	return SUCCESS;
}

static ssize_t device_read(struct file *filp,	// see include/linux/fs.h   
			   char *buffer,	// buffer to fill with data 
			   size_t length,	// length of the buffer     
			   loff_t * offset)
{
	// Number of bytes actually written to the buffer 
	int bytes_read = 0;

	// If we're at the end of the message, return 0 signifying end of file 
	if (*msg_Ptr == 0)
		return 0;

	// Actually put the data into the buffer 
	while (length && *msg_Ptr) {

		// The buffer is in the user data segment, not the kernel  segment so "*" assignment won't work.  We have to use 
		// put_user which copies data from the kernel data segment to the user data segment. 
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	// Return the number of bytes put into the buffer
	return bytes_read;
}

static int device_close(struct inode *inode, struct file *file)
{
	// Decrement the usage count, or else once you opened the file, you'll never get get rid of the module. 
	module_put(THIS_MODULE);	
	Device_Open--;

	clear_interrupts();
	//printk(KERN_INFO DRIVER_NAME ": Device release (device_close)\n");

	return 0;
}

void read_sensor(void)
{
	started = 0;
	bitcount = 0;
	bytecount = 0;
    dht[0] = 0;
	dht[1] = 0;
	dht[2] = 0;
	dht[3] = 0;
	dht[4] = 0;
	GPIO_DIR_OUTPUT(gpio_pin); 	// Set pin to output
    GPIO_CLEAR_PIN(gpio_pin);	// Set low
    mdelay(20);					// DHT11 needs min 18mS to signal a startup
    GPIO_SET_PIN(gpio_pin);		// Take pin high
    udelay(40);					// Stay high for a bit before swapping to read mode
    GPIO_DIR_INPUT(gpio_pin); 	// Change to read
	
	//Start timer to time pulse length
	do_gettimeofday(&last_time);
	
	// Set up interrupts
	setup_interrupts();
	
	//Give the dht11 time to reply
	mdelay(10);
}
int check_result(void)
{
    	//Check if the read results are valid. If not then try again!
	if((dht[0] + dht[1] + dht[2] + dht[3] == dht[4]) & (dht[4] > 0))
    {
        return 0;
    }
	else
	{
	    return -1;
	}
}
/*
* Module init function
*/
static int __init dht11_init(void)
{
	int result;
	int i;
	
    result = register_chrdev(driverno, DRIVER_NAME, &fops);

	if (result < 0) {
	  printk(KERN_ALERT DRIVER_NAME "Registering dht11 driver failed with %d\n", result);
	  return result;
	}

	printk(KERN_INFO DRIVER_NAME ": driver registered!\n");

	result = init_port();
	if (result < 0)
		return result;

	return 0;

}
/*
* Module exit function
*/
static void __exit dht11_exit(void)
{
	// release mapped memory and allocated region 
	if(gpio != NULL) {
		iounmap(gpio);
		release_mem_region(GPIO_BASE, SZ_4K);
		printk(KERN_INFO DRIVER_NAME ": cleaned up resources\n");
	}

	// Unregister the driver 
	unregister_chrdev(driverno, DRIVER_NAME);
	printk(KERN_INFO DRIVER_NAME ": cleaned up module\n");
}
module_init(dht11_init);
module_exit(dht11_exit);

MODULE_DESCRIPTION("DHT11 temperature/humidity sendor driver for Raspberry Pi GPIO.");
MODULE_AUTHOR("Mateusz Badowski");
MODULE_LICENSE("GPL");

// Command line paramaters for gpio pin and driver major number
module_param(format, int, S_IRUGO);
MODULE_PARM_DESC(format, "Format of output");
module_param(gpio_pin, int, S_IRUGO);
MODULE_PARM_DESC(gpio_pin, "GPIO pin to use");
module_param(driverno, int, S_IRUGO);
MODULE_PARM_DESC(driverno, "Driver handler major value");

