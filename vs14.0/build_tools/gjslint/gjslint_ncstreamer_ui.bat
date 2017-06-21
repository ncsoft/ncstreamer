@echo off

set SrcRoot=../../../ncstreamer_ui

call gjslint.exe --nojsdoc^
 %SrcRoot%/template/app.js^
 %SrcRoot%/static/app/cef.js^
 %SrcRoot%/static/lib/ncsoft/ncsoft.js
