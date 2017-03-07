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
    // ['standby', 'setup', 'starting', 'onAir', 'stopping']
    status: 'standby',
    popupBrowserId: 0,
  },
};


document.addEventListener('DOMContentLoaded', function(event) {
  [
    'streaming-feed-description',
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
  switch (status) {
    case 'standby':
      message.textContent = '방송 준비 완료';
      button.textContent = '방송 시작';
      button.disabled = false;
      break;
    case 'setup':
      message.textContent = '방송 생성 중';
      button.textContent = '방송 시작';
      button.disabled = true;
      break;
    case 'starting':
      message.textContent = '방송 시작 중';
      button.textContent = '방송 시작';
      button.disabled = true;
      break;
    case 'onAir':
      message.textContent = '방송 중';
      button.textContent = '방송 종료';
      button.disabled = false;
      break;
    case 'stopping':
      message.textContent = '방송 종료 중';
      button.textContent = '방송 종료';
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
      const description = app.dom.streamingFeedDescription.value;
      facebook.createLiveVideo(description, function(streamUrl) {
        const source = app.dom.streamingSourcesSelect.value;
        command('streaming/start', {
          serviceProvider: 'Facebook Live',
          streamUrl: streamUrl,
          source: source,
        });
        updateStreamingStatus('starting');
      });
      updateStreamingStatus('setup');
    },
    'starting': function() {},
    'onAir': function() {
      command('streaming/stop');
      updateStreamingStatus('stopping');
    },
    'stopping': function() {},
  })[app.streaming.status]();
}


function onBeforePopupOpen(browserId) {
  if (app.streaming.status == 'setup') {
    app.streaming.popupBrowserId = browserId;
  }
}


function OnBeforePopupClose(browserId) {
  if (app.streaming.popupBrowserId == browserId) {
    app.streaming.popupBrowserId = 0;
    updateStreamingStatus('standby');
  }
}


function onStreamingStarted() {
  updateStreamingStatus('onAir');
}


function onStreamingStopped() {
  updateStreamingStatus('standby');
}
