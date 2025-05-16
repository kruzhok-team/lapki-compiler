@echo off
setlocal

echo [1/3] Run PyInstaller...
pyinstaller .\lapki-compiler.spec --noconfirm

if errorlevel 1 (
  echo Build error PyInstaller.
  goto :EOF
)

set "DIST_DIR=dist\lapki-compiler"
set "TARGET_DIR=%DIST_DIR%\"

set "INT_DIR1=%DIST_DIR%\_internal\compiler"
set "INT_DIR2=%DIST_DIR%\_internal\compiler"

if exist "%INT_DIR1%" (
    set "INTERNAL_COMPILER=%INT_DIR1%"
) else if exist "%INT_DIR2%" (
    set "INTERNAL_COMPILER=%INT_DIR2%"
) else (
    echo Cant find compiler at _internal.
    goto :EOF
)

echo [2/3] Moving compiler from %%INTERNAL_COMPILER%% at %TARGET_DIR%...
xcopy "%INTERNAL_COMPILER%" "%TARGET_DIR%" /E /I /Y >nul

echo [3/3] Remove internal copy...
rmdir /S /Q "%INTERNAL_COMPILER%"

for /F %%F in ('dir /B "%~dp0%DIST_DIR%\_internals" 2^>nul') do set HAS=1
if not defined HAS (
    rmdir /S /Q "%DIST_DIR%\_internals" 2>nul
)

echo Build completed!
endlocal