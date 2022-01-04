CFLAGS := -Wall
LDFLAGS := -pthread

OBJDIR := obj
TARGET := ftp-server

SRCS := $(wildcard *.c)
OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)


$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(TARGET) $(OBJS)
