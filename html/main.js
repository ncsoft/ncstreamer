/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const streamingSpot = document.getElementById('main-streaming-spot-select');

streamingSpot.onchange = function() {
  const selPage = document.getElementById('main-select-page-row');
  const range = document.getElementById('main-share-range-row');
  const sharedSelect = document.getElementById('main-share-range-select');
  switch (streamingSpot.selectedIndex) {
    case 1:
      selPage.hidden = true;
      range.hidden = false;
      sharedSelect.disabled = false;
      break;
    case 2:
      selPage.hidden = false;
      range.hidden = true;
      sharedSelect.disabled = true;
      break;
    default:
      break;
  }
};
