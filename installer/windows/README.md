# Windows Installer Guide

This directory contains scripts to create a Windows installer for MapAddress.

## Prerequisites

### 1. Build the Application on Windows

First, build the application in **Release** mode:

```bash
# On Windows with Qt installed
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### 2. Install NSIS (Nullsoft Scriptable Install System)

Download and install NSIS from:
- **Official Site**: https://nsis.sourceforge.io/Download
- **Direct Link**: https://nsis.sourceforge.io/Download (get the latest version)

Add NSIS to your PATH or note the installation directory (typically `C:\Program Files (x86)\NSIS`).

### 3. Convert SVG Icon to ICO

You need to convert the app icon to ICO format. Use one of these methods:

**Option A: Using ImageMagick on Windows**
```bash
magick convert app-icon-256.png -define icon:auto-resize=16,32,48,64,128,256 app-icon.ico
```

**Option B: Using Online Converter**
- Upload `resources/icons/app-icon.svg` or `app-icon-256.png`
- Download as `.ico` format with multiple sizes
- Save to `resources/icons/app-icon.ico`

**Option C: Using GIMP**
1. Open PNG in GIMP
2. File → Export As → Choose `.ico`
3. Select multiple sizes in the dialog

## Building the Installer

### Method 1: Automated Deployment + NSIS (Recommended)

1. **Deploy the application**:
   ```cmd
   cd installer\windows
   deploy.bat
   ```
   
   This will:
   - Copy the executable
   - Run `windeployqt` to gather Qt dependencies
   - Create a portable ZIP file
   - Prepare files for the installer

2. **Edit `installer.nsi`** if needed:
   - Update Qt paths (if different from default)
   - Adjust file lists based on your Qt version

3. **Build the installer**:
   ```cmd
   makensis installer.nsi
   ```
   
   This creates: `MapAddress-1.0.0-Setup.exe`

### Method 2: Manual Steps

1. **Build in Release mode**:
   ```cmd
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   ```

2. **Create deployment directory**:
   ```cmd
   mkdir installer\windows\deploy
   copy build\Release\MapAddress.exe installer\windows\deploy\
   ```

3. **Deploy Qt dependencies**:
   ```cmd
   cd installer\windows\deploy
   C:\Qt\6.7.0\mingw_64\bin\windeployqt.exe --release MapAddress.exe
   ```

4. **Build installer**:
   ```cmd
   cd ..
   makensis installer.nsi
   ```

## What Gets Installed

The installer will:

✅ Install the application to `Program Files`  
✅ Copy all Qt dependencies (DLLs, plugins)  
✅ Create Start Menu shortcuts  
✅ Create Desktop shortcut  
✅ Add to Add/Remove Programs  
✅ Create uninstaller  
✅ Set registry keys for proper uninstallation  

## Installer Customization

### Change Install Location
Edit `installer.nsi`, line 11:
```nsis
!define INSTALL_DIR "$PROGRAMFILES64\${APP_PUBLISHER}\${APP_NAME}"
```

### Add More Files
In the `Section "Main Application"` block, add:
```nsis
File "path\to\your\file.ext"
```

### Change Version Number
Edit `installer.nsi`, line 5:
```nsis
!define APP_VERSION "1.0.0"
```

### Customize UI
Add custom bitmaps (164x314 pixels for wizard, 150x57 for header):
```nsis
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "wizard.bmp"
```

## Qt Dependencies

The application requires these Qt 6 components:
- Qt6Core
- Qt6Gui
- Qt6Widgets
- Qt6Network
- Qt6WebEngineCore
- Qt6WebEngineWidgets
- Qt6WebChannel
- Qt6Sql
- Qt6Positioning

Plus Qt plugins:
- platforms/qwindows.dll
- styles/qwindowsvistastyle.dll
- sqldrivers/qsqlite.dll

And MinGW runtime:
- libgcc_s_seh-1.dll
- libstdc++-6.dll
- libwinpthread-1.dll

**Note**: `windeployqt` automatically copies these dependencies.

## Testing the Installer

1. **Run the installer** on a clean Windows VM or test machine
2. **Verify** the application starts correctly
3. **Check** all features work (maps, database, import/export)
4. **Test uninstall** to ensure clean removal

## Troubleshooting

### windeployqt not found
- Make sure Qt bin directory is in PATH
- Or use full path: `C:\Qt\6.7.0\mingw_64\bin\windeployqt.exe`

### Missing DLLs at runtime
- Run `windeployqt` again with `--verbose` flag
- Check Qt version matches your build
- Ensure Release build (not Debug)

### NSIS errors
- Check file paths in `installer.nsi`
- Ensure all referenced files exist
- Use absolute paths or relative to .nsi file location

### Icon not showing
- Convert SVG to ICO format first
- Place in `resources/icons/app-icon.ico`
- Rebuild the installer

## Distribution

Upload the installer to:
- GitHub Releases
- Your website
- Software download sites

Include:
- Installer: `MapAddress-1.0.0-Setup.exe`
- Portable ZIP: `MapAddress-1.0.0-Portable.zip` (optional)
- README/User Guide
- License file

## Code Signing (Optional but Recommended)

For production releases, sign the installer:

1. **Get a code signing certificate**
2. **Sign the EXE**:
   ```cmd
   signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com MapAddress-1.0.0-Setup.exe
   ```

This prevents Windows SmartScreen warnings.

## Advanced: MSI Installer with WiX

For enterprise deployment, consider WiX Toolset:
- https://wixtoolset.org/
- Creates .msi instead of .exe
- Better for Group Policy deployment
- More complex but professional

See `wix-installer.wxs` (if provided) for WiX template.

## Support

For issues with installer creation:
1. Check Qt version compatibility
2. Verify all paths in scripts
3. Test on clean Windows installation
4. Review NSIS documentation: https://nsis.sourceforge.io/Docs/
