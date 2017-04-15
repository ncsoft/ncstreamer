@echo off

set SrcRoot=../../html

call gjslint.exe -nojsdoc %SrcRoot%/app.js
call gjslint.exe -nojsdoc %SrcRoot%/app/cef.js
call gjslint.exe -nojsdoc %SrcRoot%/app/custom_style.js
