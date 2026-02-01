#!/bin/bash
# Create simple test images using ImageMagick (if available) or inform user

command -v convert >/dev/null 2>&1 || {
    echo "ImageMagick not found. Creating instruction file..."
    cat > README.txt << 'EOREADME'
CVN Test Assets
===============

Please add the following test images to demonstrate the engine:

bg/cafe_day.png - 1280x720 background image
ch/chocola_neutral.png - 512x1024 character sprite  
ch/vanilla_smile.png - 512x1024 character sprite

You can use any PNG images for testing.
For now, run the runtime and it will display empty screens.
EOREADME
    exit 0
}

echo "Creating test images with ImageMagick..."

mkdir -p bg ch

# Create a simple cafe background (gradient with text)
convert -size 1280x720 gradient:skyblue-wheat \
    -pointsize 72 -fill black -gravity center -annotate +0+0 "Cafe Background" \
    bg/cafe_day.png

# Create Chocola sprite (pink-ish)
convert -size 512x1024 xc:"#FFB6C1" \
    -fill white -gravity center \
    -pointsize 48 -annotate +0-400 "Chocola" \
    -pointsize 32 -annotate +0+0 "Character\nSprite" \
    ch/chocola_neutral.png

# Create Vanilla sprite (light blue)
convert -size 512x1024 xc:"#ADD8E6" \
    -fill white -gravity center \
    -pointsize 48 -annotate +0-400 "Vanilla" \
    -pointsize 32 -annotate +0+0 "Character\nSprite" \
    ch/vanilla_smile.png

echo "Test images created!"
ls -lh bg/ ch/
