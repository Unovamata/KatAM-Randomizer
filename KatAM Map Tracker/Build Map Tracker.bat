@echo off
setlocal

rem Builds the KatAM Map Tracker into a single executable: "KatAM Map Tracker\publish\KatAM Map Tracker.exe".
rem The executable is self-contained, so players don't need to install .NET to run it.
rem Building it requires the .NET 8 SDK: https://dotnet.microsoft.com/download/dotnet/8.0

cd /d "%~dp0"

where dotnet >nul 2>nul
if errorlevel 1 (
    echo The .NET SDK wasn't found. Install the .NET 8 SDK from https://dotnet.microsoft.com/download/dotnet/8.0
    pause
    exit /b 1
)

echo Building KatAM Map Tracker...

dotnet publish "KatAM Map Tracker.csproj" -c Release -r win-x64 --self-contained true ^
    -p:PublishSingleFile=true ^
    -p:IncludeNativeLibrariesForSelfExtract=true ^
    -p:EnableCompressionInSingleFile=true ^
    -o "publish"

if errorlevel 1 (
    echo.
    echo The build failed. Check the messages above.
    pause
    exit /b 1
)

echo.
echo Done! The tracker is in: %~dp0publish
echo   - KatAM Map Tracker.exe
echo   - Lua\katam_tracker.lua  (load it in mGBA: Tools ^> Scripting... ^> File ^> Load script)
echo.
pause
