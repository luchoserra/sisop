# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# All .c files in the current directory
SRCS = $(wildcard *.c)

# Output executable
TARGET = schedsimu

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule to build the program
all: $(TARGET)

# Rule to link object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
