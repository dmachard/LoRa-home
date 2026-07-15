# Configuration
FQBN = esp32:esp32:esp32c3:CDCOnBoot=cdc
PORT = /dev/ttyACM0
ARDUINO_CLI = arduino-cli
BUILD_DIR = ./build

.PHONY: all compile upload monitor clean help

all: compile

compile:
	@echo "=== Compiling ESP32-C3 Node Firmware ==="
	$(ARDUINO_CLI) compile --fqbn $(FQBN) --output-dir $(BUILD_DIR) lora-node.ino

upload:
	@echo "=== Flashing Firmware to $(PORT) ==="
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(FQBN) --input-dir $(BUILD_DIR)

monitor:
	@echo "=== Starting Serial Monitor on $(PORT) ==="
	$(ARDUINO_CLI) monitor -p $(PORT) -c baudrate=115200

clean:
	@echo "=== Cleaning Generated Files ==="
	rm -rf $(BUILD_DIR)

help:
	@echo "Available commands:"
	@echo "  make          : Compile the project"
	@echo "  make compile  : Compile the C++ code"
	@echo "  make upload   : Flash the board (Default port: $(PORT))"
	@echo "  make monitor  : Start serial monitor on $(PORT)"
	@echo "  make clean    : Delete build directory"
