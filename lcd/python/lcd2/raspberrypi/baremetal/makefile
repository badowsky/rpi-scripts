CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
IMG = kernel.img
ELF = baremetal.elf
#source directories
DIRS = gpio lcd main memory time lcd
BUILD = build
#assembly files
ASS = $(foreach dir, $(DIRS),$(wildcard $(dir)/*.s))
SOURCES = $(foreach dir, $(DIRS),$(wildcard $(dir)/*.c)) 
DEPS = $(foreach dir, $(DIRS),$(wildcard $(dir)/*.h))
LDSCRIPT = kernel.ld
#objects generated from the .c files
OBJECTS = $(addsuffix .o,$(basename $(SOURCES)))
#objects generated from the .s files
ASSOBJECTS = $(addsuffix .o,$(basename $(ASS)))

SDCARD =  /Volumes/Untitled/
	
all : $(IMG)

$(IMG) : $(ELF)
	$(OBJCOPY) $(BUILD)/$(ELF) -O binary $(BUILD)/$(IMG)

$(ELF) : $(OBJECTS) $(ASSOBJECTS)
	$(LD) -T $(BUILD)/$(LDSCRIPT) -o $(BUILD)/$(ELF) $(OBJECTS) $(ASSOBJECTS)
	rm  $(OBJECTS) $(ASSOBJECTS) 

$(OBJECTS) : $(SOURCES) $(DEPS)
	$(CC) -c -O2 $(patsubst %.o,%.c,$@) -I./ -o $@

$(ASSOBJECTS) : $(ASS)
	$(AS) $(patsubst %.o,%.s,$@) -o $@

install :
	cp -f $(BUILD)/$(IMG) $(SDCARD)
	-rm -Rf $(SDCARD)/.*
	diskutil eject $(SDCARD)

clean : 
	rm $(BUILD)/$(IMG)
	rm $(BUILD)/$(ELF)

