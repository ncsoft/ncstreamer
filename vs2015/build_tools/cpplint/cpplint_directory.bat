@echo off
echo cpplint starts.
setlocal EnableDelayedExpansion

set TargetDir=%1

set FilesAll=
for /r %TargetDir% %%f in (*.h *.cc) do (
  set FilesAll=!FilesAll! %%f
)

pushd "%~dp0"
call python.exe cpplint.py %FilesAll%
if %errorlevel% neq 0 (
  echo Error: cpplint
  exit /b %errorlevel%
)
popd
echo cpplint ends.
