/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const cef = (function() {
  const protocols = {
    'window/close': {
      request: [],
      response: [],
    },
    'window/minimize': {
      request: [],
      response: [],
    },
    'external_browser/pop_up': {
      request: ['uri'],
      response: [],
    },
    'service_provider/log_in': {
      request: ['serviceProvider'],
      response: ['error', '_id', 'accessToken', 'userName', 'userPages',
                 'streamServers', 'userPage', 'privacy', 'youtubePrivacy',
                 'streamServer', 'description', 'location'],
    },
    'service_provider/log_out': {
      request: ['serviceProvider'],
      response: ['error'],
    },
    'streaming/set_up': {
      request: [],
      response: ['error'],
    },
    'streaming/start': {
      request: ['source', 'streamServer', 'userPage', 'privacy', 'title',
                'description'],
      response: ['error', 'serviceProvider', 'streamUrl', 'videoId',
                 'postUrl'],
    },
    'streaming/stop': {
      request: [],
      response: ['error'],
    },
    'settings/video_quality/update': {
      request: ['width', 'height', 'fps', 'bitrate'],
      response: ['error'],
    },
    'settings/mic/search': {
      request: [],
      response: ['error', 'micList'],
    },
    'settings/mic/on': {
      request: ['deviceId', 'volume'],
      response: ['error', 'volume'],
    },
    'settings/mic/off': {
      request: [],
      response: ['error'],
    },
    'settings/mic/volume/update': {
      request: ['volume'],
      response: ['error', 'volume'],
    },
    'settings/webcam/search': {
      request: [],
      response: ['error', 'webcamList'],
    },
    'settings/webcam/on': {
      request: ['deviceId', 'normalWidth', 'normalHeight',
                'normalX', 'normalY'],
      response: ['error'],
    },
    'settings/webcam/off': {
      request: [],
      response: ['error'],
    },
    'settings/webcam/size/update': {
      request: ['normalWidth', 'normalHeight'],
      response: ['error', 'normalWidth', 'normalHeight'],
    },
    'settings/webcam/position/update': {
      request: ['normalX', 'normalY'],
      response: ['error', 'normalX', 'normalY'],
    },
    'settings/chroma_key/on': {
      request: ['color', 'similarity'],
      response: ['error'],
    },
    'settings/chroma_key/off': {
      request: [],
      response: ['error'],
    },
    'settings/chroma_key/color/update': {
      request: ['color'],
      response: ['error'],
    },
    'settings/chroma_key/similarity/update': {
      request: ['similarity'],
      response: ['error'],
    },
    'storage/user_page/update': {
      request: ['userPage'],
      response: [],
    },
    'storage/privacy/update': {
      request: ['privacy'],
      response: [],
    },
    'storage/stream_server/update': {
      request: ['streamServer'],
      response: [],
    },
    'storage/youtubePrivacy/update': {
      request: ['privacy'],
      response: [],
    },
    'remote/status': {
      request: ['requestKey', 'status', 'sourceTitle', 'userName', 'quality'],
      response: [],
    },
    'remote/start': {
      request: ['requestKey', 'error', 'source', 'userPage', 'privacy',
                'description', 'mic', 'serviceProvider', 'streamUrl',
                'postUrl', 'id', 'videoId', 'token'],
      response: [],
    },
    'remote/stop': {
      request: ['requestKey', 'error', 'source'],
      response: ['error'],
    },
    'remote/quality/update': {
      request: ['requestKey', 'error'],
      response: [],
    },
  };

  const exports = {};
  for (const type in protocols) {
    if (protocols.hasOwnProperty(type) == false) {
      continue;
    }
    const protocol = protocols[type];
    exports[camelize(type)] = {
      request: function(...values) {
        const obj = {};
        protocol.request.forEach(function(name, index) {
          obj[name] = values[index];
        });
        request(type, obj);
      },
      onResponse: function(...values) {
        // do nothing, by default.
        // override this, if necessary.
      },
    };
  }

  exports.onResponse = function(type, obj) {
    const protocol = protocols[type];
    const values = [];
    protocol.response.forEach(function(name) {
      values.push(obj[name]);
    });
    return exports[camelize(type)].onResponse(...values);
  };

  return exports;


  // private functions
  function camelize(str) {
    return str.replace(/([_\-\/])([a-z])/g, function(match, p1, p2) {
      return p2.toUpperCase();
    });
  }


  function request(type, args) {
    let argsArr = [];
    for (const key in args) {
      if (args.hasOwnProperty(key)) {
        argsArr.push([key, args[key]].join('='));
      }
    }
    window.sendEvent(type, argsArr.join(','));
  }
})();


const remote = {
  startRequestKey: '',
  stopRequestKey: '',
  qualityUpdateRequestKey: '',
  onStreamingStatusRequest: function(requestKey) {
    const status = app.streaming.status;
    const sourceTitle = app.dom.gameSelect.children[0].textContent;
    const userName = app.dom.providerUserName.textContent || '';
    const quality = app.dom.qualitySelect.children[0].value;
    cef.remoteStatus.request(
        requestKey, status, sourceTitle, userName, quality);
  },
  onStreamingStartRequest: function(requestKey, args) {
    const sourceTitle = args.sourceTitle;

    if (!app.dom.providerUserName.textContent) {
      cef.remoteStart.request(requestKey, 'no user');
      return;
    }

    const status = app.streaming.status;
    if (status != 'standby') {
      const startTitle = getTitleFromSource(app.streaming.startInfo.source);
      const errorType = (sourceTitle == startTitle) ?
          'not standby: self' : 'not standby: other';
      cef.remoteStart.request(requestKey, errorType);
      return;
    }

    const success = ncsoft.select.setByText(app.dom.gameSelect, sourceTitle);
    if (!success) {
      cef.remoteStart.request(requestKey, 'unknown title');
      return;
    }

    const errorType = submitControl();
    if (errorType) {
      cef.remoteStart.request(requestKey, errorType);
      return;
    }

    remote.startRequestKey = requestKey;
  },
  onStreamingStopRequest: function(requestKey, args) {
    const sourceTitle = args.sourceTitle;

    const status = app.streaming.status;
    if (status != 'onAir') {
      cef.remoteStop.request(requestKey, 'not onAir');
      return;
    }

    const startTitle = getTitleFromSource(app.streaming.startInfo.source);
    if (sourceTitle != startTitle) {
      cef.remoteStop.request(requestKey, 'title mismatch');
      return;
    }

    const errorType = submitControl();
    if (errorType) {
      cef.remoteStop.request(requestKey, errorType);
      return;
    }

    remote.stopRequestKey = requestKey;
  },
  onSettingsQualityUpdateRequest: function(requestKey, args) {
    const quality = args.quality;

    if (quality == ncsoft.select.getValue(app.dom.qualitySelect)) {
      cef.remoteQualityUpdate.request(requestKey, /*success*/ '');
      return;
    }

    const success = ncsoft.select.setByValue(app.dom.qualitySelect, quality);
    if (!success) {
      cef.remoteQualityUpdate.request(requestKey, 'unknown quality');
      return;
    }

    const requested = updateQualitySelect();
    if (!requested) {
      // assert false
      return;
    }

    remote.qualityUpdateRequestKey = requestKey;
  },
  onSettingsWebcamOnRequest: function(args) {
    app.streaming.webcam.curDeviceId = args.deviceId;
    app.streaming.webcam.size.width = args.normalWidth;
    app.streaming.webcam.size.height = args.normalHeight;
    app.streaming.webcam.position.x = args.normalX;
    app.streaming.webcam.position.y = args.normalY;
    app.dom.webcamCheckbox.checked = true;
  },
  onSettingsWebcamOffRequest: function() {
    app.dom.webcamCheckbox.checked = false;
  },
  onSettingsWebcamSizeRequest: function(args) {
    app.streaming.webcam.size.width = args.normalWidth;
    app.streaming.webcam.size.height = args.normalHeight;
  },
  onSettingsWebcamPositionRequest: function(args) {
    app.streaming.webcam.position.x = args.normalX;
    app.streaming.webcam.position.y = args.normalY;
  },
  onSettingsChromaKeyOnRequest: function(args) {
    app.streaming.webcam.chromaKey.color = args.color;
    app.streaming.webcam.chromaKey.similarity = args.similarity;
    app.dom.chromaKeyCheckbox.checked = true;
  },
  onSettingsChromaKeyOffRequest: function() {
    app.dom.chromaKeyCheckbox.checked = false;
  },
  onSettingsChromaKeyColorRequest: function(args) {
    app.streaming.webcam.chromaKey.color = args.color;
  },
  onSettingsChromaKeySimilarityRequest: function(args) {
    app.streaming.webcam.chromaKey.similarity = args.similarity;
  },
  onSettingsMicOnRequest: function(args) {
    app.streaming.mic.curDeviceId = args.deviceId;
    app.dom.micCheckbox.checked = true;
    ncsoft.slider.enable(app.dom.micVolume);
    app.streaming.mic.volume.value = args.volume;
    app.dom.micVolume.value = args.volume;
    ncsoft.slider.adjustRange(app.dom.micVolume);
  },
  onSettingsMicOffRequest: function(requestKey) {
    app.dom.micCheckbox.checked = false;
    ncsoft.slider.disable(app.dom.micVolume);
  },
  onStreamingUrlUpdateRequest: function(requestKey, args) {
    const url = args.url;
    console.info(url);
    if (app.streaming.status != 'onAir') {
      return;
    }
    app.dom.liveImage.style.display = 'none';
    app.dom.nctvText.style.display = 'block';
    app.dom.nctvLink.style.display = 'inline';

    app.streaming.nctvUrl = url;
    if (url == '') {
      app.dom.nctvLink.style.display = 'none';
    }
  },
};
