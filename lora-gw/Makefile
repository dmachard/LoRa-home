# Configuration
FQBN = esp32:esp32:esp32c6:PartitionScheme=min_spiffs
PORT = /dev/ttyACM0
ARDUINO_CLI = arduino-cli
PYTHON = python3
BUILD_DIR = ./build

.PHONY: all assets compile upload monitor clean help

all: assets compile

assets:
	@echo "=== Generating HTML/JS/CSS Headers ==="
	$(PYTHON) generate_assets.py

compile: assets
	@echo "=== Compiling ESP32-C6 Firmware ==="
	$(ARDUINO_CLI) compile --fqbn $(FQBN) --output-dir $(BUILD_DIR) lora-gw.ino

upload: compile
	@echo "=== Flashing Firmware to $(PORT) ==="
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(FQBN) --input-dir $(BUILD_DIR)

monitor:
	@echo "=== Starting Serial Monitor on $(PORT) ==="
	$(ARDUINO_CLI) monitor -p $(PORT) -c baudrate=115200

clean:
	@echo "=== Cleaning Generated Files ==="
	rm -rf $(BUILD_DIR)
	rm -f index_html.h admin_html.h update_html.h

help:
	@echo "Available commands:"
	@echo "  make          : Generate HTML assets and compile the project"
	@echo "  make assets   : Rebuild index_html.h and update_html.h from the html/ directory"
	@echo "  make compile  : Compile the C++ code"
	@echo "  make upload   : Generate assets and flash the board (Default port: $(PORT))"
	@echo "  make monitor  : Start serial monitor on $(PORT)"
	@echo "  make clean    : Delete build directory and generated header files"
