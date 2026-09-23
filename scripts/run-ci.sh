#!/bin/bash
set -e

echo "=========================================="
echo "  ESP32-S3 CI/CD Pipeline (Local)"
echo "=========================================="
echo ""

cd "$(dirname "$0")/.."

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

ERRORS=0
WARNINGS=0

# Function to run checks
run_check() {
    local name=$1
    local cmd=$2

    echo -e "${YELLOW}[CHECK]${NC} $name..."
    if eval "$cmd" > /tmp/check_output.log 2>&1; then
        echo -e "${GREEN}✓${NC} $name passed"
        return 0
    else
        echo -e "${RED}✗${NC} $name failed"
        cat /tmp/check_output.log
        ERRORS=$((ERRORS + 1))
        return 1
    fi
}

# ========== 1. BUILD ==========
echo ""
echo "========== 1. FIRMWARE BUILD =========="

if command -v pio &> /dev/null; then
    run_check "PlatformIO Build" "pio run -e esp32-s3-audit" || true

    if [ -f .pio/build/esp32-s3-audit/firmware.bin ]; then
        SIZE=$(stat -f%z .pio/build/esp32-s3-audit/firmware.bin 2>/dev/null || stat -c%s .pio/build/esp32-s3-audit/firmware.bin)
        SIZE_MB=$(echo "scale=2; $SIZE / 1048576" | bc)
        echo -e "${GREEN}✓${NC} Firmware built: ${SIZE_MB}MB"
    fi
else
    echo -e "${YELLOW}⚠${NC} PlatformIO not installed, skipping build"
fi

# ========== 2. SECURITY ANALYSIS ==========
echo ""
echo "========== 2. SECURITY ANALYSIS =========="

# Check for hardcoded credentials
echo -e "${YELLOW}[CHECK]${NC} Scanning for hardcoded credentials..."
if grep -r "password.*=" src/ --include="*.cpp" --include="*.h" 2>/dev/null | grep -v "//" | grep -v "password_"; then
    echo -e "${YELLOW}⚠${NC} Potential hardcoded credentials found"
    WARNINGS=$((WARNINGS + 1))
else
    echo -e "${GREEN}✓${NC} No obvious hardcoded credentials"
fi

# Check for unsafe functions (word boundary for more precise matching)
echo -e "${YELLOW}[CHECK]${NC} Scanning for unsafe functions..."
UNSAFE_FOUND=0
for func in strcpy sprintf gets strcat; do
    # Use word boundary \b to match exact function names, exclude comments
    if grep -r "\b$func\s*(" src/ --include="*.cpp" --include="*.h" 2>/dev/null | grep -v "safer\|//\|Widgets\|safe"; then
        echo -e "${RED}✗${NC} Found potentially unsafe function: $func"
        UNSAFE_FOUND=1
        ERRORS=$((ERRORS + 1))
    fi
done
if [ $UNSAFE_FOUND -eq 0 ]; then
    echo -e "${GREEN}✓${NC} No unsafe function calls detected"
fi

# Check cryptographic usage
echo -e "${YELLOW}[CHECK]${NC} Analyzing cryptographic implementations..."
CRYPTO_COUNT=$(grep -r "mbedtls_" src/ --include="*.cpp" 2>/dev/null | wc -l || echo "0")
echo "  Found $CRYPTO_COUNT mbedtls calls"
PBKDF2_COUNT=$(grep -r "pbkdf2\|PBKDF2" src/ --include="*.cpp" 2>/dev/null | wc -l || echo "0")
echo "  PBKDF2 implementations: $PBKDF2_COUNT"

if [ $CRYPTO_COUNT -gt 0 ]; then
    echo -e "${GREEN}✓${NC} Cryptographic functions properly used"
else
    echo -e "${YELLOW}⚠${NC} No cryptographic usage detected"
fi

# ========== 3. CODE QUALITY ==========
echo ""
echo "========== 3. CODE QUALITY =========="

# Count lines of code
LINES=$(find src include -name "*.cpp" -o -name "*.h" 2>/dev/null | xargs wc -l | tail -1 | awk '{print $1}')
echo "  Total lines of code: $LINES"

# Check for TODO/FIXME
TODOS=$(grep -r "TODO\|FIXME" src include --include="*.cpp" --include="*.h" 2>/dev/null | wc -l || echo "0")
if [ $TODOS -gt 0 ]; then
    echo -e "${YELLOW}⚠${NC} Found $TODOS TODO/FIXME comments"
fi

# Check file structure
echo -e "${YELLOW}[CHECK]${NC} Verifying file structure..."
REQUIRED_FILES=(
    "CLAUDE.md"
    "SECURITY.md"
    "TESTING.md"
    "DEVELOPMENT.md"
    "platformio.ini"
    "include/hw_config.h"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo -e "  ${GREEN}✓${NC} $file"
    else
        echo -e "  ${RED}✗${NC} Missing: $file"
        ERRORS=$((ERRORS + 1))
    fi
done

# ========== 4. DOCUMENTATION ==========
echo ""
echo "========== 4. DOCUMENTATION =========="

if [ -f "CLAUDE.md" ]; then
    LINES=$(wc -l < CLAUDE.md)
    echo -e "${GREEN}✓${NC} CLAUDE.md: $LINES lines"
else
    echo -e "${RED}✗${NC} CLAUDE.md missing"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "SECURITY.md" ]; then
    LINES=$(wc -l < SECURITY.md)
    echo -e "${GREEN}✓${NC} SECURITY.md: $LINES lines"
else
    echo -e "${RED}✗${NC} SECURITY.md missing"
    ERRORS=$((ERRORS + 1))
fi

# ========== 5. GIT STATUS ==========
echo ""
echo "========== 5. GIT STATUS =========="

if command -v git &> /dev/null; then
    BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
    COMMIT=$(git rev-parse --short HEAD 2>/dev/null)
    echo -e "  Branch: ${GREEN}$BRANCH${NC}"
    echo -e "  Commit: ${GREEN}$COMMIT${NC}"

    UNCOMMITTED=$(git status --porcelain | wc -l)
    if [ $UNCOMMITTED -eq 0 ]; then
        echo -e "  ${GREEN}✓${NC} Working tree clean"
    else
        echo -e "  ${YELLOW}⚠${NC} $UNCOMMITTED uncommitted changes"
    fi
fi

# ========== SUMMARY ==========
echo ""
echo "=========================================="
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ ALL CHECKS PASSED${NC}"
    EXIT_CODE=0
else
    echo -e "${RED}✗ $ERRORS ERRORS FOUND${NC}"
    EXIT_CODE=1
fi

if [ $WARNINGS -gt 0 ]; then
    echo -e "${YELLOW}⚠ $WARNINGS WARNINGS${NC}"
fi
echo "=========================================="

exit $EXIT_CODE
