/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const facebook = (function() {
  const exports = {
    createLiveVideo: createLiveVideo,
  };


  const facebookAppIds = {
    production: /*NC Streaming*/ '1789696898019802',
    alpha: /*NC Streaming - Alpha*/ '1795379417451550',
  };
  const facebookAppId = facebookAppIds.alpha;


  window.fbAsyncInit = function() {
    FB.init({
      appId: facebookAppId,
      xfbml: true,
      version: 'v2.8',
    });
    FB.AppEvents.logPageView();
  };


  function createLiveVideo(description, setUserName, streaming) {
    FB.getLoginStatus(function(response) {
      if (response.status == 'connected') {
        console.info({
          Facebook: 'Already logged in.',
        });
        createLiveVideoAfterLogin(description, setUserName, streaming);
      } else {
        FB.login(function(response) {
          if (!response.authResponse) {
            console.warn({
              Facebook: 'Login failed.',
            });
            return;
          }
          console.info({
            Facebook: 'Logged in, OK.',
            grantedScopes: response.authResponse.grantedScopes,
          });
          createLiveVideoAfterLogin(description, setUserName, streaming);
        }, {
          scope: 'publish_actions',
          return_scopes: true,
        });
      }
    });
  }


  function createLiveVideoAfterLogin(description, setUserName, streaming) {
    FB.api('/me', {fields: 'name'}, function(response) {
      if (response.error) {
        console.error({
          Facebook: response.error.message,
        });
        return;
      }
      console.info(response.name);
      setUserName(response.name);
    });
    FB.api('/me/live_videos', 'post', {description: description},
        function(response) {
      if (response.error) {
        console.error({
          Facebook: response.error.message,
        });
        return;
      }
      console.info({
        Facebook: '/me/live_videos',
        response: response,
      });
      streaming(response.stream_url);
    });
  }

  return exports;
})();
