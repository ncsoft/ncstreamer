@echo off

set SrcRoot=../../../ncstreamer_ui

call gjslint.exe -nojsdoc %SrcRoot%/template/app.js
call gjslint.exe -nojsdoc %SrcRoot%/static/app/cef.js
call gjslint.exe -nojsdoc %SrcRoot%/static/lib/ncsoft/ncsoft.js
