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

# Docker
docker-build:
	docker build -t kubsh:latest .

docker-run:
	docker run -it --rm \
		-v $(PWD)/data:/root/users \
		-v /etc/passwd:/etc/passwd:ro \
		-v /etc/group:/etc/group:ro \
		--privileged \
		kubsh:latest

# Debian package
deb: clean
	mkdir -p debian/DEBIAN
	mkdir -p debian/usr/local/bin
	cp $(TARGET) debian/usr/local/bin/
	
	echo "Package: kubsh" > debian/DEBIAN/control
	echo "Version: 1.0" >> debian/DEBIAN/control
	echo "Section: utils" >> debian/DEBIAN/control
	echo "Priority: optional" >> debian/DEBIAN/control
	echo "Architecture: amd64" >> debian/DEBIAN/control
	echo "Maintainer: Student <student@example.com>" >> debian/DEBIAN/control
	echo "Description: Custom shell with VFS users management" >> debian/DEBIAN/control
	echo " A simple shell with users VFS management capabilities." >> debian/DEBIAN/control
	
	dpkg-deb --build debian kubsh_1.0_amd64.deb
	rm -rf debian

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

.PHONY: all run clean deb install uninstall docker-build docker-run
