/*
 *  chardev.c - Create an input/output character device
 */

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for get_user and put_user */

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include<linux/slab.h> //kmalloc

#include "char_dev.h"
#define SUCCESS 0
#define DEVICE_NAME "char_dev"
#define BUF_LEN 32

/* 
 * Is the device open right now? Used to prevent
 * concurent access into the same device 
 */
static int Device_Open = 0;

/* 
 * The message the device will give when asked 
 */
static char Message[BUF_LEN];

/* 
 * How far did the process reading the message get?
 * Useful if the message is larger than the size of the
 * buffer we get to fill in device_read. 
 */
static char *Message_Ptr;


static char current_len = 0;

#define PIN_RS              27
#define PIN_E               17
#define PIN_D4              11
#define PIN_D5              9
#define PIN_D6              10
#define PIN_D7              22

#define CMD_CLEAR           0x01
#define SHIFT_LEFT          0x18
#define SHIFT_RIGHT         0x1C
#define CURS_BASE           0x80

#define ENABLE_DELAY        550
#define ENALBE_PULSE	    550

#define MODE_CHAR           1
#define MODE_CMD            0

#define SET_GPIO(g, value)  gpio_set_value(g, value)
#define GPIO_READ(g)        gpio_get_value(g)

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


//int my_open(struct inode *inode,struct file *filep);
//int my_release(struct inode *inode,struct file *filep);
//ssize_t my_read(struct file *filep,char *buff,size_t count,loff_t *offp );
//ssize_t my_write(struct file *filep,const char *buff,size_t len,loff_t *offp );

char my_data[80]="hi from kernel";
void write4Bits(char half_byte);
void enablePulse(void);
void writeByte(unsigned char byte, int mode);
char* byteToTwo(char byte);
void lcd_init(void);
void printChar(char character);
void printString(char data[], size_t count);
void my_delay(int us);


static struct gpio lcd[] = {
{ PIN_RS, GPIOF_OUT_INIT_LOW, "LCD_RS" },
{ PIN_E, GPIOF_OUT_INIT_LOW, "LCD_E" },
{ PIN_D4, GPIOF_OUT_INIT_LOW, "LCD_D4" },
{ PIN_D5, GPIOF_OUT_INIT_LOW, "LCD_D5" },
{ PIN_D6, GPIOF_OUT_INIT_LOW, "LCD_D6" },
{ PIN_D7, GPIOF_OUT_INIT_LOW, "LCD_D7" },
};


void my_delay(int us){
	udelay(us);

}

void write4Bits(char half_byte){
    SET_GPIO(PIN_D4, CHECK_BIT(half_byte, 0));
    SET_GPIO(PIN_D5, CHECK_BIT(half_byte, 1));
    SET_GPIO(PIN_D6, CHECK_BIT(half_byte, 2));
    SET_GPIO(PIN_D7, CHECK_BIT(half_byte, 3));
    
    enablePulse(); 
}

void enablePulse(void){
    my_delay(ENABLE_DELAY);
    SET_GPIO(PIN_E, 1);
    my_delay(ENALBE_PULSE);
    SET_GPIO(PIN_E, 0);
    my_delay(ENABLE_DELAY);
}

void writeByte(unsigned char byte, int mode){
    //Check to be save of invalid mode
    unsigned char *bits;
    if(mode == MODE_CHAR){
        //OUT_GPIO(PIN_RS, mode);
        SET_GPIO(PIN_RS, 1);
    }else if(mode == MODE_CMD){
        //OUT_GPIO(PIN_RS, mode); 
        SET_GPIO(PIN_RS, 0);
    }
    
    bits = byteToTwo(byte);

    write4Bits(*bits);

    write4Bits(*(bits+1));

    
}

char* byteToTwo(char byte){
        unsigned char i;
        unsigned char *bits = (unsigned char*) kmalloc(3, GFP_KERNEL);
        bits[0]=byte>>4;
        bits[1]=byte<<4;
        bits[1]=bits[1]>>4;
        return bits;
}

void lcd_init(void)
{
    //OUT_GPIO(PIN_RS, 0);
    //OUT_GPIO(PIN_E, 0);
    //OUT_GPIO(PIN_D4, 0);
    //OUT_GPIO(PIN_D5, 0);
    //OUT_GPIO(PIN_D6, 0);
    //OUT_GPIO(PIN_D7, 0);
    
    writeByte(0x33, MODE_CMD);
    writeByte(0x32, MODE_CMD);
    writeByte(0x28, MODE_CMD);
    writeByte(0x0C, MODE_CMD);
    writeByte(0x06, MODE_CMD);
    writeByte(0x01, MODE_CMD); 
}

void setCursorPos(int x, int y){
    //if ((x >= 0) and (x <= 7)) and (y == 0 or y == 1){
        char pos_cmd = CURS_BASE + x + (y * 16);
        writeByte(pos_cmd, MODE_CMD);
    //}else{
        //printk( "Wrong cursor position sent: x = %d, y = %d", x, y);
    //}
}

void printChar(char character)
{   
    writeByte(character, MODE_CHAR);
    
}

void printString(char data[], size_t count)
{   int len = count -1;
    int i;
    for(i=0;i<len;i++){
        printk(KERN_INFO "Petla %d", i);
        printChar(data[i]);
    }    
}

void printMessage(void)
{   //setCursorPos(0, 0);
    writeByte(CMD_CLEAR, MODE_CMD);
    writeByte(0x80, MODE_CMD);
    int i;
    for(i=0;i<current_len;i++){
        printk(KERN_INFO "Petla %c", Message[i]);
        printChar(Message[i]);
        if(i == 15) writeByte(0x0C, MODE_CMD);
    }
}

/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_open(%p)\n", file);
#endif

	/* 
	 * We don't want to talk to two processes at the same time 
	 */
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	/*
	 * Initialize the message 
	 */
	Message_Ptr = Message;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_release(%p,%p)\n", inode, file);
#endif

	/* 
	 * We're now ready for our next caller 
	 */
	Device_Open--;

	module_put(THIS_MODULE);
	return SUCCESS;
}

/* 
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file *file,	/* see include/linux/fs.h   */
			   char __user * buffer,	/* buffer to be
							 * filled with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/* 
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

#ifdef DEBUG
	printk(KERN_INFO "device_read(%p,%p,%d)\n", file, buffer, length);
#endif

	/* 
	 * If we're at the end of the message, return 0
	 * (which signifies end of file) 
	 */
	if (*Message_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *Message_Ptr) {

		/* 
		 * Because the buffer is in the user data segment,
		 * not the kernel data segment, assignment wouldn't
		 * work. Instead, we have to use put_user which
		 * copies data from the kernel data segment to the
		 * user data segment. 
		 */
		put_user(*(Message_Ptr++), buffer++);
		length--;
		bytes_read++;
	}

#ifdef DEBUG
	printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, length);
#endif

	/* 
	 * Read functions are supposed to return the number
	 * of bytes actually inserted into the buffer 
	 */
	return bytes_read;
}

/* 
 * This function is called when somebody tries to
 * write into our device file. 
 */
static ssize_t
device_write(struct file *file,
	     const char __user * buffer, size_t length, loff_t * offset)
{
	int i;


	printk(KERN_INFO "device_write(%p,%d)", file, length);


	for (i = 0; i < length && i < BUF_LEN; i++)
		get_user(Message[i], buffer + i);
        current_len = length -1;
	Message_Ptr = Message;
        printMessage();

	/* 
	 * Again, return the number of input characters used 
	 */
	return i;
}


/* Module Declarations */

/* 
 * This structure will hold the functions to be called
 * when a process does something to the device we
 * created. Since a pointer to this structure is kept in
 * the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. 
 */
struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */
};

/* 
 * Initialize the module - Register the character device 
 */
int init_module()
{
	int ret_val;
	/* 
	 * Register the character device (atleast try) 
	 */
	ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

	/* 
	 * Negative values signify an error 
	 */
	if (ret_val < 0) {
		printk(KERN_ALERT "%s failed with %d\n",
		       "Sorry, registering the character device ", ret_val);
		return ret_val;
	}

	printk(KERN_INFO "%s The major device number is %d.\n",
	       "Registeration is a success", MAJOR_NUM);
	printk(KERN_INFO "If you want to talk to the device driver,\n");
	printk(KERN_INFO "you'll have to create a device file. \n");
	printk(KERN_INFO "We suggest you use:\n");
	printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
	printk(KERN_INFO "The device file name is important, because\n");
	printk(KERN_INFO "the ioctl program assumes that's the\n");
	printk(KERN_INFO "file you'll use.\n");
        
        int ret = 0;
        printk(KERN_INFO "Pins registration...");
        //ret = gpio_request_one(PIN_E, GPIOF_OUT_INIT_LOW, "LCD_E");
        ret = gpio_request_array(lcd, ARRAY_SIZE(lcd));
        if (ret) {
            printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
        }
	return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
	
        int i;
        for(i = 0; i < ARRAY_SIZE(lcd); i++) {
            gpio_set_value(lcd[i].gpio, 0);
        }
        //unregister all GPIOs
        gpio_free_array(lcd, ARRAY_SIZE(lcd));
	/* 
	 * Unregister the device 
	 */
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

	/* 
	 * If there's an error, report it 
	 */
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Badowski");
MODULE_DESCRIPTION("LCD 2x16 Char Device");