/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


function command(cmd) {
  location.href = 'command://' + cmd;
}


function onClickFacebook() {
  command('facebook');
}
