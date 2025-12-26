.PHONY: all build check libs help

ARDUINO_CLI ?= arduino-cli
FQBN ?= esp8266:esp8266:nodemcuv2
SKETCH_DIR := max72LedNodeMCU_Scroll_Working
BUILD_DIR ?= build
LIBRARIES := MD_Parola MD_MAX72XX ArduinoJson BMP280_DEV

all: build

build: check libs
	@$(ARDUINO_CLI) compile --fqbn "$(FQBN)" --output-dir "$(BUILD_DIR)" "$(SKETCH_DIR)"

check:
	@sh ./configure

libs:
	@for lib in $(LIBRARIES); do \
		$(ARDUINO_CLI) lib install "$$lib"; \
	done

help:
	@printf "Targets:\n"
	@printf "  all   - compile the firmware via Arduino CLI\n"
	@printf "  build - compile the firmware via Arduino CLI\n"
	@printf "  check - run ./configure dependency checks\n"
	@printf "  libs  - install required Arduino libraries\n"
	@printf "\nVariables:\n"
	@printf "  ARDUINO_CLI - override Arduino CLI binary (default: arduino-cli)\n"
	@printf "  FQBN        - fully qualified board name (default: esp8266:esp8266:nodemcuv2)\n"
	@printf "  BUILD_DIR   - output directory for artifacts (default: build)\n"
