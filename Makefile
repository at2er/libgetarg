CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra
AR = ar
PREFIX = /usr/local

HEADER_DIR = $(PREFIX)/include
TARGET_DIR = $(PREFIX)/lib

HEADER = getarg.h
TARGET = libgetarg.a

SRC = getarg.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean debug install uninstall
all: $(TARGET) $(HEADER)
debug: all main.c
	$(CC) -o getarg main.c $(CFLAGS) -L. -lgetarg

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(TARGET): $(OBJ)
	$(AR) -rcs $@ $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET) getarg

install: all
	mkdir -p $(HEADER_DIR) $(TARGET_DIR)
	cp -f $(HEADER) $(HEADER_DIR)/$(HEADER)
	cp -f $(TARGET) $(TARGET_DIR)/$(TARGET)

uninstall:
	rm -f $(HEADER_DIR)/$(HEADER) $(TARGET_DIR)/$(TARGET)
