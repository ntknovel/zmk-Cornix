@echo off
cd /d "%~dp0"
where py >nul 2>nul
if not errorlevel 1 (
  py -3 tools\update_quick_macro_text.py
) else (
  python tools\update_quick_macro_text.py
)
if errorlevel 1 pause
