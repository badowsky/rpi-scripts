//#include<linux/module.h>
//#include<linux/init.h>
//#include<linux/kernel.h>
//#include<linux/slab.h>
//#include <linux/time.h>
//#include <linux/gpio.h>
//#include <linux/delay.h>
//#include <linux/fs.h>
//
//#include <linux/errno.h>
//#include <asm/current.h>
//#include <asm/segment.h>
//#include <asm/uaccess.h>

#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <linux/fs.h> //fops

//#include <asm/uaccess.h> //copy_from_user

//#include<linux/slab.h> //kmalloc

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

#define ENABLE_DELAY        55
#define ENALBE_PULSE	    55

#define MODE_CHAR           1
#define MODE_CMD            0

#define SET_GPIO(g, value)  gpio_set_value(g, value)
#define GPIO_READ(g)        gpio_get_value(g)

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

//int my_open(struct inode *inode,struct file *filep);
//int my_release(struct inode *inode,struct file *filep);
//ssize_t my_read(struct file *filep,char *buff,size_t count,loff_t *offp );
//ssize_t my_write(struct file *filep,const char *buff,size_t len,loff_t *offp );
//
char my_data[80]="hi from kernel";
//void write4Bits(char half_byte);
//void enablePulse(void);
//void writeByte(unsigned char byte, int mode);
//char* byteToTwo(char byte);
//void lcd_init(void);
//void printChar(char character);
//void printString(const char *buff, size_t count);
//void my_delay(int us);
//
//static struct gpio lcd[] = {
//{ 27, GPIOF_OUT_INIT_LOW, "LCD_RS" },
//{ 17, GPIOF_OUT_INIT_LOW, "LCD_E" },
//{ 11, GPIOF_OUT_INIT_LOW, "LCD_D4" },
//{ 9, GPIOF_OUT_INIT_LOW, "LCD_D5" },
//{ 10, GPIOF_OUT_INIT_LOW, "LCD_D6" },
//{ 22, GPIOF_OUT_INIT_LOW, "LCD_D7" },
//};
//
//
//void my_delay(int us){
//	udelay(us);
//
//}
//
//void write4Bits(char half_byte){
//    SET_GPIO(PIN_D4, CHECK_BIT(half_byte, 0));
//    SET_GPIO(PIN_D5, CHECK_BIT(half_byte, 1));
//    SET_GPIO(PIN_D6, CHECK_BIT(half_byte, 2));
//    SET_GPIO(PIN_D7, CHECK_BIT(half_byte, 3));
//    
//    enablePulse(); 
//}
//
//void enablePulse(void){
//    my_delay(ENABLE_DELAY);
//    SET_GPIO(PIN_E, 1);
//    my_delay(ENALBE_PULSE);
//    SET_GPIO(PIN_E, 0);
//    my_delay(ENABLE_DELAY);
//}
//
//void writeByte(unsigned char byte, int mode){
//    //Check to be save of invalid mode
//    unsigned char *bits;
//    if(mode == MODE_CHAR){
//        //OUT_GPIO(PIN_RS, mode);
//        SET_GPIO(PIN_RS, 1);
//    }else if(mode == MODE_CMD){
//        //OUT_GPIO(PIN_RS, mode); 
//        SET_GPIO(PIN_RS, 0);
//    }
//    
//    bits = byteToTwo(byte);
//
//    write4Bits(*bits);
//
//    write4Bits(*(bits+1));
//
//    
//}
//
//char* byteToTwo(char byte){
//        unsigned char i;
//        unsigned char *bits = (unsigned char*) kmalloc(3, GFP_KERNEL);
//        bits[0]=byte>>4;
//        bits[1]=byte<<4;
//        bits[1]=bits[1]>>4;
//        return bits;
//}
//
//void lcd_init(void)
//{
//    //OUT_GPIO(PIN_RS, 0);
//    //OUT_GPIO(PIN_E, 0);
//    //OUT_GPIO(PIN_D4, 0);
//    //OUT_GPIO(PIN_D5, 0);
//    //OUT_GPIO(PIN_D6, 0);
//    //OUT_GPIO(PIN_D7, 0);
//    
//    writeByte(0x33, MODE_CMD);
//    writeByte(0x32, MODE_CMD);
//    writeByte(0x28, MODE_CMD);
//    writeByte(0x0C, MODE_CMD);
//    writeByte(0x06, MODE_CMD);
//    writeByte(0x01, MODE_CMD); 
//}
//
//void setCursorPos(int x, int y){
//    //if ((x >= 0) and (x <= 7)) and (y == 0 or y == 1){
//        unsigned char pos_cmd = CURS_BASE + x + (y * 16);
//        writeByte(pos_cmd, MODE_CMD);
//    //}else{
//        //printk( "Wrong cursor position sent: x = %d, y = %d", x, y);
//    //}
//}
//
//void printChar(char character)
//{   
//    writeByte(character, MODE_CHAR);
//    
//}
//
//void printString(const char *buff, size_t count)
//{   //int len = count -1;
//    int i;
//    for(i=0;i<count;i++){
//        printChar(*(buff+i));
//    }
//    
//}

//  FILE OPERATIONS:



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
	//if ( copy_to_user(buff,my_data,strlen(my_data)) != 0 )
		printk( "Kernel -> userspace copy failed!\n" );
	return 0;//strlen(my_data);

}

ssize_t my_write(struct file *filep, const char *buff, size_t len, loff_t *offp )
{
	/* function to copy user space buffer to kernel space*/
    //char *my_data = (char*)kmalloc(len, GFP_KERNEL);
	//if ( copy_from_user(my_data, buff, len) != 0 )
		printk( "Userspace -> kernel copy failed!\n" );
    //printString(my_data, len);
	return 0;
}

struct file_operations my_fops={
	open: my_open,
	read: my_read,
	write: my_write,
	release: my_release,
};

static int __init chardev_init(void)
{
    int ret = 0;
    printk(KERN_INFO "LCD char_dev registration.");
    ret = gpio_request_one(PIN_E, GPIOF_OUT_INIT_LOW, "LCD_E");
    //ret = gpio_request_array(lcd, ARRAY_SIZE(lcd));
    if (ret) {
        printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
    }

    if(register_chrdev(222, "my_device", &my_fops)){
        printk(KERN_ERR "Register filed.");
    }
    return 0;
}
static void __exit chardev_exit(void)
{
    int i;
    //for(i = 0; i < ARRAY_SIZE(lcd); i++) {
//        gpio_set_value(lcd[i].gpio, 0);
//    }
    // unregister all GPIOs
    //gpio_free_array(lcd, ARRAY_SIZE(lcd));
    gpio_free(PIN_E);
    printk("LCD char_dev unregistration.");
    unregister_chrdev(222, "my_device");
    return ;
}

MODULE_AUTHOR("Mateusz Badowski");
MODULE_DESCRIPTION("LCD 2x16 Char Device");

module_init(chardev_init);
module_exit(chardev_exit);
