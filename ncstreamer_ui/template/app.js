/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  dom: {},
  streaming: {
    // ['standby', 'setup', 'starting', 'onAir', 'stopping']
    status: 'standby',
    popupBrowserId: 0,
    quality: {
      high: {
        resolution: {
          width: 1280,
          height: 720,
        },
        fps: 30,
        bitrate: 4000,
      },
      medium: {
        resolution: {
          width: 1280,
          height: 720,
        },
        fps: 30,
        bitrate: 3000,
      },
      low: {
        resolution: {
          width: 854,
          height: 480,
        },
        fps: 30,
        bitrate: 1000,
      },
    },
  },
  service: {
    user: null,
  },
};


document.addEventListener('DOMContentLoaded', function(event) {
  [
    'login-page-panel',
    'main-page-panel',
  ].forEach(function(domCls) {
    app.dom[toCamel(domCls)] = document.getElementsByClassName(domCls);
  });

  [
    'streaming-setting-button',
    'streaming-minimize-button',
    'streaming-close-button',
    'streaming-login-button',
    'provider-user-name',
    'provider-page-link',
    'streaming-user-page-select',
    'streaming-managing-page-select',
    'streaming-page-access',
    'streaming-nctv-tooltip',
    'streaming-game-select',
    'streaming-feed-description',
    'streaming-mic-checkbox',
    'streaming-error-text',
    'streaming-normal-text',
    'streaming-control-button',
    'streaming-quality-select',
  ].forEach(function(domId) {
    app.dom[toCamel(domId)] = document.getElementById(domId);
  });

  app.dom.streamingSettingButton.addEventListener(
      'click', onStreamingSettingButtonClicked);
  app.dom.streamingMinimizeButton.addEventListener(
      'click', onStreamingMinimizeButtonClicked);
  app.dom.streamingCloseButton.addEventListener(
      'click', onStreamingCloseButtonClicked);
  app.dom.streamingLoginButton.addEventListener(
      'click', onStreamingLoginButtonClicked);
  app.dom.providerPageLink.addEventListener(
      'click', onProviderPageLinkClicked);
  app.dom.streamingUserPageSelect.addEventListener(
      'customSelectChange', onStreamingUserPageSelectChanged);
  app.dom.streamingManagingPageSelect.addEventListener(
      'customSelectChange', onStreamingManagingPageSelectChanged);
  app.dom.streamingPageAccess.addEventListener(
      'customSelectChange', onStreamingPageAccessChanged);
  app.dom.streamingGameSelect.addEventListener(
      'customSelectChange', onStreamingGameSelectChanged);
  app.dom.streamingMicCheckbox.addEventListener(
      'change', onStreamingMicCheckboxChanged);
  app.dom.streamingControlButton.addEventListener(
      'click', onStreamingControlButtonClicked);
  app.dom.streamingQualitySelect.addEventListener(
      'customSelectChange', onStreamingQualitySelectChanged);

  setUpSteamingQuality();
});


function toCamel(str) {
  return str.replace(/(\-[a-z])/g, function(match) {
    return match.toUpperCase().replace('-', '');
  });
}


function addClass(element, name) {
  element.className += ' ' + name;
}


function removeClass(element, name) {
  const check = new RegExp('(\\s|^)' + name + '(\\s|$)');
  element.className = element.className.replace(check, ' ').trim();
}


function updateStreamingStatus(status) {
  console.info({ status: status });

  app.streaming.status = status;
  const message = app.dom.streamingNormalText;
  const button = app.dom.streamingControlButton;
  switch (status) {
    case 'standby':
      removeClass(button, 'loading');
      message.textContent = '%READY_BROADCASTING%';
      button.textContent = '%START_BROADCASTING%';
      button.disabled = false;
      break;
    case 'setup':
      removeClass(button, 'loading');
      message.textContent = '%MAKING_BROADCASTING%';
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      break;
    case 'starting':
      addClass(button, 'loading');
      message.textContent = '%STARTING_BROADCASTING%';
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      break;
    case 'onAir':
      removeClass(button, 'loading');
      message.textContent = '%ONAIR%';
      button.textContent = '%END_BROADCASTING%';
      button.disabled = false;
      break;
    case 'stopping':
      addClass(button, 'loading');
      message.textContent = '%ENDING_BROADCASTING%';
      button.textContent = '%END_BROADCASTING%';
      button.disabled = true;
      break;
  }
}


function setUpStreamingSources(obj) {
  if (!obj.hasOwnProperty('sources'))
    return;

  const display = app.dom.streamingGameSelect.children[0];
  const contents = app.dom.streamingGameSelect.children[1];
  const input = app.dom.streamingGameSelect.children[2];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  if (obj.sources.length == 0) {
    display.textContent = '%NO_PLAYING_GAME%';
  } else {
    for (const source of obj.sources) {
      const li = document.createElement('li');
      const node = document.createTextNode(source.split(':')[0]);
      li.setAttribute('data-value', source);
      li.appendChild(node);
      contents.appendChild(li);
    }
    input.value = contents.firstChild.getAttribute('data-value');
    display.textContent = contents.firstChild.textContent;
  }
}


function onStreamingSettingButtonClicked() {
  console.info('click streamingSettingButton');
}


function onStreamingMinimizeButtonClicked() {
  console.info('click streamingMinimizeButton');
  cef.windowMinimize.request();
}


function onStreamingCloseButtonClicked() {
  console.info('click streamingCloseeButton');
  cef.windowClose.request();
}


function onStreamingLoginButtonClicked() {
  console.info('click streamingLoginButton');
  cef.serviceProviderLogIn.request('Facebook Live');
}


function onProviderPageLinkClicked() {
  console.info('click providerPageLink');

  const link = (app.dom.streamingUserPageSelect.children[2].value == 2) ?
      app.service.user.pages[
          app.dom.streamingManagingPageSelect.children[2].value].link :
      app.service.user.link;
  cef.externalBrowserPopUp.request(link);
}


function onStreamingUserPageSelectChanged() {
  console.info('change streamingUserPageSelect');
  const managingSelect = app.dom.streamingManagingPageSelect;
  const privacySelect = app.dom.streamingPageAccess;
  if (app.dom.streamingUserPageSelect.children[2].value == 2) {
    managingSelect.style.display = 'block';
    privacySelect.style.display = 'none';
  } else {
    managingSelect.style.display = 'none';
    privacySelect.style.display = 'block';
  }
}


function onStreamingManagingPageSelectChanged() {
  console.info('change streamingManagingPageSelect');
}


function onStreamingPageAccessChanged() {
  console.info('change streamingPageAccess');
  const input = app.dom.streamingPageAccess.children[2].value;
  if (input == 'EVERYONE') {
    app.dom.streamingNctvTooltip.style.display = 'none';
  } else {
    app.dom.streamingNctvTooltip.style.display = 'block';
  }
}


function onStreamingGameSelectChanged() {
  console.info('change streamingGameSelect');
}


function onStreamingMicCheckboxChanged() {
  console.info('change streamingMicCheckbox');
  if (app.dom.streamingMicCheckbox.checked) {
    console.info('mic on');
    cef.settingsMicOn.request();
  } else {
    console.info('mic off');
    cef.settingsMicOff.request();
  }
}


function onStreamingControlButtonClicked() {
  console.info('change streamingControlButton');
  ({
    'standby': function() {
      const source = app.dom.streamingGameSelect.children[2].value;
      const userPage = app.dom.streamingUserPageSelect.children[2].value == 2 ?
          app.dom.streamingManagingPageSelect.children[2].value : 'me';
      const privacy = app.dom.streamingPageAccess.children[2].value;
      const description = app.dom.streamingFeedDescription.value;
      if (source == '' || userPage == '' || privacy == '')
        return;

      cef.streamingStart.request(
          source, userPage, privacy, '' /* title */, description);
      updateStreamingStatus('starting');
    },
    'starting': function() {},
    'onAir': function() {
      cef.streamingStop.request();
      updateStreamingStatus('stopping');
    },
    'stopping': function() {},
  })[app.streaming.status]();
}


function onStreamingQualitySelectChanged() {
  const curValue = app.dom.streamingQualitySelect.children[2].value;
  const curQuality = app.streaming.quality[curValue];
  console.info({ streamingQuality: curValue });
  cef.settingsVideoQualityUpdate.request(
      curQuality.resolution.width,
      curQuality.resolution.height,
      curQuality.fps,
      curQuality.bitrate);
}


function setUpSteamingQuality() {
  const display = app.dom.streamingQualitySelect.children[0];
  const contents = app.dom.streamingQualitySelect.children[1];
  const input = app.dom.streamingQualitySelect.children[2];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  for (const level in app.streaming.quality) {
    if (!app.streaming.quality.hasOwnProperty(level))
      return;

    const quality = app.streaming.quality[level];
    const li = document.createElement('li');
    const node = document.createTextNode([
        level,
        quality.resolution.width + '*' + quality.resolution.height,
        'fps: ' + quality.fps,
        'bitrate: ' + quality.bitrate].join(', '));
    li.setAttribute('data-value', level);
    li.appendChild(node);
    contents.appendChild(li);
  }
  input.value = contents.firstChild.getAttribute('data-value');
  display.textContent = contents.firstChild.textContent;
  onStreamingQualitySelectChanged();
}


cef.serviceProviderLogIn.onResponse = function(userName, userLink, userPages) {
  app.service.user = {
    name: userName,
    link: userLink,
    pages: {},
  };
  for (const userPage of userPages) {
    app.service.user.pages[userPage.id] = userPage;
  }

  for (const element of app.dom.loginPagePanel) {
    element.style.display = 'none';
  }
  for (const element of app.dom.mainPagePanel) {
    element.style.display = 'block';
  }
  app.dom.streamingSettingButton.style.display = 'inline';
  app.dom.streamingMinimizeButton.style.display = 'inline';

  app.dom.providerUserName.textContent = userName;

  const display = app.dom.streamingManagingPageSelect.children[0];
  const contents = app.dom.streamingManagingPageSelect.children[1];
  const input = app.dom.streamingManagingPageSelect.children[2];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  if (userPages.length == 0) {
    display.textContent = '%NO_MANAGING_PAGE%';
  } else {
    for (const userPage of userPages) {
      const li = document.createElement('li');
      const node = document.createTextNode(userPage.name);
      li.setAttribute('data-value', userPage.id);
      li.appendChild(node);
      contents.appendChild(li);
    }
    input.value = contents.firstChild.getAttribute('data-value');
    display.textContent = contents.firstChild.textContent;
  }
};


cef.streamingStart.onResponse = function() {
  updateStreamingStatus('onAir');
};


cef.streamingStop.onResponse = function() {
  updateStreamingStatus('standby');
};
