EXEC = pc-test
.PHONY: all
all: $(EXEC)

CC ?= gcc
CFLAGS = -std=c11 -Wall -g
LDFLAGS = -lpthread

OBJS := \
    pc-test.o \
    queue.o

deps := $(OBJS:%.o=.%.o.d)

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -MF .$@.d -o $@ $<

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

check: $(EXEC)
	@./$(EXEC)

clean:
	$(RM) $(EXEC) $(OBJS) $(deps)

-include $(deps)
