PRODUCT= Daemon

CFLAGS=-Wall -Werror -Wextra -O2 -lrt -lpthread

MAIN_SOURCES= $(wildcard *.cpp)

OBJECTS = $(MAIN_SOURCES:.cpp=.o)

LDFLAGS= -lusb-1.0

LIBS = /home/student/buildroot/output/target/usr/lib/libusb-1.0.so

GCC=arm-linux-g++

TARGET = Daemon

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(MAIN_SOURCES)
	@$(GCC) $(MAIN_SOURCES) $(CFLAGS) $(LIBS) -o $@

	#@scp  Daemon root@10.0.0.42:/bin

clean:
	@rm -rf $(PRODUCT)
