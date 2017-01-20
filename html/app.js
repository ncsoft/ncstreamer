/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


function command(cmd) {
  var uri = ['command://', cmd].join('');
  location.href = uri;
}


function onClickFacebook() {
  createFacebookLiveVideo();
}
