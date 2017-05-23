@echo on

set ProjectDir=%~1
set Configuration=%~2


call "%ProjectDir%..\ncstreamer_ui\build_tools\generate_static_ui_per_locale.bat" %ProjectDir% "%Configuration%"
if %ERRORLEVEL% NEQ 0 (
  exit /b %ERRORLEVEL%
)

call "%ProjectDir%..\ncstreamer_cef\build_tools\import_ui.bat" %ProjectDir% "%Configuration%"
if %ERRORLEVEL% NEQ 0 (
  exit /b %ERRORLEVEL%
)

exit /b %ERRORLEVEL%
