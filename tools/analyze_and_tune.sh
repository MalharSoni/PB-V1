#!/bin/bash
# Automated PID Tuning Workflow
# Analyzes latest telemetry file and provides tuning recommendations

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "========================================================================"
echo "VEX V5 PID TUNING WORKFLOW"
echo "========================================================================"

# Find latest auton file
LATEST_FILE=$(ls -t /Volumes/V5-DATA/auton_*.csv 2>/dev/null | head -1)

if [ -z "$LATEST_FILE" ]; then
    echo -e "${RED}Error: No auton log files found on SD card${NC}"
    echo "Make sure:"
    echo "  1. SD card is inserted and mounted"
    echo "  2. You ran autonomous with auto-logging enabled"
    exit 1
fi

echo -e "${GREEN}Found latest log:${NC} $LATEST_FILE"

# Get file size
FILE_SIZE=$(ls -lh "$LATEST_FILE" | awk '{print $5}')
echo "File size: $FILE_SIZE"

if [ "$FILE_SIZE" = "0B" ]; then
    echo -e "${RED}Error: Log file is empty (0 bytes)${NC}"
    echo "Possible causes:"
    echo "  1. Didn't close manual logging before running autonomous (press A to toggle off)"
    echo "  2. Robot didn't move during autonomous"
    echo "  3. SD card write error"
    exit 1
fi

echo ""
echo "Analyzing telemetry data..."
echo ""

# Run analysis with appropriate targets
# 48" forward movement + 90° turn
python3 tools/plot_telemetry.py "$LATEST_FILE" 48

echo ""
echo "========================================================================"
echo "NEXT STEPS:"
echo "========================================================================"
echo "1. Open the plot: open /Volumes/V5-DATA/telemetry_analysis.png"
echo "2. Review the PID recommendations above"
echo "3. Edit src/globals.cpp to adjust PID values"
echo "4. Rebuild: make"
echo "5. Upload: pros upload"
echo "6. Test again and re-run this script"
echo ""
echo "Current PID values are in: src/globals.cpp (lines ~160-200)"
echo "========================================================================"
