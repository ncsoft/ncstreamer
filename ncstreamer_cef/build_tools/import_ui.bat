@echo off
@echo Importing UI begins.

set ProjectDir=%~1
set Configuration=%~2

set FromDir=%ProjectDir%build\ncstreamer_ui\static_ui
if not exist "%FromDir%" (
  echo FromDir doesn't exist: %FromDir%
  exit /b 1
)

set ToDir=%ProjectDir%build\ncstreamer_cef\%Configuration%\bin\32bit\ui

robocopy /mir "%FromDir%" "%ToDir%"
echo ROBOCOPY RESULT: %ERRORLEVEL%
if %ERRORLEVEL% GEQ 8 (
  exit /b 1
)
set ERRORLEVEL=0

@echo Importing UI ends: %ERRORLEVEL%
exit /b %ERRORLEVEL%
