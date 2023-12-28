# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Source files
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Executable
TARGET = tarsau

# Build all
all: $(TARGET)

# Build executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

# Help target
help:
	@echo "Valid targets: all, clean"

.PHONY: all clean help

