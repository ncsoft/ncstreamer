@echo off

set SrcRoot=../../html

call gjslint.exe %SrcRoot%/app.js
call gjslint.exe %SrcRoot%/app/lib/facebook.js
