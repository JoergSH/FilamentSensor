#!/bin/bash
# Script to update firmware binaries in the firmware/ folder

echo "=== Centauri Carbon Monitor - Firmware Update Script ==="
echo ""

# Build directory
BUILD_DIR=".pio/build/nologo_esp32c3_super_mini"
FIRMWARE_DIR="firmware"

# Check if build exists
if [ ! -f "$BUILD_DIR/firmware.bin" ]; then
    echo "❌ Error: Build not found. Run 'platformio run' first."
    exit 1
fi

# Create firmware directory if it doesn't exist
mkdir -p "$FIRMWARE_DIR"

echo "📦 Copying firmware files..."

# Copy binaries
cp "$BUILD_DIR/bootloader.bin" "$FIRMWARE_DIR/"
cp "$BUILD_DIR/partitions.bin" "$FIRMWARE_DIR/"
cp "$BUILD_DIR/firmware.bin" "$FIRMWARE_DIR/"

echo "✅ Firmware files updated:"
echo ""
echo "   bootloader.bin  -> $FIRMWARE_DIR/"
echo "   partitions.bin  -> $FIRMWARE_DIR/"
echo "   firmware.bin    -> $FIRMWARE_DIR/"
echo ""

# Show file sizes
echo "📊 File sizes:"
ls -lh "$FIRMWARE_DIR"/*.bin | awk '{print "   " $9 ": " $5}'
echo ""

# Calculate total size
TOTAL_SIZE=$(du -sh "$FIRMWARE_DIR" | awk '{print $1}')
echo "   Total: $TOTAL_SIZE"
echo ""

# Update version info
echo "📝 Updating VERSION.txt..."
BUILD_DATE=$(date +"%Y-%m-%d")
BUILD_TIME=$(date +"%H:%M UTC")

# Get file sizes
BOOTLOADER_SIZE=$(stat -f%z "$FIRMWARE_DIR/bootloader.bin" 2>/dev/null || stat -c%s "$FIRMWARE_DIR/bootloader.bin" 2>/dev/null)
PARTITIONS_SIZE=$(stat -f%z "$FIRMWARE_DIR/partitions.bin" 2>/dev/null || stat -c%s "$FIRMWARE_DIR/partitions.bin" 2>/dev/null)
FIRMWARE_SIZE=$(stat -f%z "$FIRMWARE_DIR/firmware.bin" 2>/dev/null || stat -c%s "$FIRMWARE_DIR/firmware.bin" 2>/dev/null)
TOTAL_BYTES=$((BOOTLOADER_SIZE + PARTITIONS_SIZE + FIRMWARE_SIZE))

# Format numbers with thousand separators
BOOTLOADER_SIZE_FMT=$(printf "%'d" $BOOTLOADER_SIZE)
PARTITIONS_SIZE_FMT=$(printf "%'d" $PARTITIONS_SIZE)
FIRMWARE_SIZE_FMT=$(printf "%'d" $FIRMWARE_SIZE)
TOTAL_BYTES_FMT=$(printf "%'d" $TOTAL_BYTES)

sed -i "s/Build Date: .*/Build Date: $BUILD_DATE/" "$FIRMWARE_DIR/VERSION.txt"
sed -i "s/Build Time: .*/Build Time: $BUILD_TIME/" "$FIRMWARE_DIR/VERSION.txt"
sed -i "s/bootloader.bin: .* bytes/bootloader.bin: $BOOTLOADER_SIZE_FMT bytes/" "$FIRMWARE_DIR/VERSION.txt"
sed -i "s/partitions.bin: .* bytes/partitions.bin: $PARTITIONS_SIZE_FMT bytes/" "$FIRMWARE_DIR/VERSION.txt"
sed -i "s/firmware.bin:   .* bytes/firmware.bin:   $FIRMWARE_SIZE_FMT bytes/" "$FIRMWARE_DIR/VERSION.txt"
sed -i "s/Total:          .* bytes/Total:          $TOTAL_BYTES_FMT bytes/" "$FIRMWARE_DIR/VERSION.txt"

echo "✅ Done! Firmware files are ready for git commit."
echo ""
echo "Next steps:"
echo "  1. git add firmware/"
echo "  2. git commit -m \"Update firmware binaries\""
echo "  3. git push"
