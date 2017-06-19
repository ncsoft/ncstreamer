@echo off
setlocal EnableDelayedExpansion

@echo Generating static UI per locale begins.

set ProjectDir=%~1
set Configuration=%~2

robocopy /mir "%ProjectDir%..\ncstreamer_ui\static" "%ProjectDir%build\ncstreamer_ui\static_ui\static"
echo ROBOCOPY RESULT: %ERRORLEVEL%
if %ERRORLEVEL% GEQ 8 (
  exit /b 1
)
set ERRORLEVEL=0

set Generator=%ProjectDir%build\static_ui_generator\%Configuration%\static_ui_generator.exe
set Texts=%ProjectDir%..\ncstreamer_ui\localized_texts.json
set TemplateDir=%ProjectDir%..\ncstreamer_ui\template
set OutputDir=%ProjectDir%build\ncstreamer_ui\static_ui

set "startTime=%time: =0%"
"%Generator%" --texts="%Texts%" --input-dir="%TemplateDir%" --output-dir="%OutputDir%"
set "endTime=%time: =0%"

set "end=!endTime:%time:~8,1%=%%100)*100+1!"  &  set "start=!startTime:%time:~8,1%=%%100)*100+1!"
set /A "elap=((((10!end:%time:~2,1%=%%100)*60+1!%%100)-((((10!start:%time:~2,1%=%%100)*60+1!%%100)"

@echo Generating static UI per locale ends: %ERRORLEVEL%, elapsed[!elap! centi-seconds]
exit /b %ERRORLEVEL%
