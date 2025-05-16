@echo off
setlocal EnableDelayedExpansion

echo [1/3] Run PyInstaller...
pyinstaller --clean --name lapki-compiler compiler\__main__.py --onefile --noconfirm
if errorlevel 1 (
    echo ERROR: PyInstaller build failed.
    goto :EOF
)

set "TARGET_DIR=dist"
set "SOURCE_DIR=compiler"

echo [2/3] Copying selected items to %TARGET_DIR%...

@echo off
setlocal

if not exist "%TARGET_DIR%" mkdir "%TARGET_DIR%"

set "ITEMS=compiler\platforms compiler\library compiler\ACCESS_TOKENS.txt"

for %%X in (%ITEMS%) do (
    if exist "%%~X\" (
        echo Copying directory %%~X → %TARGET_DIR%\%%~nxX ...
        xcopy "%%~X" "%TARGET_DIR%\%%~nxX" /E /I /Y >nul
    ) else if exist "%%~X" (
        echo Copying file      %%~X → %TARGET_DIR%\%%~nxX ...
        copy /Y "%%~X" "%TARGET_DIR%\%%~nxX" >nul
    ) else (
        echo WARNING: %%~X not found!
    )
)

echo All specified items have been copied to %TARGET_DIR%\
endlocal
endlocal