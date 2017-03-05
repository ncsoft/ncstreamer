/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


function importScript(file) {
  const elem = document.createElement('script');
  elem.src = file;
  document.body.appendChild(elem);
}


importScript('lib/facebook.js');


const app = {
  dom: {},
  streaming: {
    status: 'standby',  // ['standby', 'starting', 'onAir', 'stopping']
  },
};


document.addEventListener('DOMContentLoaded', function(event) {
  [
    'streaming-sources-select',
    'streaming-status',
    'streaming-button',
  ].forEach(function(domId) {
    app.dom[toCamel(domId)] = document.getElementById(domId);
  });

  app.dom.streamingButton.addEventListener('click', onStreamingButtonClicked);
});


function toCamel(str) {
  return str.replace(/(\-[a-z])/g, function(match) {
    return match.toUpperCase().replace('-', '');
  });
}


function updateStreamingStatus(status) {
  console.info({ status: status });

  app.streaming.status = status;
  const message = app.dom.streamingStatus;
  const button = app.dom.streamingButton;
  switch(status) {
    case 'standby':
      message.textContent = 'Status: ' + status;
      button.name = 'Start Streaming';
      button.disabled = false;
      break;
    case 'starting':
      message.textContent = 'Status: ' + status;
      button.name = 'Starting';
      button.disabled = true;
      break;
    case 'onAir':
      message.textContent = 'Status: ' + status;
      button.name = 'Stop Streaming';
      button.disabled = false;
      break;
    case 'stopping':
      message.textContent = 'Status: ' + status;
      button.name = 'Stopping';
      button.disabled = true;
      break;
  }
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


function setUpStreamingSources(obj) {
  if (!obj.hasOwnProperty('sources'))
    return;

  for (const child of app.dom.streamingSourcesSelect.children) {
    child.remove();
  }

  for (const source of obj.sources) {
    const option = document.createElement('option');
    option.value = source;
    option.text = source.split(':')[0];
    app.dom.streamingSourcesSelect.add(option);
  }
}


function onStreamingButtonClicked() {
  ({
    'standby': function() {
      facebook.createLiveVideo(function(streamUrl) {
        const source = app.dom.streamingSourcesSelect.value;
        command('streaming/start', {
          serviceProvider: 'Facebook Live',
          streamUrl: streamUrl,
          source: source,
        });
      });
      updateStreamingStatus('starting');
    },
    'starting': function() {},
    'onAir': function() {
      command('streaming/stop');
      updateStreamingStatus('stopping');
    },
    'stopping': function() {},
  })[app.streaming.status]();
}


function onStreamingStarted() {
  updateStreamingStatus('onAir');
}


function onStreamingStopped() {
  updateStreamingStatus('standby');
}
