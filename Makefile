CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = kubsh
SRC = kubsh.c vfs.c
LIBS = -lreadline

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

.PHONY: all run clean install uninstall
