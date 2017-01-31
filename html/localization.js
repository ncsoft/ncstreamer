/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


translateTo('ko');


function translateTo(language) {
  const strSet = document.getElementsByClassName('ids');
  for (const str of strSet) {
    const key = str.innerHTML;
    str.innerHTML = texts[key][language];
  }
}
