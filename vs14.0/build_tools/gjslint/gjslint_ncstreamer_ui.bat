@echo off

set SrcRoot=../../../ncstreamer_ui

@echo gjslint begins.
call gjslint.exe --nojsdoc --strict^
 %SrcRoot%/template/app.js^
 %SrcRoot%/static/app/cef.js^
 %SrcRoot%/static/lib/ncsoft/ncsoft.js

if %ERRORLEVEL% NEQ 0 (
  exit /b %ERRORLEVEL%
)

@echo gjslint ends.
exit /b %ERRORLEVEL%
