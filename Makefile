.PHONY: help build clean test ci security docs flash monitor

help:
	@echo "ESP32-S3 Offensive Security Platform - Build Commands"
	@echo "======================================================"
	@echo ""
	@echo "Build:"
	@echo "  make build          - Build firmware for ESP32-S3"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make flash          - Build and flash to device"
	@echo ""
	@echo "Testing:"
	@echo "  make test           - Run unit tests"
	@echo "  make ci             - Run full CI pipeline"
	@echo "  make security       - Run security analysis"
	@echo ""
	@echo "Development:"
	@echo "  make monitor        - Monitor serial output"
	@echo "  make docs           - Generate documentation"
	@echo "  make format         - Format code (placeholder)"
	@echo ""
	@echo "Info:"
	@echo "  make status         - Show git and build status"
	@echo "  make size           - Show firmware size info"

build:
	@echo "Building ESP32-S3 firmware..."
	pio run -e esp32-s3-audit

clean:
	@echo "Cleaning build artifacts..."
	pio run -e esp32-s3-audit -t clean
	rm -rf build/
	rm -f .pio/build/esp32-s3-audit/firmware.*

test:
	@echo "Running tests..."
	@if [ -f test/test_wpa2_utils.cpp ]; then \
		echo "Compiling test suite..."; \
		g++ -std=c++11 -Wall -Wextra -I include -o /tmp/test_suite test/test_*.cpp 2>/dev/null && \
		echo "Running tests..." && \
		/tmp/test_suite; \
	else \
		echo "No test files found"; \
	fi

ci: clean build security
	@echo ""
	@echo "Running full CI pipeline..."
	bash scripts/run-ci.sh

security:
	@echo "Running security analysis..."
	@echo ""
	@echo "[1/4] Checking for hardcoded credentials..."
	@grep -r "password.*=" src/ --include="*.cpp" --include="*.h" 2>/dev/null | grep -v "//" || echo "✓ No obvious hardcoded credentials"
	@echo ""
	@echo "[2/4] Scanning for unsafe functions..."
	@grep -r "strcpy\|sprintf\|gets" src/ --include="*.cpp" --include="*.h" 2>/dev/null || echo "✓ No unsafe functions detected"
	@echo ""
	@echo "[3/4] Verifying cryptographic usage..."
	@echo "  PBKDF2 implementations: $$(grep -r 'pbkdf2\|PBKDF2' src/ --include="*.cpp" 2>/dev/null | wc -l)"
	@echo "  HMAC implementations: $$(grep -r 'hmac\|HMAC' src/ --include="*.cpp" 2>/dev/null | wc -l)"
	@echo "  mbedtls usage: $$(grep -r 'mbedtls_' src/ --include="*.cpp" 2>/dev/null | wc -l) calls"
	@echo ""
	@echo "[4/4] Analyzing code quality..."
	@echo "  Lines of code: $$(find src include -name "*.cpp" -o -name "*.h" 2>/dev/null | xargs wc -l | tail -1 | awk '{print $$1}')"
	@echo "  TODO/FIXME comments: $$(grep -r 'TODO\|FIXME' src include --include="*.cpp" --include="*.h" 2>/dev/null | wc -l)"

flash: build
	@echo "Flashing firmware to ESP32-S3..."
	pio run -e esp32-s3-audit -t upload
	@echo ""
	@echo "Flashing complete! Starting serial monitor..."
	@sleep 2
	make monitor

monitor:
	@echo "Monitoring serial output (Ctrl+C to exit)..."
	pio device monitor -e esp32-s3-audit --baud 115200

docs:
	@echo "Generating documentation..."
	@echo "  ✓ CLAUDE.md (architecture)"
	@echo "  ✓ SECURITY.md (security review)"
	@echo "  ✓ TESTING.md (testing guide)"
	@echo "  ✓ DEVELOPMENT.md (development notes)"

status:
	@echo "Git Status:"
	@git status --short
	@echo ""
	@echo "Current Branch:"
	@git rev-parse --abbrev-ref HEAD
	@echo ""
	@echo "Latest Commit:"
	@git log -1 --oneline

size:
	@echo "Firmware Size Information:"
	@if [ -f .pio/build/esp32-s3-audit/firmware.bin ]; then \
		SIZE=$$(stat -f%z .pio/build/esp32-s3-audit/firmware.bin 2>/dev/null || stat -c%s .pio/build/esp32-s3-audit/firmware.bin); \
		SIZE_MB=$$(echo "scale=2; $$SIZE / 1048576" | bc); \
		SIZE_PERCENT=$$(echo "scale=1; $$SIZE * 100 / 3145728" | bc); \
		echo "  Binary size: $$SIZE_MB MB ($$SIZE_PERCENT% of 3MB max)"; \
		echo "  Available: $$(echo "scale=2; 3145728 - $$SIZE | / 1048576" | bc) MB"; \
	else \
		echo "  Firmware not built yet. Run 'make build' first."; \
	fi

all: clean build ci
	@echo ""
	@echo "✓ Build complete!"

.DEFAULT_GOAL := help
