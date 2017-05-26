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
      response: ['userName', 'userLink', 'userPages', 'userPage', 'privacy'],
    },
    'service_provider/log_out': {
      request: ['serviceProvider'],
      response: ['error'],
    },
    'streaming/start': {
      request: ['source', 'userPage', 'privacy', 'title', 'description',
                'mic'],
      response: ['error'],
    },
    'streaming/stop': {
      request: [],
      response: ['error'],
    },
    'settings/video_quality/update': {
      request: ['width', 'height', 'fps', 'bitrate'],
      response: [],
    },
    'settings/mic/on': {
      request: [],
      response: [],
    },
    'settings/mic/off': {
      request: [],
      response: [],
    },
    'storage/user_page/update': {
      request: ['userPage'],
      response: [],
    },
    'storage/privacy/update': {
      request: ['privacy'],
      response: [],
    },
    'remote/status': {
      request: ['requestKey', 'status', 'sourceTitle', 'userName', 'quality'],
      response: [],
    },
    'remote/start': {
      request: ['requestKey', 'error'],
      response: [],
    },
    'remote/stop': {
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
        argsArr.push([key, encodeURIComponent(args[key])].join('='));
      }
    }
    const uri = ['cef://', type, '?', argsArr.join('&')].join('');
    location.href = uri;
  }
})();


const remote = {
  startRequestKey: null,
  stopRequestKey: null,
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

    const status = app.streaming.status;
    if (status != 'standby') {
      cef.remoteStart.request(requestKey, 'busy');
      return;
    }

    const success = ncsoft.select.setByText(app.dom.gameSelect, sourceTitle);
    if (!success) {
      cef.remoteStart.request(requestKey, 'unknown title');
      return;
    }

    const requested = submitControl();
    if (!requested) {
      cef.remoteStart.request(requestKey, 'no page');
      return;
    }

    remote.startRequestKey = requestKey;
  },
  onStreamingStopRequest: function(requestKey, args) {
    const sourceTitle = args.sourceTitle;

    const status = app.streaming.status;
    if (status != 'onAir') {
      cef.remoteStart.request(requestKey, 'idle');
      return;
    }

    if (sourceTitle != app.streaming.start.sourceTitle) {
      cef.remoteStart.request(requestKey, 'title mismatch');
      return;
    }

    const requested = submitControl();
    if (!requested) {
      // assert false
      return;
    }

    remote.stopRequestKey = requestKey;
  },
};
