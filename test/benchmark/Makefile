CC = g++

TARGET = launch_bank launch_intset

CFLAGS += -Wall
CFLAGS += -Wno-unused-label -Wno-unused-function --std=c++11

LDFLAGS += -L$(LIBDIR) -l$(TM)
CPPFLAGS += -I$(SRCDIR)

.PHONY:	all clean

all: $(TARGET)

$(TARGET): %: %.cpp
	$(CC) $(CFLAGS) $< -o $@ `dpu-pkg-config --cflags --libs dpu` -g

clean:
	rm -f $(TARGET) *.o