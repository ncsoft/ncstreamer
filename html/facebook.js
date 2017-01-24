/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const facebookAppIds = {
  production: '',  // TODO(khpark): TBD
  test: '1513651075629637'
};
const facebookAppId = facebookAppIds.test;


window.fbAsyncInit = function() {
  FB.init({
    appId: facebookAppId,
    xfbml: true,
    version: 'v2.8'
  });
  FB.AppEvents.logPageView();
};


function createFacebookLiveVideo(callback) {
  FB.getLoginStatus(function(response) {
    if (response.status == 'connected') {
      console.info({
        Facebook: 'Already logged in.'
      });
      createFacebookLiveVideoAfterLogin(callback);
    } else {
      FB.login(function(response) {
        if (!response.authResponse) {
          console.warn({
            Facebook: 'Login failed.'
          });
          return;
        }
        console.info({
          Facebook: 'Logged in, OK.',
          grantedScopes: response.authResponse.grantedScopes
        });
        createFacebookLiveVideoAfterLogin(callback);
      }, {
        scope: 'publish_actions',
        return_scopes: true
      });
    }
  });
}


function createFacebookLiveVideoAfterLogin(callback) {
  FB.api('/me/live_videos', 'post', function(response) {
    if (response.error) {
      console.error({
        Facebook: 'response.error.message'
      });
      return;
    }
    console.info({
      Facebook: '/me/live_videos',
      response: response
    });
    callback(response.stream_url);
  });
}
