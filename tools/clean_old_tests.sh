#!/bin/bash
# Clean up old test files from SD card to avoid confusion

echo "Cleaning old autonomous test files from SD card..."

# Archive old files to a backup directory
BACKUP_DIR="/Volumes/V5-DATA/archive"
mkdir -p "$BACKUP_DIR"

# Move all old auton files to archive
echo "Moving old auton_*.csv files to archive..."
mv /Volumes/V5-DATA/auton_*.csv "$BACKUP_DIR/" 2>/dev/null

# Move all old run files to archive
echo "Moving old run_*.csv files to archive..."
mv /Volumes/V5-DATA/run_*.csv "$BACKUP_DIR/" 2>/dev/null

# Move 0-byte files to trash
echo "Removing 0-byte files..."
find /Volumes/V5-DATA/ -name "*.csv" -size 0 -delete

echo "✓ SD card cleaned!"
echo "Old files moved to: $BACKUP_DIR"
echo ""
echo "Ready for new test runs."
