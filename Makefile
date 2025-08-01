# SPDX-License-Identifier: GPL-2.0
VERSION = 0
PATCHLEVEL = 1
SUBLEVEL = 0
NAME = 'luka - formerly dc2'

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic

TARGET = luka
SRC = luka.c

all: clean $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
