CC = gcc
CFLAGS = -Wall -std=c99
CDEBUG_FLAGS = -Wall -std=c99 -g
AR = ar
PREFIX = /usr/local

HEADER_DIR = $(PREFIX)/include
TARGET_DIR = $(PREFIX)/lib

HEADER = getarg.h
TARGET = libgetarg.a

SRC = getarg.c
OBJ = $(SRC:.c=.o)

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
	mkdir -p $(HEADER_DIR) $(TARGET_DIR)
	cp -f $(HEADER) $(HEADER_DIR)/$(HEADER)
	cp -f $(TARGET) $(TARGET_DIR)/$(TARGET)

uninstall:
	rm -f $(HEADER_DIR)/$(HEADER) $(TARGET_DIR)/$(TARGET)
