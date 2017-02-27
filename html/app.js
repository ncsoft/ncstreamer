/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  streaming: {
    status: 'standby',  // ['standby', 'starting', 'onAir', 'stopping']
  },
};


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
  switch (app.streaming.status) {
    case 'standby': {
      createFacebookLiveVideo(function(streamUrl) {
        const select = document.getElementById('streaming-sources-select');
        const source = select.value;
        command('streaming/start', {
          serviceProvider: 'Facebook Live',
          streamUrl: streamUrl,
          source: source,
        });
        app.streaming.status = 'onAir';
      });
      break;
    }
    case 'onAir': {
      command('streaming/stop');
      app.streaming.status = 'standby';
    }
  }
}


function setUpStreamingSources(obj) {
  if (!obj.hasOwnProperty('sources'))
    return;
  const select = document.getElementById('streaming-sources-select');
  for (const source of obj.sources) {
    const option = document.createElement('option');
    option.text = source;
    select.add(option);
  }
}
