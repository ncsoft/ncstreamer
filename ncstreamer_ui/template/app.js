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
    'setting-button',
    'minimize-button',
    'close-button',
    'login-button',
    'provider-user-name',
    'provider-page-link',
    'me-page-select',
    'managing-page-select',
    'privacy-select',
    'nctv-tooltip',
    'game-select',
    'feed-description',
    'mic-checkbox',
    'error-text',
    'caution-text',
    'live-image',
    'control-button',
    'quality-select',
  ].forEach(function(domId) {
    app.dom[toCamel(domId)] = document.getElementById(domId);
  });

  app.dom.settingButton.addEventListener(
      'click', onSettingButtonClicked);
  app.dom.minimizeButton.addEventListener(
      'click', onMinimizeButtonClicked);
  app.dom.closeButton.addEventListener(
      'click', onCloseButtonClicked);
  app.dom.loginButton.addEventListener(
      'click', onLoginButtonClicked);
  app.dom.providerPageLink.addEventListener(
      'click', onProviderPageLinkClicked);
  app.dom.mePageSelect.addEventListener(
      'customSelectChange', onMePageSelectChanged);
  app.dom.managingPageSelect.addEventListener(
      'customSelectChange', onManagingPageSelectChanged);
  app.dom.privacySelect.addEventListener(
      'customSelectChange', onPrivacySelectChanged);
  app.dom.nctvTooltip.addEventListener(
      'click', onNctvTooltipClosed);
  app.dom.gameSelect.addEventListener(
      'customSelectChange', onGameSelectChanged);
  app.dom.micCheckbox.addEventListener(
      'change', onMicCheckboxChanged);
  app.dom.controlButton.addEventListener(
      'click', onControlButtonClicked);
  app.dom.qualitySelect.addEventListener(
      'customSelectChange', onQualitySelectChanged);

  disableSelect(app.dom.privacySelect);
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
  app.dom.cautionText.style.display = 'none';
  app.dom.liveImage.style.display = 'none';
  const button = app.dom.controlButton;
  const error = app.dom.errorText;
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
      app.dom.cautionText.style.display = 'block';
      break;
    case 'starting':
      addClass(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      app.dom.cautionText.style.display = 'block';
      break;
    case 'onAir':
      removeClass(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = false;
      app.dom.liveImage.style.display = 'block';
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


function getCurrentUserPage() {
  return (app.dom.mePageSelect.children[0].value == 2) ?
      app.dom.managingPageSelect.children[0].value : 'me';
}


function updateStreamingSources(obj) {
  if (!obj.hasOwnProperty('sources')) {
    return;
  }
  const sources = obj.sources;

  const gameSelect = app.dom.gameSelect;
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


function onSettingButtonClicked() {
  console.info('click settingButton');
}


function onMinimizeButtonClicked() {
  console.info('click minimizeButton');
  cef.windowMinimize.request();
}


function onCloseButtonClicked() {
  console.info('click closeeButton');
  cef.windowClose.request();
}


function onLoginButtonClicked() {
  console.info('click loginButton');
  cef.serviceProviderLogIn.request('Facebook Live');
}


function onProviderPageLinkClicked() {
  console.info('click providerPageLink');
  const link = (app.dom.mePageSelect.children[0].value == 2) ?
      app.service.user.pages[
          app.dom.managingPageSelect.children[0].value].link :
      app.service.user.link;
  cef.externalBrowserPopUp.request(link);
}


function onMePageSelectChanged() {
  console.info('change mePageSelect');
  const managingSelect = app.dom.managingPageSelect;
  const privacySelect = app.dom.privacySelect;
  if (app.dom.mePageSelect.children[0].value == 2) {
    managingSelect.style.display = 'block';
    privacySelect.style.display = 'none';
    disableSelect(privacySelect);
  } else {
    managingSelect.style.display = 'none';
    privacySelect.style.display = 'block';
    enableSelect(privacySelect);
  }

  const userPage = getCurrentUserPage();
  cef.storageUserPageUpdate.request(userPage);
}


function onManagingPageSelectChanged() {
  console.info('change managingPageSelect');

  const userPage = getCurrentUserPage();
  cef.storageUserPageUpdate.request(userPage);
}


function onPrivacySelectChanged() {
  console.info('change privacySelect');
  const input = app.dom.privacySelect.children[0].value;
  const tooltip = app.dom.nctvTooltip;
  if (input == 'EVERYONE' || tooltip.hasOwnProperty('show-once')) {
    tooltip.style.display = 'none';
  } else {
    tooltip.style.display = 'block';
  }

  cef.storagePrivacyUpdate.request(input);
}


function onNctvTooltipClosed() {
  console.info('close nctvTooltip');
  Object.defineProperty(app.dom.nctvTooltip, 'show-once', {});
}


function onGameSelectChanged() {
  console.info('change gameSelect');
}


function onMicCheckboxChanged() {
  console.info('change micCheckbox');
  if (app.dom.micCheckbox.checked) {
    console.info('mic on');
    cef.settingsMicOn.request();
  } else {
    console.info('mic off');
    cef.settingsMicOff.request();
  }
}


function onControlButtonClicked() {
  console.info('change controlButton');
  ({
    'standby': function() {
      const source = app.dom.gameSelect.children[0].value;
      const userPage = getCurrentUserPage();
      const privacy = app.dom.privacySelect.children[0].value;
      const description = app.dom.feedDescription.value;
      const mic = app.dom.micCheckbox.checked;
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


function onQualitySelectChanged() {
  const curValue = app.dom.qualitySelect.children[0].value;
  const curQuality = app.streaming.quality[curValue];
  console.info({ streamingQuality: curValue });
  cef.settingsVideoQualityUpdate.request(
      curQuality.resolution.width,
      curQuality.resolution.height,
      curQuality.fps,
      curQuality.bitrate);
}


function setUpSteamingQuality() {
  const display = app.dom.qualitySelect.children[0];
  const contents = app.dom.qualitySelect.children[1];
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
  onQualitySelectChanged();
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
  app.dom.settingButton.style.display = 'inline';
  app.dom.minimizeButton.style.display = 'inline';

  app.dom.providerUserName.textContent = userName;

  const managingPageSelect = app.dom.managingPageSelect;
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
