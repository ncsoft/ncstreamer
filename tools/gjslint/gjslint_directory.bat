@echo off
echo gjslint starts.
setlocal EnableDelayedExpansion

set TargetDir=%1

set FilesAll=
for /r %TargetDir% %%f in (*.js) do (
  set FilesAll=!FilesAll! %%f
)

pushd "%~dp0"
call gjslint.exe %FilesAll%
if %errorlevel% neq 0 (
  echo Error: gjslint
  exit /b %errorlevel%
)
popd
echo gjslint ends.
