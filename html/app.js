/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  streaming: {
    status: 'standby',  // ['standby', 'starting', 'onAir', 'stopping']
  },
};


function updateStreamingStatus(status) {
  console.info({ status: status });

  app.streaming.status = status;
  const div = document.getElementById('streaming-status');
  div.textContent = "Status: " + status;
}


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
      updateStreamingStatus('starting');
      createFacebookLiveVideo(function(streamUrl) {
        const select = document.getElementById('streaming-sources-select');
        const source = select.value;
        command('streaming/start', {
          serviceProvider: 'Facebook Live',
          streamUrl: streamUrl,
          source: source,
        });
      });
      break;
    }
    case 'onAir': {
      updateStreamingStatus('stopping');
      command('streaming/stop');
      break;
    }
    default: {
      break;
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


function onStreamingStarted() {
  updateStreamingStatus('onAir');
}


function onStreamingStopped() {
  updateStreamingStatus('standby');
}
