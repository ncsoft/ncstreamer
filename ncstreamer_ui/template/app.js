/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  dom: {},
  streaming: {
    // ['standby', 'setup', 'starting', 'onAir', 'stopping', 'error']
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


document.addEventListener('contextmenu', function(event) {
  event.preventDefault();
});


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
    'streaming-caution-text',
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
  app.dom.streamingNctvTooltip.addEventListener(
      'click', onStreamingNctvTooltipClosed);
  app.dom.streamingGameSelect.addEventListener(
      'customSelectChange', onStreamingGameSelectChanged);
  app.dom.streamingMicCheckbox.addEventListener(
      'change', onStreamingMicCheckboxChanged);
  app.dom.streamingControlButton.addEventListener(
      'click', onStreamingControlButtonClicked);
  app.dom.streamingQualitySelect.addEventListener(
      'customSelectChange', onStreamingQualitySelectChanged);

  disableSelect(app.dom.streamingPageAccess);
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


function enableSelect(element) {
  element.children[0].style.display = 'block';
  element.children[2].style.display = 'none';
}


function disableSelect(element) {
  element.children[0].style.display = 'none';
  element.children[2].style.display = 'block';
}


function updateStreamingStatus(status) {
  console.info({status: status});

  app.streaming.status = status;
  app.dom.streamingCautionText.style.display = 'none';
  const button = app.dom.streamingControlButton;
  const error = app.dom.streamingErrorText;
  error.style.display = 'none';
  switch (status) {
    case 'standby':
      removeClass(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = false;
      break;
    case 'setup':
      removeClass(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      app.dom.streamingCautionText.style.display = 'block';
      break;
    case 'starting':
      addClass(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      app.dom.streamingCautionText.style.display = 'block';
      break;
    case 'onAir':
      removeClass(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = false;
      break;
    case 'stopping':
      addClass(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = true;
      break;
    case 'error':
      removeClass(button, 'loading');
      error.textContent = '%ERROR_MESSAGE%';
      error.style.display = 'block';
      button.textContent = '%START_BROADCASTING%';
      button.disabled = false;
      break;
  }
}


function updateStreamingSources(obj) {
  if (!obj.hasOwnProperty('sources')) {
    return;
  }
  const sources = obj.sources;

  const gameSelect = app.dom.streamingGameSelect;
  const display = gameSelect.children[0];
  const contents = gameSelect.children[1];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  if (sources.length == 0) {
    disableSelect(gameSelect);
  } else {
    enableSelect(gameSelect);
    for (const source of sources) {
      const li = document.createElement('li');
      const aTag = document.createElement('a');
      aTag.textContent = source.split(':')[0];
      li.setAttribute('data-value', source);
      li.appendChild(aTag);
      contents.appendChild(li);
    }
    display.value = contents.firstChild.getAttribute('data-value');
    display.innerHTML = contents.firstChild.firstChild.textContent +
                        '<span class="caret"></span>';
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
  const link = (app.dom.streamingUserPageSelect.children[0].value == 2) ?
      app.service.user.pages[
          app.dom.streamingManagingPageSelect.children[0].value].link :
      app.service.user.link;
  cef.externalBrowserPopUp.request(link);
}


function onStreamingUserPageSelectChanged() {
  console.info('change streamingUserPageSelect');
  const managingSelect = app.dom.streamingManagingPageSelect;
  const privacySelect = app.dom.streamingPageAccess;
  if (app.dom.streamingUserPageSelect.children[0].value == 2) {
    managingSelect.style.display = 'block';
    privacySelect.style.display = 'none';
    disableSelect(privacySelect);
  } else {
    managingSelect.style.display = 'none';
    privacySelect.style.display = 'block';
    enableSelect(privacySelect);
  }
}


function onStreamingManagingPageSelectChanged() {
  console.info('change streamingManagingPageSelect');
}


function onStreamingPageAccessChanged() {
  console.info('change streamingPageAccess');
  const input = app.dom.streamingPageAccess.children[0].value;
  const tooltip = app.dom.streamingNctvTooltip;
  if (input == 'EVERYONE' || tooltip.hasOwnProperty('show-once')) {
    tooltip.style.display = 'none';
  } else {
    tooltip.style.display = 'block';
  }
}


function onStreamingNctvTooltipClosed() {
  console.info('close streamingNctvTooltip');
  Object.defineProperty(app.dom.streamingNctvTooltip, 'show-once', {});
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
      const source = app.dom.streamingGameSelect.children[0].value;
      const userPage = app.dom.streamingUserPageSelect.children[0].value == 2 ?
          app.dom.streamingManagingPageSelect.children[0].value : 'me';
      const privacy = app.dom.streamingPageAccess.children[0].value;
      const description = app.dom.streamingFeedDescription.value;
      const mic = app.dom.streamingMicCheckbox.checked;
      if (source == '' || userPage == '' || privacy == '')
        return;

      cef.streamingStart.request(
          source, userPage, privacy, '' /* title */, description, mic);
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
  const curValue = app.dom.streamingQualitySelect.children[0].value;
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
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  for (const level in app.streaming.quality) {
    if (!app.streaming.quality.hasOwnProperty(level))
      return;

    const quality = app.streaming.quality[level];
    const li = document.createElement('li');
    const aTag = document.createElement('a');
    aTag.textContent = [
        level,
        quality.resolution.width + '*' + quality.resolution.height,
        'fps: ' + quality.fps,
        'bitrate: ' + quality.bitrate].join(', ');
    li.setAttribute('data-value', level);
    li.appendChild(aTag);
    contents.appendChild(li);
  }
  display.value = contents.firstChild.getAttribute('data-value');
  display.innerHTML = contents.firstChild.firstChild.textContent +
                      '<span class="caret"></span>';
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

  const managingPageSelect = app.dom.streamingManagingPageSelect;
  const display = managingPageSelect.children[0];
  const contents = managingPageSelect.children[1];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  if (userPages.length == 0) {
    disableSelect(managingPageSelect);
  } else {
    enableSelect(managingPageSelect);
    for (const userPage of userPages) {
      const li = document.createElement('li');
      const aTag = document.createElement('a');
      aTag.textContent = userPage.name;
      li.setAttribute('data-value', userPage.id);
      li.appendChild(aTag);
      contents.appendChild(li);
    }
    display.value = contents.firstChild.getAttribute('data-value');
    display.innerHTML = contents.firstChild.firstChild.textContent +
                        '<span class="caret"></span>';
  }
};


cef.streamingStart.onResponse = function(error) {
  console.info(error);
  if (error != "") {
    updateStreamingStatus('error');
  } else {
    updateStreamingStatus('onAir');
  }
};


cef.streamingStop.onResponse = function() {
  updateStreamingStatus('standby');
};
