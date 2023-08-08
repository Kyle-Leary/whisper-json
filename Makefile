CC := gcc
CFLAGS += -Isrc -Iapi -Wall -g 

BACKEND_DIR := linux

# Directories
SRC_DIR := src 
OBJ_DIR := build
TEST_SRC_PATH := testmain.c 

TEST_TARGET := wjsontest
TARGET := libwjson.a

# just compile everything in SRC_DIR.
SRCS := $(shell find $(SRC_DIR) -type f -name "*.c") 
OBJS = $(SRCS:.c=.o)

all: clean $(TARGET)
	@echo  Successfully created the library at $(TARGET).

test: clean $(TEST_TARGET)
	@echo  Successfully created the test at $(TEST_TARGET). Running tests...
	./$(TEST_TARGET)

# then archive them into a .a file.
$(TARGET): $(OBJS) 
	ar rcs $(TARGET) $(OBJS)

# putting test.c outside of the SRC_DIR, so it won't be picked up in the library build.
# link the test.c file against the library, and just run the executable generated to test the library.
# link test.c BEFORE the .a static lib?? why does this matter??
$(TEST_TARGET): $(TARGET)
	$(CC) -static $(TEST_SRC_PATH) $< -o $@ $(CFLAGS) 

# compile all the objects
.c.o:
	$(CC) $(CFLAGS) -c $<  -o $@


clean:
	rm -f $(shell find . -name "*.o") $(TARGET) $(TEST_TARGET)
