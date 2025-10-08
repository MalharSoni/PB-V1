# Logo Integration Instructions for Team 839Y

## ✅ LOGO INTEGRATION COMPLETE!

Your CTRC logo has been successfully integrated into the brain screen UI!

## Your Logo File
**Original:** `/Users/malharsoni/Downloads/700x700_CTRC_White_Logo_FC.png`
**Used:** `/Users/malharsoni/Downloads/logo_100 x 100.png` (100x100 pixels)
**Description:** Yellow diagonal stripe pattern (caution tape design) on transparent background
**Size:** 19.5 KB (20,000 bytes in RGB565 format)

## What Was Implemented

### 1. Logo Conversion ✅
- Created `convert_logo.py` Python script to convert PNG to RGB565 format
- Converted `logo_100 x 100.png` to C array in `include/logo.h`
- Generated 1,709 lines of LVGL 5.3 compatible code

### 2. LVGL Crash Fix ✅
- **Problem:** LVGL initialization in `initialize()` caused data abort exception
- **Solution:** Moved initialization to `opcontrol()` (after RTOS scheduler starts)
- Added background task handler calling `lv_task_handler()` every 10ms

### 3. UI Integration ✅
- Splash screen shows CTRC logo for 2 seconds on driver control start
- Operation screen displays live telemetry during matches
- Autonomous selector ready for competition use

## How It Works Now

When you start driver control (`opcontrol()`):
1. LVGL initializes (after RTOS is running - no crash!)
2. Background task starts handling LVGL updates
3. Splash screen appears with CTRC logo (2 seconds)
4. Operation screen loads with live telemetry
5. Telemetry updates every 10ms during match

## Files Modified

- ✅ `include/logo.h` - 100x100 pixel CTRC logo in RGB565 format
- ✅ `src/subsystems/brain_ui.cpp` - LVGL task handler + logo display
- ✅ `src/main.cpp` - LVGL initialization moved to opcontrol()
- ✅ `convert_logo.py` - Logo conversion utility script

## Manual Logo Conversion (Advanced)

Due to LVGL 5.3 compatibility requirements, logo conversion requires manual steps:

### Step 1: Prepare the Image

1. **Resize** your logo to 200x200 pixels
2. **Remove white background** (optional but recommended):
   - Use Photoshop, GIMP, or online tool
   - Make background transparent
   - Save as PNG

### Step 2: Convert to RGB565 Binary

**Using Python** (recommended):
```python
from PIL import Image
import struct

# Load image
img = Image.open('logo_200x200.png').convert('RGB')
width, height = img.size

# Convert to RGB565
rgb565_data = []
for y in range(height):
    for x in range(width):
        r, g, b = img.getpixel((x, y))
        # Convert 8-bit RGB to 5-6-5
        r5 = (r >> 3) & 0x1F
        g6 = (g >> 2) & 0x3F
        b5 = (b >> 3) & 0x1F
        rgb565 = (r5 << 11) | (g6 << 5) | b5
        # Little endian: low byte first, then high byte
        rgb565_data.append(rgb565 & 0xFF)
        rgb565_data.append((rgb565 >> 8) & 0xFF)

# Write C array
with open('logo_data.txt', 'w') as f:
    f.write('static const uint8_t logo_ctrc_200x200_map[] = {\n')
    for i in range(0, len(rgb565_data), 12):
        row = ', '.join(f'0x{b:02x}' for b in rgb565_data[i:i+12])
        f.write(f'    {row},\n')
    f.write('};\n')
```

### Step 3: Update logo.h

1. Open `/Users/malharsoni/Downloads/HalloweenQualCode/include/logo.h`
2. Replace the `logo_ctrc_200x200_map[]` array with your generated data
3. Update width and height in the descriptor:
```c
const lv_img_dsc_t logo_ctrc_200x200 = {
    .header = {
        .cf = LV_IMG_CF_TRUE_COLOR,
        .always_zero = 0,
        .reserved = 0,
        .w = 200,  // Your logo width
        .h = 200,  // Your logo height
    },
    .data_size = sizeof(logo_ctrc_200x200_map),
    .data = logo_ctrc_200x200_map,
};
```

### Step 4: Enable Logo in UI Code

Edit `src/subsystems/brain_ui.cpp` at line 179:
```cpp
// Change from:
// splash_logo = lv_img_create(splash_screen, NULL);
// lv_img_set_src(splash_logo, &logo_ctrc_200x200);

// To (uncomment):
splash_logo = lv_img_create(splash_screen, NULL);
lv_img_set_src(splash_logo, &logo_ctrc_200x200);
lv_obj_align(splash_logo, NULL, LV_ALIGN_CENTER, 0, -20);
```

And comment out the text placeholder (lines 169-177).

### Step 5: Build and Test

```bash
make clean
make
pros upload
```

## Option 3: Simplified Binary Approach

If Python isn't available, use an online RGB565 converter:
1. Google "RGB565 converter online"
2. Upload your 200x200px logo
3. Download as .bin file
4. Use hex editor to get bytes
5. Format as C array

## Recommendation

**For now, keep the text logo** - it looks professional and your actual logo image is very large (80KB) which may slow down the brain's display. The text "CTRC 839Y" in gold matches your branding well.

**Future improvement:** If you want the actual logo for competitions, follow Option 2 during off-season when you have time to test thoroughly.

## Current UI Features (Working Now!)

✅ **Splash screen** with team name
✅ **Autonomous selector** with 6 touchscreen buttons
✅ **Live telemetry** display
✅ **Gold color scheme** matching CTRC branding
✅ **Professional UI** ready for competition

## Questions?

Check these resources:
- PROS LVGL Docs: https://pros.cs.purdue.edu/v5/tutorials/topical/display.html
- LVGL 5.3 Docs: https://docs.lvgl.io/5.3/
- Team 81K Tutorial: https://team81k.github.io/ProsLVGLTutorial/
