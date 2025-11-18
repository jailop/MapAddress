# Android Build Guide for MapAddress

This guide explains how to build an Android APK for MapAddress using Qt.

## Prerequisites

### 1. Install Required Tools

#### Android SDK
1. Download Android Studio from https://developer.android.com/studio
2. Install Android SDK (API Level 33 or 34 recommended)
3. Install Android NDK (r25c or later)
4. Location: `~/Android/Sdk/` (Linux/Mac) or `C:\Users\<username>\AppData\Local\Android\Sdk` (Windows)

#### Qt for Android
1. Install Qt for Android from Qt Maintenance Tool
2. Required components:
   - Qt 6.x for Android (ARM64-v8a and ARMv7)
   - Qt WebEngine for Android (if available)
   - Android Tools (SDK, NDK, OpenJDK)

### 2. Set Environment Variables

Add to `~/.bashrc` or `~/.zshrc`:

```bash
export ANDROID_SDK_ROOT=~/Android/Sdk
export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/25.2.9519653
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export PATH=$PATH:$ANDROID_SDK_ROOT/platform-tools
export PATH=$PATH:$ANDROID_SDK_ROOT/cmdline-tools/latest/bin
```

## Building the APK

### Method 1: Using Qt Creator (Recommended)

1. **Open Project in Qt Creator**
   ```bash
   qtcreator CMakeLists.txt
   ```

2. **Configure Android Kit**
   - Go to: Tools → Options → Devices → Android
   - Set SDK Location, NDK Location, JDK Location
   - Click "Apply"

3. **Select Android Kit**
   - Click "Projects" on left sidebar
   - Select "Android Qt 6.x.x ARM64" kit
   - Configure build settings (Release mode)

4. **Build and Deploy**
   - Select "Android Qt 6.x.x ARM64" from kit selector
   - Click Build → Build Project "MapAddress"
   - Click Build → Deploy Project "MapAddress"
   - Or: Press Ctrl+R to Build → Run → Deploy

5. **Create APK**
   - Build → Build APK
   - APK will be in: `build-MapAddress-Android_Qt_6_x_x_arm64-Release/android-build/build/outputs/apk/release/`

### Method 2: Command Line Build

1. **Configure CMake for Android**

```bash
mkdir build-android
cd build-android

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-33 \
  -DCMAKE_BUILD_TYPE=Release \
  -DQt6_DIR=$HOME/Qt/6.7.0/android_arm64_v8a/lib/cmake/Qt6 \
  -DCMAKE_FIND_ROOT_PATH=$HOME/Qt/6.7.0/android_arm64_v8a
```

2. **Build**

```bash
make -j$(nproc)
```

3. **Create APK with androiddeployqt**

```bash
$HOME/Qt/6.7.0/android_arm64_v8a/bin/androiddeployqt \
  --input android-MapAddress-deployment-settings.json \
  --output android-build \
  --android-platform android-33 \
  --jdk $JAVA_HOME \
  --gradle \
  --release
```

## Android-Specific Configuration

### 1. Create AndroidManifest.xml

Create `installer/android/AndroidManifest.xml`:

```xml
<?xml version="1.0"?>
<manifest package="com.datainquiry.mapaddress" 
          xmlns:android="http://schemas.android.com/apk/res/android" 
          android:versionName="1.0.0" 
          android:versionCode="1" 
          android:installLocation="auto">
    
    <application android:hardwareAccelerated="true" 
                 android:name="org.qtproject.qt.android.bindings.QtApplication" 
                 android:label="MapAddress"
                 android:icon="@drawable/icon">
        
        <activity android:configChanges="orientation|uiMode|screenLayout|screenSize|smallestScreenSize|layoutDirection|locale|fontScale|keyboard|keyboardHidden|navigation|mcc|mnc|density" 
                  android:name="org.qtproject.qt.android.bindings.QtActivity" 
                  android:label="MapAddress" 
                  android:screenOrientation="unspecified" 
                  android:launchMode="singleTop"
                  android:exported="true">
            <intent-filter>
                <action android:name="android.intent.action.MAIN"/>
                <category android:name="android.intent.category.LAUNCHER"/>
            </intent-filter>
            
            <meta-data android:name="android.app.lib_name" android:value="MapAddress"/>
            <meta-data android:name="android.app.qt_sources_resource_id" android:resource="@array/qt_sources"/>
            <meta-data android:name="android.app.repository" android:value="default"/>
            <meta-data android:name="android.app.qt_libs_resource_id" android:resource="@array/qt_libs"/>
            <meta-data android:name="android.app.bundled_libs_resource_id" android:resource="@array/bundled_libs"/>
        </activity>
    </application>
    
    <!-- Permissions -->
    <uses-permission android:name="android.permission.INTERNET"/>
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"/>
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
    <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION"/>
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"/>
    
    <uses-sdk android:minSdkVersion="28" android:targetSdkVersion="33"/>
</manifest>
```

### 2. Update CMakeLists.txt for Android

Add to the end of `CMakeLists.txt`:

```cmake
# Android-specific configuration
if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/installer/android"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 33
    )
    
    # Set Android app details
    set(ANDROID_PACKAGE_NAME "com.datainquiry.mapaddress")
    set(ANDROID_APP_NAME "MapAddress")
    set(ANDROID_VERSION_CODE 1)
    set(ANDROID_VERSION_NAME "1.0.0")
endif()
```

### 3. Add App Icon

Convert your app icon to Android drawable format:

```bash
# Create required drawable directories
mkdir -p installer/android/res/drawable-hdpi
mkdir -p installer/android/res/drawable-mdpi
mkdir -p installer/android/res/drawable-xhdpi
mkdir -p installer/android/res/drawable-xxhdpi
mkdir -p installer/android/res/drawable-xxxhdpi

# Convert icons (requires ImageMagick)
convert resources/icons/app-icon-512.png -resize 72x72 installer/android/res/drawable-hdpi/icon.png
convert resources/icons/app-icon-512.png -resize 48x48 installer/android/res/drawable-mdpi/icon.png
convert resources/icons/app-icon-512.png -resize 96x96 installer/android/res/drawable-xhdpi/icon.png
convert resources/icons/app-icon-512.png -resize 144x144 installer/android/res/drawable-xxhdpi/icon.png
convert resources/icons/app-icon-512.png -resize 192x192 installer/android/res/drawable-xxxhdpi/icon.png
```

## Troubleshooting

### Qt WebEngine on Android

**Important**: Qt WebEngine is **NOT available** for Android in standard Qt builds. The map display will not work as-is.

**Solutions**:

#### Option 1: Use Qt WebView (Recommended)
Replace QWebEngineView with QWebView which uses Android's native WebView:

1. Update CMakeLists.txt: Replace `Qt6::WebEngineWidgets` with `Qt6::WebView`
2. Update code: Replace QWebEngineView with QWebView
3. QWebView uses system WebView (lighter and native)

#### Option 2: Use QtLocation + MapboxGL
Use Qt Location with Mapbox GL plugin for native maps:
- Better performance on mobile
- Native Android integration
- No WebEngine dependency

#### Option 3: Embed Leaflet in QAndroidWebView
Use Android's WebView directly via JNI

### Common Issues

**Issue**: SDK/NDK not found
```bash
# Solution: Set paths in Qt Creator
Tools → Options → Devices → Android
Set correct paths and click "Apply"
```

**Issue**: Build fails with "No Android toolchain"
```bash
# Solution: Install NDK
sdkmanager --install "ndk;25.2.9519653"
```

**Issue**: Missing gradle
```bash
# Solution: Qt Creator will download automatically
# Or install manually:
sdkmanager --install "cmdline-tools;latest"
```

**Issue**: APK not installing on device
```bash
# Solution: Enable USB debugging and install via adb
adb install -r MapAddress-release-signed.apk
```

## Signing the APK

### 1. Create Keystore (First Time Only)

```bash
keytool -genkey -v -keystore mapaddress-release.keystore \
  -alias mapaddress -keyalg RSA -keysize 2048 -validity 10000
```

### 2. Sign APK

```bash
jarsigner -verbose -sigalg SHA256withRSA -digestalg SHA-256 \
  -keystore mapaddress-release.keystore \
  build/outputs/apk/release/MapAddress-release-unsigned.apk \
  mapaddress
```

### 3. Zipalign

```bash
$ANDROID_SDK_ROOT/build-tools/33.0.0/zipalign -v 4 \
  MapAddress-release-unsigned.apk \
  MapAddress-release-signed.apk
```

## Distribution

### Google Play Store

1. Create developer account: https://play.google.com/console
2. Create app listing
3. Upload signed APK
4. Set pricing and distribution
5. Submit for review

### Alternative Distribution

- Host APK on your website
- Distribute via email/cloud storage
- F-Droid (open source apps)
- Amazon Appstore
- Samsung Galaxy Store

## Testing on Device

```bash
# Install via USB
adb install MapAddress-release-signed.apk

# View logs
adb logcat | grep MapAddress

# Uninstall
adb uninstall com.datainquiry.mapaddress
```

## App Size Optimization

```cmake
# In CMakeLists.txt
if(ANDROID)
    # Strip symbols for smaller APK
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
    
    # Use ProGuard for further optimization
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_ABIS "arm64-v8a"  # Build only ARM64
    )
endif()
```

## Limitations on Android

1. **No Qt WebEngine** - Need alternative map display
2. **Storage Access** - Requires runtime permissions
3. **GPS Permission** - Need to request at runtime
4. **Battery Usage** - Maps can drain battery
5. **Screen Sizes** - Test on multiple devices

## Recommended Changes for Mobile

1. **Touch-Friendly UI**
   - Larger buttons (48dp minimum)
   - Touch targets with padding
   - Swipe gestures

2. **Mobile Navigation**
   - Bottom navigation bar
   - Drawer menu
   - Back button handling

3. **Performance**
   - Lazy loading
   - Image optimization
   - Network caching

4. **Permissions Handling**
   ```cpp
   // Request permissions at runtime
   QtAndroid::requestPermissions(
       QStringList() << "android.permission.ACCESS_FINE_LOCATION"
   );
   ```

## Complete Build Script

Save as `build-android.sh`:

```bash
#!/bin/bash
set -e

echo "Building MapAddress for Android..."

# Configuration
QT_DIR=$HOME/Qt/6.7.0/android_arm64_v8a
BUILD_DIR=build-android
APK_OUTPUT=MapAddress-release.apk

# Clean previous build
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-33 \
  -DCMAKE_BUILD_TYPE=Release \
  -DQt6_DIR=$QT_DIR/lib/cmake/Qt6

# Build
make -j$(nproc)

# Deploy
$QT_DIR/bin/androiddeployqt \
  --input android-MapAddress-deployment-settings.json \
  --output android-build \
  --android-platform android-33 \
  --gradle \
  --release

echo "APK created: android-build/build/outputs/apk/release/$APK_OUTPUT"
```

## Support

For issues:
- Qt Android documentation: https://doc.qt.io/qt-6/android.html
- Stack Overflow: tag `qt` + `android`
- Qt Forum: https://forum.qt.io/category/31/android
