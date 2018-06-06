PRODUCT= web

CFLAGS=-Wall -Werror -Wextra -O2 -lrt -lpthread

MAIN_SOURCES= $(wildcard *.cpp)

OBJECTS = $(MAIN_SOURCES:.cpp=.o)

LDFLAGS= -lusb-1.0

LIBS = /home/student/buildroot/output/target/usr/lib/libusb-1.0.so

GCC=arm-linux-g++

TARGET = web

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(MAIN_SOURCES)
	@$(GCC) $(MAIN_SOURCES) $(CFLAGS) $(LIBS) -o $@.cgi

	@scp  $(PRODUCT).cgi root@10.0.0.42:/var/www/cgi-bin

clean:
	@rm -rf $(PRODUCT)
