# Quick Start: Building Windows Installer

## Step 1: Build on Windows

```powershell
# Clone and build
git clone <repository>
cd mapaddress
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

## Step 2: Create ICO Icon (One-time)

Using ImageMagick (download from https://imagemagick.org):
```powershell
cd ..\resources\icons
magick convert app-icon-256.png -define icon:auto-resize=16,32,48,64,128,256 app-icon.ico
```

## Step 3: Deploy Application

```cmd
cd ..\..\installer\windows
deploy.bat
```

## Step 4: Build Installer

Install NSIS first: https://nsis.sourceforge.io/Download

Then:
```cmd
makensis installer.nsi
```

Done! You'll have: `MapAddress-1.0.0-Setup.exe`

## Alternative: Use CMake CPack

```powershell
cd build
cpack -C Release -G NSIS
```

This creates the installer automatically!
