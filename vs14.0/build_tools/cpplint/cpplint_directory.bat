@echo off
echo cpplint starts.
setlocal EnableDelayedExpansion

set CppLintDir=%1
set TargetDir=%2

set Index=0
set FilesAll=
for /r %TargetDir% %%f in (*.h *.cc) do (
  set /a Index+=1
  set FilesAll=!FilesAll! %%f
  if !Index! geq 50 (  rem command-line string limitation
    pushd "%~dp0"
    call python.exe "%CppLintDir%/cpplint.py" --output=vs7 !FilesAll!
    if %errorlevel% neq 0 (
      echo Error: cpplint
      exit /b %errorlevel%
    )
    popd
	set FilesAll=
	set Index=0
  )
)

pushd "%~dp0"
call python.exe "%CppLintDir%/cpplint.py" --output=vs7 %FilesAll%
if %errorlevel% neq 0 (
  echo Error: cpplint
  exit /b %errorlevel%
)
popd
echo cpplint ends.
