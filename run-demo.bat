@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo [FastString] Running Demo (via JitPack)...
cd examples\Demo
call mvn compile exec:java -Dexec.mainClass=faststring.Demo
cd ..\..
pause
