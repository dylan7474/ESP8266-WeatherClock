.PHONY: all check help

all:
	@echo "No native build target. Use the Arduino IDE/CLI to compile the sketch."

check:
	@./configure

help:
	@printf "Targets:\n"
	@printf "  all   - print a build hint\n"
	@printf "  check - run ./configure dependency checks\n"
