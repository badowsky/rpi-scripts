/*
* Basic kernel module using a timer and GPIOs to flash a LED.
*
* Author:
* Stefan Wendler (devnull@kaltpost.de)
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
*/
#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>

#define INP_GPIO(g)         gpio_direction_input(g)
#define OUT_GPIO(g)         gpio_direction_output(g, 1)
#define SET_GPIO_HIGH(g)    gpio_set_value(g, 1)
#define SET_GPIO_LOW(g)     gpio_set_value(g, 0)
#define GPIO_READ(g)        gpio_get_value(g)

#define DS_PIN	10

/*
* Timer function called periodically
*/
static void blink_timer_func(unsigned long data)
{
    printk(KERN_INFO "%s\n", __func__);
    gpio_set_value(LED1, data);
    /* schedule next execution */
    blink_timer.data = !data; // makes the LED toggle
    blink_timer.expires = jiffies + (1*HZ); // 1 sec.
    add_timer(&blink_timer);
}

void my_delay(int n){
    
}
void resetPulse(void){
    printk(KERN_INFO "Sending reset pulse.");
	OUT_GPIO(DS_PIN);
	// pin low for 480 us
	SET_GPIO_LOW(DS_PIN)
	my_delay(480);
    INP_GPIO(DS_PIN);
    my_delay(60);
}

int  initialize(void)
{
    printk(KERN_INFO "Trying to initialize.");
    int loop;

    resetPulse();
    if(GPIO_READ(DS_PIN)==0)
    {   
        my_delay(300);//Sprobowac dla 420
        printk(KERN_INFO "Initialize succesfull.\n");
        return 1;
    }
    printk(KERN_INFO "Initialize failed.\n");
    return 0;
}

// Simple io functions
inline void writeBit(int bit){
    SET_GPIO_LOW(DS_PIN);
	if(bit)
    {
        my_delay(1);
        SET_GPIO_HIGH(DS_PIN);
        my_delay(59);

    }
    else
    {
        my_delay(60);
        SET_GPIO_HIGH(DS_PIN);
    }
}

inline int readBit(void)
{
    OUT_GPIO(DS_PIN);
    // PIN LOW
    SET_GPIO_LOW(DS_PIN);
    my_delay(5);
    // set INPUT
    INP_GPIO(DS_PIN);
    my_delay(10);
    if(GPIO_READ(DS_PIN)!=0){
        my_delay(45);
        return 1;
    }
    my_delay(45);
    return 0;
    
}

void writeByte(unsigned char value)
{
    unsigned char Mask=1;
    int i;
    int bit;
    OUT_GPIO(DS_PIN);
    for(i=0;i<8;i++)
    {
        bit = (value & Mask)!=0;
        writeBit(bit);
        Mask*=2;
    }


    my_delay(100);
}

unsigned char readByte(void)
{

   unsigned char Mask=1;
   unsigned char data=0;

   int i;
   for(i=0;i<8;i++)
     {
       if (readBit())
       data |= Mask;
       Mask*=2;
      }

    return data;
}

void readDeviceAdress(void){
    unsigned char addres[8];
    int i;
    if (initialize()){
        writeByte(READ_ROM);
        for(i=0;i<8;i++){
            addres[i] = readByte();
        }
    }
    for(i=0;i<8;i++){
        printk(KERN_INFO "%x", addres[i]);
    }
}
/*
* Module init function
*/
static int __init gpiomod_init(void)
{
    int ret = 0;
    printk(KERN_INFO "%s\n", __func__);
    // register, turn off
    ret = gpio_request_one(DS_PIN, GPIOF_OUT_INIT_LOW, "MOJ_DS");
    if (ret) {
        printk(KERN_ERR "Unable to request GPIOs: %d\n", ret);
        return ret;
    }
	
    readDeviceAdress();
    return ret;
}
/*
* Module exit function
*/
static void __exit gpiomod_exit(void)
{
    printk(KERN_INFO "%s\n", __func__);
    // turn DS_PIN off
    gpio_set_value(DS_PIN, 0);
    // unregister GPIO
    gpio_free(DS_PIN);
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Wendler");
MODULE_DESCRIPTION("Basic kernel module using a timer and GPIOs to flash a LED.");
module_init(gpiomod_init);
module_exit(gpiomod_exit);