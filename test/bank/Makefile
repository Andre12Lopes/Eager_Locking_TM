 ROOT = ../..

CC = dpu-upmem-dpurte-clang
NR_TASKLETS = 2

TM = stm
SRCDIR = $(ROOT)/src
INCDIR = $(ROOT)/include
LIBDIR = $(ROOT)/lib
TMLIB = $(LIBDIR)/lib$(TM).a
TARGET = bank

CFLAGS += -Wall -Wno-unused-label -Wno-unused-function -std=c11

LDFLAGS += -L$(LIBDIR) -l$(TM)
CPPFLAGS += -I$(SRCDIR)

# DEFINES += -DTX_IN_MRAM
DEFINES += -DDATA_IN_MRAM

# DEFINES += -DRO_TX

.PHONY:	all clean

all: $(TARGET)

%.o: %.c
	$(CC) -DNR_TASKLETS=$(NR_TASKLETS) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c -o $@ $<

bank.o: macros.h

$(TARGET): $(TARGET).o $(TMLIB)
	$(CC) -o $@ $< -DNR_TASKLETS=$(NR_TASKLETS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o *.s
