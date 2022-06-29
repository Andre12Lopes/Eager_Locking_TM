ROOT = ../..

CC = dpu-upmem-dpurte-clang
NR_TASKLETS = 12

TM = stm
SRCDIR = $(ROOT)/src
INCDIR = $(ROOT)/include
LIBDIR = $(ROOT)/lib
TMLIB = $(LIBDIR)/lib$(TM).a
TARGET = intset

TARGET_OBJS = intset.o linked_list.o skip_list.o hash_set.o dpu_alloc.o

CFLAGS += -Wall -Wno-unused-label -Wno-unused-function -std=c11

LDFLAGS += -L$(LIBDIR) -l$(TM)
CPPFLAGS += -I$(SRCDIR)

# DEFINES += -DTX_IN_MRAM
DEFINES += -DLINKED_LIST
# DEFINES += -DHASH_SET

.PHONY:	all clean

all: $(TARGET)

$(TARGET): $(TARGET_OBJS) $(TMLIB)
	$(CC) -o $@ $(TARGET_OBJS) -DNR_TASKLETS=$(NR_TASKLETS) $(LDFLAGS)

intset.o: intset.c linked_list.h intset_macros.h
linked_list.o: linked_list.c dpu_alloc.h intset_macros.h
skip_list.o: skip_list.c dpu_alloc.h intset_macros.h
hash_set.o: hash_set.c dpu_alloc.h intset_macros.h
dpu_alloc.o: dpu_alloc.c

.c.o:
	$(CC) -DNR_TASKLETS=$(NR_TASKLETS) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c $<

clean:
	rm -f $(TARGET) *.o *.s
