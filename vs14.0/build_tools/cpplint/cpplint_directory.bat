@echo off
echo cpplint starts.
setlocal EnableDelayedExpansion

set CppLintDir=%1
set TargetDir=%2

set FilesAll=
for /r %TargetDir% %%f in (*.h *.cc) do (
  set FilesAll=!FilesAll! %%f
)

pushd "%~dp0"
call python.exe "%CppLintDir%/cpplint.py" --output=vs7 %FilesAll%
if %errorlevel% neq 0 (
  echo Error: cpplint
  exit /b %errorlevel%
)
popd
echo cpplint ends.
