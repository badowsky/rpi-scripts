TARGET = lcd
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	gcc -o $(TARGET) -l wiringPi $(OBJECTS)

%.o : %.c
	gcc $^ -c -o $@

$(SOURCES): Lcd.h



    

