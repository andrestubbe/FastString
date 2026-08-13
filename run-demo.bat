@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo ⚡ Building Main Project (FastString)...
call mvn install -DskipTests -q
if %ERRORLEVEL% NEQ 0 ( echo ❌ Main build failed. & pause & exit /b %ERRORLEVEL% )

echo 🚀 Running Interactive Visual Demo for FastString...
cd examples\Demo
call mvn compile exec:java -Dexec.mainClass=faststring.Demo -q

cd ..\..
pause
