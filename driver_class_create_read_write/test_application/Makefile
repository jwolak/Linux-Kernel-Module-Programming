CC = aarch64-linux-gnu-gcc
CFLAGS = -Wall -Wextra -O2
TARGET = test_simple_module.arm64
SOURCES = test_application.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean
