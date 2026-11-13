@echo off
if "%~1"=="" (
    echo Usage: run.bat ^<ROM_file^>
    echo Examples:
    echo   run.bat Pong.ch8
    echo   run.bat tetris.ch8
    echo   run.bat space.ch8
    exit /b 1
)
chip8.exe "%~1"
