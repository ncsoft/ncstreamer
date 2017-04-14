@echo off

set SrcRoot=../../html

call gjslint.exe -nojsdoc %SrcRoot%/app.js
