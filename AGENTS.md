# Agent guidance

## Project overview
- The firmware lives in `max72LedNodeMCU_Scroll_Working_v10.9.10.ino`.
- Build artifacts are generated via the Arduino toolchain; the Makefiles only provide convenience targets.

## Contribution notes
- Keep changes focused and avoid reformatting the sketch unless necessary.
- Document any user-facing behavior changes in `README.md`.
- Avoid introducing secrets; use placeholders and document where real values belong.

## Automation tips
- Run `./configure` to confirm the Arduino CLI and ESP8266 core are available.
- The Makefile targets are intended for automation checks, not full firmware flashing.

## Validation
- Prefer running `make` and `make check` (or `make -f Makefile.win check` on Windows shells).
- Note any missing dependencies in the final summary.
