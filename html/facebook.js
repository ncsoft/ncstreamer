/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


var facebookAppIds = {
  production: '',  // TODO(khpark): TBD
  test: '1513651075629637'
};
var facebookAppId = facebookAppIds.test;


window.fbAsyncInit = function() {
  FB.init({
    appId: facebookAppId,
    xfbml: true,
    version: 'v2.8'
  });
  FB.AppEvents.logPageView();
};
