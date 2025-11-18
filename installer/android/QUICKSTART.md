# Quick Start: Android Build

## TL;DR - Fastest Way

### Option 1: Qt Creator (GUI - Easiest)
1. Open Qt Creator
2. Open `CMakeLists.txt`
3. Select Android kit
4. Press **Build → Build APK**
5. Done! APK in `build-*-Android/android-build/build/outputs/apk/`

### Option 2: Command Line
```bash
cd installer/android
./build-android.sh
```

## Prerequisites (One-Time Setup)

1. **Install Android Studio**
   - Download: https://developer.android.com/studio
   - Install Android SDK (API 33)
   - Install Android NDK (r25c)

2. **Install Qt for Android**
   - Run Qt Maintenance Tool
   - Select: Qt 6.x → Android (ARM64-v8a)

3. **Set Environment Variables**
   ```bash
   export ANDROID_SDK_ROOT=~/Android/Sdk
   export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/25.2.9519653
   export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
   ```

## ⚠️ Important Note: WebEngine Limitation

**Qt WebEngine is NOT available for Android.**

Your app currently uses `QWebEngineView` for maps, which **will not work on Android**.

### Solutions:

#### Option A: Use QtWebView (Native WebView)
Replace `QWebEngineView` with `QWebView`:
- Uses Android's built-in WebView
- Lighter weight
- Native performance

#### Option B: Use QtLocation + MapboxGL
- Native Qt maps
- Better mobile performance
- Offline capability

#### Option C: Hybrid Approach
```cpp
#ifdef Q_OS_ANDROID
    // Use QWebView or QtLocation
#else
    // Use QWebEngineView
#endif
```

## Build Output

Successfully built APK will be:
- **Location**: `MapAddress-android.apk`
- **Size**: ~40-60 MB (depending on Qt libraries)
- **Architecture**: ARM64-v8a

## Install on Device

```bash
# Enable USB debugging on your Android device
# Connect via USB

adb install MapAddress-android.apk

# View app logs
adb logcat | grep MapAddress
```

## File Locations

- **Source**: `/home/jailop/devel/datainquiry/mapaddress/`
- **Android configs**: `installer/android/`
- **Build output**: `build-android/`
- **APK**: `MapAddress-android.apk`

## Common Issues

**"Qt WebEngine not found"**
→ Normal! See WebEngine Limitation above

**"SDK not found"**
→ Set `ANDROID_SDK_ROOT` environment variable

**"NDK not found"**
→ Install: `sdkmanager --install "ndk;25.2.9519653"`

**"Build fails"**
→ Check Qt Creator → Devices → Android settings

## Next Steps

1. ✅ Fix WebEngine issue (choose solution A, B, or C above)
2. ✅ Test on Android device/emulator
3. ✅ Adjust UI for touch (larger buttons)
4. ✅ Request location permissions at runtime
5. ✅ Sign APK for release
6. ✅ Upload to Google Play Store

For detailed instructions, see `README.md`
