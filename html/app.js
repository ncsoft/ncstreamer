/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


function command(cmd, args) {
  let argsArr = [];
  for (const key in args) {
    if (args.hasOwnProperty(key)) {
      argsArr.push([key, encodeURIComponent(args[key])].join('='));
    }
  }
  const uri = ['command://', cmd, '?', argsArr.join('&')].join('');
  location.href = uri;
}


function onClickFacebook() {
  createFacebookLiveVideo(function(streamUrl) {
    command('streaming/start', {
      serviceProvider: 'facebook',
      streamUrl: streamUrl,
    });
  });
}
