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
#include <asm/div64.h>
#include <asm/types.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

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

#define MAX_READS_SCRATCHPAD	10
#define MAX_READS_DEV_ID	10
#define DS_PIN	10
#define DRIVER_NAME "my_temp"
#define DRIVER_MAJOR 81




#define SUCCESS 0
#define BUF_LEN 25
#define ROM_LEN 8

#define ERROR -666
// ------------------------ //


static int Device_Open = 0;				// Is device open?  Used to prevent multiple access to device 
static char temp[BUF_LEN];				// The temp the device will give when asked 
static char *tempPtr;

unsigned char rom_out[ROM_LEN] = {0x28, 0x8E, 0x09, 0x2F, 0x03, 0x00, 0x00, 0x1A};
unsigned char rom_in[ROM_LEN] = {0x28, 0xDA, 0x15, 0x78, 0x01, 0x00, 0x00, 0xA4};
unsigned char *romPtr;
u8 ScratchPad[9];

static u8 w1_crc8_table[] = {
         0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
         157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
         35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
         190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
         70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
         219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
         101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
         248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
         140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
         17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
         175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
         50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
         202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
         87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
         233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
         116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
 };
// Forward declarations
static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset);
static struct file_operations fops = {
	.read = device_read,
	.open = device_open,
	.release = device_close,
        .write = device_write,
};

// -------------------------- IO --------------------------- //

inline void my_delay(int n){

    udelay(n);

}
void resetPulse(void){

    OUT_GPIO_LOW(DS_PIN);
    my_delay(500);
    SET_GPIO_HIGH(DS_PIN);
    INP_GPIO(DS_PIN);

}

u8  initialize(void){

    u8 presence;

    resetPulse();

	my_delay(70);
	presence = GPIO_READ(DS_PIN);
	my_delay(1000);

	//If presence is 0 then return 1 otherwise 0, so 1 is good...
	return presence ? 0 : 1;
}


inline void writeBit(int bit){

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

inline int readBit(void){

    //sample should end after 15us since start of pulling low
    int bit;
    OUT_GPIO_LOW(DS_PIN);
    my_delay(6);
    SET_GPIO_HIGH(DS_PIN);//dunno if this is needed
    INP_GPIO(DS_PIN);
    my_delay(9); //previous delay + this delay should be < 15

    bit = GPIO_READ(DS_PIN);
    my_delay(55);
    return bit ? 1 : 0;
}

void writeByte(unsigned char value){

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
}

u8 readByte(void){

   u8 Mask=1;
   u8 data=0;

   int i;
   for(i=0;i<8;i++)
     {
       if (readBit())
       data |= Mask;
       Mask*=2;
      }

    return data;
}
// ------------------------- IO END ------------------------ //


// ------------------------- UTILS ------------------------- //
u8 calculate_crc(u8 * data, int len){

    u8 crc = 0;

    while (len--)
	    crc = w1_crc8_table[crc ^ *data++];

    return crc;
}

int convertTemp(){

    u16 reading = ScratchPad[0] + (ScratchPad[1] << 8);
    u16 inv = reading & 0xf000;
    int val = 0;
    //printf("Converting temperature from:\nMSB: %x\nLSB: %x\n", ScratchPad[1], ScratchPad[0]);
    if (inv == 0xf000){
        reading = (reading ^ 0xffff) + 1;
        val = - reading*1000/16;
    }else{
    	val =  reading*1000/16;
    }
    //printf("Converted temp: %d \n", val);
    return val;
}
// ----------------------- UTILS END ----------------------- //

// ------------------ OPERATIONS ON DEVICE ----------------- //
void readDeviceID(void){

    u8 address[8];
    int i, j;

    for(i=0;i<MAX_READS_DEV_ID;i++){
        if (initialize()){

            writeByte(READ_ROM);

            for(j=0;j<8;j++){
                address[j] = readByte();
            }
        }
        for(j=0;j<8;j++){
            printk(KERN_INFO "Address[%d]: %02X\n", j, address[j]);
        }
        if(address[7] == calculate_crc(address, 7)){
            printk(KERN_INFO "Readed address crc(%02X) OK (in %d try).", address[7], i);
        return;
        }
    }
}

int letConvertTemp(unsigned char rom[8]){

    if(initialize())
    {   int i;
        writeByte(SKIP_ROM);
        //for(i=0;i<8;i++){
        //    writeByte(rom[i]);
        //}
        writeByte(CONVERT_T);
        
        for(i=0;i<8;i++)
        {
            mdelay(100);//wait 100ms - min Tconv
            if(readBit())
                return 1;
        }
    }
    return 0;
}

int readScratchPad(unsigned char rom[8]){

    int i, j;

    for(i=0; i<MAX_READS_SCRATCHPAD;i++){
        if(initialize())
        {	
            //Address device
            writeByte(MATCH_ROM);
            for(j=0;j<8;j++){
                writeByte(rom[j]);
            }

            //Tell we want to read scratchpad
            writeByte(READ_SCRATCHPAD);

            //Read it
            for(j=0;j<9;j++)
            {
                ScratchPad[j]=readByte();
            }

            //Check if scratchpad was transfered correctly
            if(ScratchPad[8] == calculate_crc(ScratchPad, 8)){
                    for(j=0;j<9;j++)
                            printk(KERN_INFO "ScratchPad[%d]: %02X\n", j, ScratchPad[j]);
                //If yes, skip other loops and return success
                printk(KERN_INFO "Readed in %d try.\n", i);
                return 1;
            }
        }
    }
    //Printed if all loops will be spent without success
    printk(KERN_INFO "Read scratchpad failed... :<\n");
    return 0;
}

// --------------- OPERATIONS ON DEVICE - END --------------- //

int readTemp(unsigned char rom[8]){
    if (!letConvertTemp(rom))
        return ERROR;
    if (!readScratchPad(rom))
        return ERROR;

    int converted_temp = convertTemp();

    printk(KERN_INFO "Converted temperature: %d\n", converted_temp);

    return converted_temp;
}


static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
        return -EBUSY;
    try_module_get(THIS_MODULE);		//Increase use count
    Device_Open++;
    int read_temp_in = readTemp(rom_in);
    int read_temp_out = readTemp(rom_out);
    sprintf(temp, "IN: ");
    if (read_temp_in == ERROR){
        sprintf(temp, "Failed.");
    }else{
        sprintf(temp, "%d", read_temp_in);
    }
    sprintf(temp, "\nOUT: ");
    if (read_temp_in == ERROR){
        sprintf(temp, "Failed.");
    }else{
        sprintf(temp, "%d", read_temp_out);
    }
    tempPtr = temp;
    return SUCCESS;
}

static ssize_t device_read(struct file *filp,	// see include/linux/fs.h   
			   char *buffer,	// buffer to fill with data 
			   size_t length,	// length of the buffer     
			   loff_t * offset)
{
        //printing rom connected device
        //readDeviceID();
	// Number of bytes actually written to the buffer 
	int bytes_read = 0;

	// If we're at the end of the message, return 0 signifying end of file 
	if (*tempPtr == 0)
		return 0;

	// Actually put the data into the buffer 
	while (length && *tempPtr) {

		// The buffer is in the user data segment, not the kernel  segment so "*" assignment won't work.  We have to use 
		// put_user which copies data from the kernel data segment to the user data segment. 
		put_user(*(tempPtr++), buffer++);

		length--;
		bytes_read++;
	}

	// Return the number of bytes put into the buffer
	return bytes_read;
}
static ssize_t device_write(struct file *file,
                            const char __user * buffer,
                            size_t length,
                            loff_t * offset)
{
    //int i, j;
    //int buffer_index = i*2+j;
    //unsigned char string_rom[2];
    //unsigned char *string_romPtr;
    //printk(KERN_INFO "device_write(length: %d)", length);
    //int current_len = length - 1;
    //for (i = 0; buffer_index < current_len && buffer_index < BUF_LEN; i++){
    //    for(j = 0; j<2; j++){
    //        get_user(string_rom[j], buffer + buffer_index);
    //    }
    //    string_romPtr=string_rom;
    //    rom[i] = (unsigned char)kstrtol(string_romPtr, NULL, 16);
    //}

 
        
    
    
    romPtr = rom;


    /* 
     * Again, return the number of input characters used 
     * decresed by one because of resending if return < sent
     */
    printk(KERN_INFO "Returning: %d)", length);
    return length;
}

static int device_close(struct inode *inode, struct file *file)
{
	// Decrement the usage count, or else once you opened the file, you'll never get get rid of the module. 
	module_put(THIS_MODULE);	
	Device_Open--;


	printk(KERN_INFO DRIVER_NAME ": Device release my_therm.\n");

	return 0;
}

/*
* Module init function
*/
static int __init my_therm_init(void)
{
    int result = 0;
    printk(KERN_INFO "%s\n", __func__);
    // register, turn off
    result = gpio_request_one(DS_PIN, GPIOF_OUT_INIT_LOW, "MOJ_DS");
    if (result) {
        printk(KERN_ERR "Unable to request GPIO: %d\n", result);
        return result;
    }
    
    result = register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &fops);

	if (result < 0) {
	  printk(KERN_ALERT DRIVER_NAME "Registering DS18B20 driver failed with %d\n", result);
	  return result;
	}
    return result;
}
/*
* Module exit function
*/
static void __exit my_therm_exit(void)
{
    printk(KERN_INFO "%s\n", __func__);
    // turn DS_PIN off
    gpio_set_value(DS_PIN, 0);
    // unregister GPIO
    gpio_free(DS_PIN);
    // Unregister the driver 
    unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
    printk(DRIVER_NAME ": cleaned up module\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateusz Badowski");
MODULE_DESCRIPTION("Basic kernel module using GPIOs to get DS18B20 measure.");
module_init(my_therm_init);
module_exit(my_therm_exit);