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
    'service_provider/log_in': {
      request: ['serviceProvider'],
      response: ['userName', 'userPages'],
    },
    'streaming/start': {
      request: ['source', 'userPage', 'privacy', 'title', 'description'],
      response: [],
    },
    'streaming/stop': {
      request: [],
      response: [],
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
