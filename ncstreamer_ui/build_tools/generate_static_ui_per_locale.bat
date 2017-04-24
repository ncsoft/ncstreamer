@echo off
@echo Generating static UI per locale begins.

set ProjectDir=%~1
set Configuration=%~2

set Generator=%ProjectDir%build\static_ui_generator\%Configuration%\static_ui_generator.exe
set Texts=%ProjectDir%..\ncstreamer_ui\localized_texts.json
set TemplateDir=%ProjectDir%..\ncstreamer_ui\template
set OutputDir=%ProjectDir%build\ncstreamer_ui\static_ui

call "%Generator%" --texts="%Texts%" --input-dir="%TemplateDir%" --output-dir="%OutputDir%"
@echo Generating static UI per locale ends.
