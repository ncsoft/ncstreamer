/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const cef = (function() {
  const exports = {
    request: request,
  };

  return exports;


  // private functions
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
