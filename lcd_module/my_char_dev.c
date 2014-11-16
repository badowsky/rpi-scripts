#include<linux/module.h>
#include<linux/init.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include"my_char_dev.h"

#define PIN_RS
#define PIN_E
#define PIN_D4
#define PIN_D5
#define PIN_D6
#define PIN_D7

#define ENABLE_DELAY        
#define ENALBE_PULSE

#define MODE_CHAR           1
#define MODE_CMD            0

#define INP_GPIO(g)         gpio_direction_input(g)
#define OUT_GPIO(g, value)  gpio_direction_output(g, value)
#define SET_GPIO_HIGH(g)    gpio_set_value(g, 1)
#define SET_GPIO_LOW(g)     gpio_set_value(g, 0)
#define GPIO_READ(g)        gpio_get_value(g)

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)

static int chardev_init(void);
static void chardev_exit(void);


char my_data[80]="hi from kernel";

void write4Bits(byte)
{
    SET_GPIO_LOW(PIN_D4);
    SET_GPIO_LOW(PIN_D5);
    SET_GPIO_LOW(PIN_D6);
    SET_GPIO_LOW(PIN_D7);
    if(CHECK_BIT(byte, 0)) SET_GPIO_HIGH(PIN_D4);
    if(CHECK_BIT(byte, 1)) SET_GPIO_HIGH(PIN_D5);
    if(CHECK_BIT(byte, 2)) SET_GPIO_HIGH(PIN_D6);
    if(CHECK_BIT(byte, 3)) SET_GPIO_HIGH(PIN_D7);
    
    enablePulse(); 
}

void enablePulse(void)
{
    write4Bits(*bits);
    delay(ENABLE_DELAY);
    SET_GPIO_HIGH(PIN_E)
    delay(ENALBE_PULSE);
    SET_GPIO_LOW(PIN_E)
    delay(ENABLE_DELAY);
}

void writeByte(unsigned char byte, int mode)
{   //Check to be save of invalid mode
    unsigned char *bits;
    if(mode == MODE_CHAR){
        #OUT_GPIO(PIN_RS, mode);
        SET_GPIO_HIGH(PIN_RS);
    }else if(mode == MODE_CMD){
        #OUT_GPIO(PIN_RS, mode); 
        SET_GPIO_LOW(PIN_RS);
    }
    
    bits = byteToTwo(byte);

    write4Bits(*bits)

    write4Bits(*(bits+1));

    
}

char* byteToTwo(char byte){
        unsigned char i;
        unsigned char *bits;
        bits=(char *)malloc(2);
        bits[0]=byte>>4;
        bits[1]=byte<<4;
        bits[1]=bits[1]>>4;
        return bits;
}

void lcd_init(void)
{
    OUT_GPIO(PIN_RS, 0);
    OUT_GPIO(PIN_E, 0);
    OUT_GPIO(PIN_D4, 0);
    OUT_GPIO(PIN_D5, 0);
    OUT_GPIO(PIN_D6, 0);
    OUT_GPIO(PIN_D7, 0);
    
    writeByte(0x33, MODE_CMD);
    writeByte(0x32, MODE_CMD);
    writeByte(0x28, MODE_CMD);
    writeByte(0x0C, MODE_CMD);
    writeByte(0x06, MODE_CMD);
    writeByte(0x01, MODE_CMD);
    
    
    
}

void printChar(char character)
{   
    writeByte(character);
    
}

void printString(const char *buff, size_t count)
{
    int i;
    for(i=0;i<count;i++){
        printChar(*(buff+i));
    }
    
}



struct file_operations my_fops={
	open: my_open,
	read: my_read,
	write: my_write,
	release: my_release,
};

int my_open(struct inode *inode, struct file *filep)
{
	/*MOD_INC_USE_COUNT;*/ /* increments usage count of module */
	return 0;
}

int my_release(struct inode *inode, struct file *filep)
{
	/*MOD_DEC_USE_COUNT;*/ /* decrements usage count of module */
	return 0;
}

ssize_t my_read(struct file *filep, char *buff, size_t count, loff_t *offp )
{
	/* function to copy kernel space buffer to user space*/
	if ( copy_to_user(buff,my_data,strlen(my_data)) != 0 )
		printk( "Kernel -> userspace copy failed!\n" );
	return strlen(my_data);

}

ssize_t my_write(struct file *filep, const char *buff, size_t count, loff_t *offp )
{
	/* function to copy user space buffer to kernel space*/
	if ( copy_from_user(my_data,buff,count) != 0 )
		printk( "Userspace -> kernel copy failed!\n" );
    printString(buff, count);
	return 0;
}


static int chardev_init(void)
{
printk(KERN_INFO "LCD char_dev registration.");
if(register_chrdev(222, "my_device", &my_fops)){
	printk(KERN_ERR "Register filed.");
}
return 0;
}
static void chardev_exit(void)
{
printk("LCD char_dev unregistration.");
unregister_chrdev(222, "my_device");
return ;
}

MODULE_AUTHOR("Mateusz Badowski");
MODULE_DESCRIPTION("LCD 2x16 Char Device");

module_init(chardev_init);
module_exit(chardev_exit);