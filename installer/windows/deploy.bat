@echo off
REM Windows Deployment Script for MapAddress
REM This script packages the application with all required Qt dependencies

echo ==================================
echo MapAddress Windows Deployment
echo ==================================

REM Set paths (adjust these for your environment)
set QT_DIR=C:\Qt\6.7.0\mingw_64
set BUILD_DIR=..\..\build
set DEPLOY_DIR=.\deploy
set APP_NAME=MapAddress.exe

echo.
echo Cleaning previous deployment...
if exist "%DEPLOY_DIR%" rmdir /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%"

echo.
echo Copying application executable...
copy "%BUILD_DIR%\%APP_NAME%" "%DEPLOY_DIR%\" || (
    echo ERROR: Could not find %BUILD_DIR%\%APP_NAME%
    echo Make sure you have built the application in Release mode
    exit /b 1
)

echo.
echo Running windeployqt...
"%QT_DIR%\bin\windeployqt.exe" ^
    --release ^
    --no-translations ^
    --no-system-d3d-compiler ^
    --no-opengl-sw ^
    "%DEPLOY_DIR%\%APP_NAME%" || (
    echo ERROR: windeployqt failed
    exit /b 1
)

echo.
echo Copying additional files...
copy "..\..\README.md" "%DEPLOY_DIR%\README.txt"

echo.
echo Creating portable archive...
cd "%DEPLOY_DIR%"
powershell Compress-Archive -Path * -DestinationPath ..\MapAddress-1.0.0-Portable.zip -Force
cd ..

echo.
echo ==================================
echo Deployment complete!
echo ==================================
echo.
echo Files are in: %DEPLOY_DIR%
echo Portable ZIP: MapAddress-1.0.0-Portable.zip
echo.
echo To create installer:
echo   1. Install NSIS from https://nsis.sourceforge.io/
echo   2. Run: makensis installer.nsi
echo.

pause
