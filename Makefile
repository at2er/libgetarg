CC = gcc
CFLAGS = -Wall -std=c99
CDEBUG_FLAGS = -Wall -std=c99 -g
AR = ar

SRC = getarg.c
HEADER = getarg.h
OBJ = $(SRC:.c=.o)
TARGET = libgetarg.a
TARGET_DIR = /usr/local/lib
HEADER_DIR = /usr/local/include

DEBUG_TARGET = getarg.debug
DEBUG_OBJ = $(SRC:.c=.debug.o)

.PHONY: all clean debug install uninstall
all: $(TARGET) $(HEADER)
debug: main.c $(DEBUG_TARGET) $(HEADER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(CDEBUG_FLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(AR) -rcs $@ $(OBJ)

$(DEBUG_TARGET): main.c $(DEBUG_OBJ)
	$(CC) $(CDEBUG_FLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)\
		$(DEBUG_OBJ) $(DEBUG_TARGET)

install: all
	mkdir -p $(TARGET_DIR)
	cp -f $(TARGET) $(TARGET_DIR)/$(TARGET)
	cp -f $(HEADER) $(HEADER_DIR)/$(HEADER)

uninstall:
	rm -f $(TARGET_DIR)/$(TARGET) $(HEADER_DIR)/$(HEADER)
