/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const cef = (function() {
  const protocols = {
    'streaming/start': {
      request: ['serviceProvider', 'streamUrl', 'source'],
    },
    'streaming/stop': {
      request: [],
    },
    'settings/video_quality/update': {
      request: ['width', 'height', 'fps', 'bitrate'],
    },
    'settings/mic/on': {
      request: [],
    },
    'settings/mic/off': {
      request: [],
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
    };
  }

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
