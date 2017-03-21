/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


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
