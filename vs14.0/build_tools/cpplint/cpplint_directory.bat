@echo off
echo cpplint starts.
setlocal EnableDelayedExpansion

set CppLintDir=%1
set TargetDir=%2

set Index=0
set Files=
for /r %TargetDir% %%f in (*.h *.cc) do (
  set /a Index+=1
  set Files=!Files! %%f
  if !Index! geq 50 (  rem command-line string limitation
    call:executeLint "%CppLintDir%/cpplint.py" "!Files!"
	if !errorlevel! neq 0 (
      echo Error: cpplint
      exit /b !errorlevel!
    )
	set Files=
	set Index=0
  )
)

call:executeLint "%CppLintDir%/cpplint.py" "%Files%"
if %errorlevel% neq 0 (
  echo Error: cpplint
  exit /b %errorlevel%
)
echo cpplint ends.
exit /b %errorlevel%


:executeLint
pushd "%~dp0"
call python.exe %1 --output=vs7 %~2
popd
exit /b %errorlevel%
