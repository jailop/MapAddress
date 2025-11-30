#!/bin/bash
set -e

APP_NAME="MapAddress"
VERSION="${VERSION:-1.0.0}"
DMG_NAME="${APP_NAME}-${VERSION}.dmg"

echo "=== Creating DMG for ${APP_NAME} v${VERSION} ==="

# Check if app bundle exists
if [ ! -d "build/${APP_NAME}.app" ]; then
    echo "ERROR: build/${APP_NAME}.app not found!"
    exit 1
fi

echo "✓ Found ${APP_NAME}.app"

# Create DMG using create-dmg
create-dmg \
  --volname "${APP_NAME}" \
  --volicon "mapaddress.png" \
  --window-pos 200 120 \
  --window-size 800 400 \
  --icon-size 100 \
  --icon "${APP_NAME}.app" 200 190 \
  --hide-extension "${APP_NAME}.app" \
  --app-drop-link 600 185 \
  "${DMG_NAME}" \
  "build/${APP_NAME}.app" || {
    echo "WARNING: create-dmg with options failed, trying simple approach..."
    # Fallback to simple DMG creation
    hdiutil create -volname "${APP_NAME}" -srcfolder "build/${APP_NAME}.app" -ov -format UDZO "${DMG_NAME}"
  }

if [ -f "${DMG_NAME}" ]; then
    echo "✓ DMG created successfully: ${DMG_NAME}"
    ls -lh "${DMG_NAME}"
else
    echo "ERROR: Failed to create DMG"
    exit 1
fi
