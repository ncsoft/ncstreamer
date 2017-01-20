@echo off

set SrcRoot=../../html

call gjslint.exe %SrcRoot%/app.js
call gjslint.exe -nojsdoc %SrcRoot%/facebook.js
call gjslint.exe -nojsdoc %SrcRoot%/main.js
