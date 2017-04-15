#!/bin/sh

exec vs14.0/build_tools/cpplint/cpplint_directory.bat "$GOOGLE_STYLEGUIDE_PATH/cpplint" ncstreamer_cef/src
