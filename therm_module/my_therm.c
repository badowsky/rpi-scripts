/*
* Basic kernel module to read DS18B20 temperature sensor.
*
* Author:
* Mateusz Badowski (kontakt@mbadowski.pl)
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
#include <linux/time.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>

// -------------------------- GPIO OPERATIONS -------------------------- //

// set GPIO pin g as input 
//#define GPIO_DIR_INPUT(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
// set GPIO pin g as output 
//#define GPIO_DIR_OUTPUT(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
// get logical value from gpio pin g 
///#define GPIO_READ_PIN(g) (*(gpio+13) & (1<<(g))) && 1
// sets   bits which are 1 ignores bits which are 0 
//#define GPIO_SET_PIN(g)	*(gpio+7) = 1<<g;
// clears bits which are 1 ignores bits which are 0 
//#define GPIO_CLEAR_PIN(g) *(gpio+10) = 1<<g;


// set GPIO pin g as input 
//#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
// set GPIO pin g as output 
//#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
//#define OUT_GPIO_HIGH(g)    OUT_GPIO(g); SET_GPIO_HIGH(g)
//#define OUT_GPIO_LOW(g)    OUT_GPIO(g); SET_GPIO_LOW(g)

// get logical value from gpio pin g 
//#define GPIO_READ(g) (*(gpio+13) & (1<<(g))) && 1
// sets   bits which are 1 ignores bits which are 0 
//#define SET_GPIO_HIGH(g)	*(gpio+7) = 1<<g;
// clears bits which are 1 ignores bits which are 0 
//#define SET_GPIO_LOW(g) *(gpio+10) = 1<<g;


//OLD
#define INP_GPIO(g)         gpio_direction_input(g)
#define OUT_GPIO_HIGH(g)    gpio_direction_output(g, 1)
#define OUT_GPIO_LOW(g)     gpio_direction_output(g, 0)

#define SET_GPIO_HIGH(g)    gpio_set_value(g, 1)
#define SET_GPIO_LOW(g)     gpio_set_value(g, 0)
#define GPIO_READ(g)        gpio_get_value(g)

// --------------------------------------------------------------------- //

// --------- ROM COMMANDS ---------- //

#define SEARCH_ROM			0xF0
#define MATCH_ROM			0x55
#define READ_ROM			0x33
#define SKIP_ROM       		0xCC
#define ALARM_SEARCH		0xEC

// --------------------------------- //

// ------- FUNCTION COMMANDS ------- //

#define CONVERT_T       	0x44
#define READ_SCRATCHPAD     0xBE
#define WRITE_SCRATCHPAD    0x4E
#define COPY_SCRATCHPAD     0x48
#define RECAL_E				0xB8
#define READ_POWER_SUPPLY   0xB4

// --------------------------------- //

// ------- SETTINGS ------- //

#define DS_PIN	10

// ------------------------ //

inline void my_delay(int n){
    udelay(n);
}
void resetPulse(void){
    //printk(KERN_INFO "Sending reset pulse.");
	OUT_GPIO_LOW(DS_PIN);
	//SET_GPIO_LOW(DS_PIN);
	my_delay(500);
	SET_GPIO_HIGH(DS_PIN);
    INP_GPIO(DS_PIN);

}

int  initialize(void)
{	int presence;
    //printk(KERN_INFO "Trying to initialize.");
    resetPulse();

	my_delay(70);
	presence = GPIO_READ(DS_PIN);
	my_delay(1000);

	//If presence is 0 then return 1 otherwise 0, so 1 is good...
	return presence ? 0 : 1;
}

// Simple io functions
inline void writeBit(int bit)
{
    if(bit)
    {
    	SET_GPIO_LOW(DS_PIN);
        my_delay(6);//release after max 15us
        SET_GPIO_HIGH(DS_PIN);
        my_delay(64);//leave at least 45us for reading

    }
    else
    {
    	SET_GPIO_LOW(DS_PIN);
        my_delay(60);
        SET_GPIO_HIGH(DS_PIN);
        my_delay(10);
    }
}

inline int readBit(void)
{	//sample should end after 15us since start of pulling low
    int bit;
	OUT_GPIO_LOW(DS_PIN);

	//SET_GPIO_LOW(DS_PIN);
	my_delay(1);
	SET_GPIO_HIGH(DS_PIN);//dunno if this is needed
	INP_GPIO(DS_PIN);
	my_delay(13); //previous delay + this delay should be < 15
 
	bit = GPIO_READ(DS_PIN);
	my_delay(55);
	return bit ? 1 : 0;
}

void writeByte(unsigned char value)
{
    unsigned char Mask=1;
    int i;
    int bit;
    OUT_GPIO_HIGH(DS_PIN);
    for(i=0;i<8;i++)
    {
        bit = (value & Mask)!=0;
        writeBit(bit);
        Mask*=2;
    }
    //my_delay(100);//why?
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

void readDeviceID(void){
    unsigned char adress[8];
    int i;
    if (initialize()){
        writeByte(READ_ROM);
        for(i=0;i<8;i++){
            adress[i] = readByte();
        }
    }
    for(i=0;i<8;i++){
        printk(KERN_INFO "adress[%d]: %x\n", i, adress[i]);
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
	int i;
	for(i=0;i<10;i++){
		printk(KERN_INFO "Try number: %d\n", i+1);
    	readDeviceID();
	}
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
