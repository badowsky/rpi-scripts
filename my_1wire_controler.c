#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GPIO_READ(g)  (*(gpio + 13) &= (1<<(g)))

// ROM COMMANDS
#define SEARCH_ROM						0xF0
#define MATCH_ROM						0x55
#define READ_ROM						0x33
#define SKIP_ROM       					0xCC
#define ALARM_SEARCH					0xEC

// FUNCTION COMMANDS
#define CONVERT_T       		0x44
#define READ_SCRATCHPAD         0xBE
#define WRITE_SCRATCHPAD        0x4E
#define COPY_SCRATCHPAD         0x48
#define RECAL_E					0xB8
#define READ_POWER_SUPPLY		0xB4

unsigned char ScratchPad[9];
double  temperature;
int   resolution;

void setup_io();


#define  DS_PIN  10
#define DELAY1US  smalldelay();


void resetPulse(void){
	INP_GPIO(DS_PIN);
	OUT_GPIO(DS_PIN);
	// pin low for 480 us
	GPIO_CLR=1<<DS_PIN;
	usleep(480);
}


int  initialize(void)
{
 int loop;

   INP_GPIO(DS_PIN);//sprobowac bez
   usleep(1000);//sprobowac bez

   resetPulse();

   INP_GPIO(DS_PIN);
   usleep(60);
   if(GPIO_READ(DS_PIN)==0)
   {
     usleep(380);//Sprobowac dla 420
     return 1;
   }
 
  return 0;
}


void smalldelay(void)
{
  int loop2;
   for(loop2=0;loop2<100;loop2++);
}


// Simple io functions
void writeBit(int bit){
	if(bit)
	        {
	           DELAY1US
	           INP_GPIO(DS_PIN);
	           usleep(60);

	        }
	        else
	        {
	           usleep(60);
	           INP_GPIO(DS_PIN);
	           usleep(1);
	        }
}

void writeByte(unsigned char value)
{
  unsigned char Mask=1;
  int i;
  int bit;

   for(i=0;i<8;i++)
     {
       INP_GPIO(DS_PIN);
       OUT_GPIO(DS_PIN);
       //pin low
       GPIO_CLR= 1 <<DS_PIN;
       bit = (value & Mask)!=0;
       writeBit(bit)

      Mask*=2;
      usleep(60);
    }


   usleep(100);
}

int readBit(void)
{
   INP_GPIO(DS_PIN);
   OUT_GPIO(DS_PIN);
   // PIN LOW
   GPIO_CLR= 1 << DS_PIN;
   DELAY1US
   // set INPUT
   INP_GPIO(DS_PIN);
   DELAY1US
   DELAY1US
   DELAY1US
   if(GPIO_READ(DS_PIN)!=0)
     return 1;
   return 0;
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
       usleep(60);
      }

    return data;
}




int ReadScratchPad(void)
{
   int loop;

    if(initialize())
     {
       writeByte(SKIP_ROM);
       writeByte(READ_SCRATCHPAD);
       for(loop=0;loop<9;loop++)
         {
          ScratchPad[loop]=readByte();
        }
    return 1;
  }
  return 0;
}

unsigned char  CalcCRC(unsigned char * data, unsigned char  byteSize)
{
   unsigned char  shift_register = 0;
   unsigned char  loop,loop2;
   char  DataByte;

   for(loop = 0; loop < byteSize; loop++)
   {
      DataByte = *(data + loop);
      for(loop2 = 0; loop2 < 8; loop2++)
      {
         if((shift_register ^ DataByte)& 1)
         {
            shift_register = shift_register >> 1;
            shift_register ^=  0x8C;
         }
         else
            shift_register = shift_register >> 1;
         DataByte = DataByte >> 1;
      }
   }
   return shift_register;
}

int ReadSensor(void)
{
  int maxloop;
  int RetryCount;
  int loop;
  unsigned char  CRCByte;
  union {
   short SHORT;
   unsigned char CHAR[2];
  }IntTemp;


  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  temperature=-9999.9;

  for(RetryCount=0;RetryCount<3;RetryCount++)
  {
	  printf("Try number %d", RetryCount);

	  if(!initialize()) continue;

	  // start a conversion
	  writeByte(SKIP_ROM);
	  writeByte(CONVERT_T);


	  maxloop=0;
	  // wait until ready
	  while(!readBit())
	  {
		  putchar('.');
		  maxloop++;
		  if(maxloop>100000) break;
	  }

	  if(maxloop>100000) continue;


	  if(!ReadScratchPad()) continue;

		 for(loop=0;loop<9;loop++)
			 printf("%02X ",ScratchPad[loop]);
		 printf("\n");fflush(stdout);

	  // OK Check sum Check;
	  CRCByte= CalcCRC(ScratchPad,8);

	  if(CRCByte!=ScratchPad[8]) continue;

	  //Check Resolution
	  resolution=0;
	  switch(ScratchPad[4])
	  {
	  	  case  0x1f: resolution=9;break;
	  	  case  0x3f: resolution=10;break;
	  	  case  0x5f: resolution=11;break;
	  	  case  0x7f: resolution=12;break;
	  }

	  if(resolution==0) continue;
	  // Read Temperature

	  IntTemp.CHAR[0]=ScratchPad[0];
	  IntTemp.CHAR[1]=ScratchPad[1];


	  temperature =  0.0625 * (double) IntTemp.SHORT;

	  printf("%02d bits  Temperature: %6.2f +/- %f Celsius\n", resolution ,temperature, 0.0625 * (double)  (1<<(12 - resolution)));

	  return 1;
   }
  return 0;
}

void WriteScratchPad(unsigned char TH, unsigned char TL, unsigned char config)
{
int loop;

    // First reset device

    initialize();

    usleep(1000);
    // Skip ROM command
     writeByte(SKIP_ROM);


     // Write Scratch pad

    writeByte(WRITE_SCRATCHPAD);

    // Write TH

    writeByte(TH);

    // Write TL

    writeByte(TL);

    // Write config

    writeByte(config);
}

void  CopyScratchPad(void)
{

   // Reset device
    initialize();
    usleep(1000);

   // Skip ROM Command

    writeByte(SKIP_ROM);

   //  copy scratch pad

    writeByte(COPY_SCRATCHPAD);
    usleep(100000);
}

int main(int argc, char **argv)
{
  int loop;
  int config;
  // Set up gpi pointer for direct register access
  setup_io();
  if(ReadSensor())
    {
     printf("DS18B20 Resolution (9,10,11 or 12) ?");fflush(stdout);

    config=0;
    if(scanf("%d",&resolution)==1)
      {
        switch(resolution)
         {
           case 9:  config=0x1f;break;
           case 10: config=0x3f;break;
           case 11: config=0x5f;break;
           case 12: config=0x7f;break;
         }
      }

    if(config==0)
         printf("Invalid Value! Nothing done.\n");
    else
    {
      printf("Try to set %d bits  config=%2X\n",resolution,config);
      usleep(1000);
      WriteScratchPad(ScratchPad[2],ScratchPad[3],config);
      usleep(1000);
      CopyScratchPad();
    }
  }

  return 0;

} // main


//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;


} // setup_io
