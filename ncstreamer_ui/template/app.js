/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  dom: {},
  streaming: {
    // ['standby', 'setup', 'starting', 'onAir', 'stopping']
    status: 'standby',
    start: {},
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
  options: {
    hidesSettings: false,
  },
  errorType: null,
};


document.addEventListener('contextmenu', function(event) {
  event.preventDefault();
});


document.addEventListener('DOMContentLoaded', function(event) {
  ncsoft.onDOMContentLoaded();

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
    'own-page-select',
    'privacy-select',
    'game-select',
    'feed-description',
    'mic-checkbox',
    'error-text',
    'caution-text',
    'live-image',
    'control-button',
    'provider-user-name-in-settings',
    'provider-user-disconnect-in-settings',
    'quality-select',
    'settings-confirm-button',
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
      'ncsoftSelectChange', onMePageSelectChanged);
  app.dom.ownPageSelect.addEventListener(
      'ncsoftSelectChange', onOwnPageSelectChanged);
  app.dom.privacySelect.addEventListener(
      'ncsoftSelectChange', onPrivacySelectChanged);
  app.dom.gameSelect.addEventListener(
      'ncsoftSelectChange', onGameSelectChanged);
  app.dom.micCheckbox.addEventListener(
      'change', onMicCheckboxChanged);
  app.dom.controlButton.addEventListener(
      'click', onControlButtonClicked);
  app.dom.providerUserDisconnectInSettings.addEventListener(
      'click', onProviderUserDisconnectInSettingsClicked);
  app.dom.qualitySelect.addEventListener(
      'ncsoftSelectChange', onQualitySelectChanged);

  ncsoft.select.disable(app.dom.privacySelect);
  ncsoft.select.disable(app.dom.gameSelect);
  setUpSteamingQuality();
});


function toCamel(str) {
  return str.replace(/(\-[a-z])/g, function(match) {
    return match.toUpperCase().replace('-', '');
  });
}


function updateStreamingStatus(status) {
  console.info({status: status});

  app.streaming.status = status;
  app.dom.cautionText.style.display = 'none';
  app.dom.liveImage.style.display = 'none';
  const button = app.dom.controlButton;
  switch (status) {
    case 'standby':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = false;
      break;
    case 'setup':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      app.dom.cautionText.style.display = 'block';
      break;
    case 'starting':
      ncsoft.klass.add(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = true;
      app.dom.cautionText.style.display = 'block';
      break;
    case 'onAir':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = false;
      app.dom.liveImage.style.display = 'block';
      break;
    case 'stopping':
      ncsoft.klass.add(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = true;
      break;
  }
}


function hideSettings() {
  app.options.hidesSettings = true;
  app.dom.settingButton.style.display = 'none';
}


function getCurrentUserPage() {
  return (app.dom.mePageSelect.children[0].value == 2) ?
      app.dom.ownPageSelect.children[0].value : 'me';
}


function setUpUserPage(userPage) {
  if (!userPage) {
    return;
  }

  if (userPage == 'me') {
    ncsoft.select.setByIndex(app.dom.mePageSelect, 0);
    updateDependentsOnMePageSelect();
  } else {
    ncsoft.select.setByIndex(app.dom.mePageSelect, 1);
    updateDependentsOnMePageSelect();

    ncsoft.select.setByValue(app.dom.ownPageSelect, userPage);
  }
}


function setUpPrivacy(privacy) {
  ncsoft.select.setByValue(app.dom.privacySelect, privacy);
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
    ncsoft.select.disable(gameSelect);
  } else {
    ncsoft.select.enable(gameSelect);
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
          app.dom.ownPageSelect.children[0].value].link :
      app.service.user.link;
  cef.externalBrowserPopUp.request(link);
}


function onMePageSelectChanged() {
  console.info('change mePageSelect');
  if (app.errorType == 'mePage select empty') {
    app.dom.errorText.style.display = 'none';
  }
  updateDependentsOnMePageSelect();

  const userPage = getCurrentUserPage();
  if (!userPage) {
    return;
  }
  cef.storageUserPageUpdate.request(userPage);
}


function updateDependentsOnMePageSelect() {
  const ownSelect = app.dom.ownPageSelect;
  const privacySelect = app.dom.privacySelect;
  if (app.dom.mePageSelect.children[0].value == 2) {
    ownSelect.style.display = 'block';
    privacySelect.style.display = 'none';
    ncsoft.select.disable(privacySelect);
  } else {
    ownSelect.style.display = 'none';
    privacySelect.style.display = 'block';
    ncsoft.select.enable(privacySelect);
  }
}


function onOwnPageSelectChanged() {
  console.info('change ownPageSelect');
  if (app.errorType == 'ownPage select empty') {
    app.dom.errorText.style.display = 'none';
  }
  const userPage = getCurrentUserPage();
  if (!userPage) {
    return;
  }
  cef.storageUserPageUpdate.request(userPage);
}


function onPrivacySelectChanged() {
  console.info('change privacySelect');
  if (app.errorType == 'privacy select empty') {
    app.dom.errorText.style.display = 'none';
  }

  const privacy = app.dom.privacySelect.children[0].value;
  cef.storagePrivacyUpdate.request(privacy);
}


function onGameSelectChanged() {
  console.info('change gameSelect');
  if (app.errorType == 'game select empty') {
    app.dom.errorText.style.display = 'none';
  }
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
  submitControl();
}


function submitControl() {
  if (app.errorType == 'fail streaming' ||
      app.errorType == 'stop streaming first') {
    app.dom.errorText.style.display = 'none';
  }
  return ({
    'standby': function() {
      if (!checkSelectValueValidation()) {
        return false;
      }

      const source = app.dom.gameSelect.children[0].value;
      const userPage = getCurrentUserPage();
      const privacy = app.dom.privacySelect.children[0].value;
      const description = app.dom.feedDescription.value;
      const mic = app.dom.micCheckbox.checked;
      if (source == '' || userPage == '' || privacy == '')
        return false;

      cef.streamingStart.request(
          source, userPage, privacy, '' /* title */, description, mic);
      app.streaming.start.sourceTitle =
          ncsoft.select.getText(app.dom.gameSelect);
      updateStreamingStatus('starting');
      return true;
    },
    'starting': function() {
      return true;
    },
    'onAir': function() {
      cef.streamingStop.request();
      updateStreamingStatus('stopping');
      return true;
    },
    'stopping': function() {
      return true;
    },
  })[app.streaming.status]();
}


function onProviderUserDisconnectInSettingsClicked() {
  console.info('click providerUserDisconnectInSettings');

  app.dom.settingsConfirmButton.click();

  if (app.streaming.status != 'standby') {
    setUpError('stop streaming first');
    return;
  }

  cef.serviceProviderLogOut.request('Facebook Live');
}


function onQualitySelectChanged() {
  updateQualitySelect();
}


function updateQualitySelect() {
  const curValue = app.dom.qualitySelect.children[0].value;
  const curQuality = app.streaming.quality[curValue];
  console.info({ streamingQuality: curValue });
  cef.settingsVideoQualityUpdate.request(
      curQuality.resolution.width,
      curQuality.resolution.height,
      curQuality.fps,
      curQuality.bitrate);
  return true;
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


function setUpError(type) {
  app.errorType = type;
  showErrorText();
}


function showErrorText() {
  const error = app.dom.errorText;
  switch (app.errorType) {
    case 'fail streaming':
      error.textContent = '%ERROR_MESSAGE%';
      break;
    case 'stop streaming first':
      error.textContent = '%STOP_STREAMING_FIRST%';
      break;
    case 'mePage select empty':
      error.textContent = '%NO_SELECT_ME_PAGE%';
      break;
    case 'privacy select empty':
      error.textContent = '%NO_SELECT_PRIVACY%';
      break;
    case 'ownPage select empty':
      error.textContent = '%NO_SELECT_OWN_PAGE%';
      break;
    case 'game select empty':
       error.textContent = '%NO_SELECT_GAME%';
       break;
    default:
      error.TextContent = '%ERROR_MESSAGE%';
      break;
  }
  error.style.display = 'block';
}


function checkSelectValueValidation() {
  const error = app.dom.errorText;
  if (app.dom.mePageSelect.children[0].value == '') {
    setUpError('mePage select empty');
    return false;
  }

  if (app.dom.mePageSelect.children[0].value == 1 &&
      app.dom.privacySelect.children[0].value == '') {
    setUpError('privacy select empty');
    return false;
  }

  if (app.dom.mePageSelect.children[0].value == 2 &&
      app.dom.ownPageSelect.children[0].value == '') {
    setUpError('ownPage select empty');
    return false;
  }

  if (app.dom.gameSelect.children[0].value == '') {
    setUpError('game select empty');
    return false;
  }
  return true;
}


cef.serviceProviderLogIn.onResponse = function(
    userName, userLink, userPages, userPage, privacy) {
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
  if (app.options.hidesSettings == false) {
  app.dom.settingButton.style.display = 'inline';
  }
  app.dom.minimizeButton.style.display = 'inline';

  app.dom.providerUserName.textContent = userName;
  app.dom.providerUserNameInSettings.textContent = userName;

  const ownPageSelect = app.dom.ownPageSelect;
  const display = ownPageSelect.children[0];
  const contents = ownPageSelect.children[1];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  if (userPages.length == 0) {
    ncsoft.select.disable(ownPageSelect);
  } else {
    ncsoft.select.enable(ownPageSelect);
    for (const ownPage of userPages) {
      const li = document.createElement('li');
      const aTag = document.createElement('a');
      aTag.textContent = ownPage.name;
      li.setAttribute('data-value', ownPage.id);
      li.appendChild(aTag);
      contents.appendChild(li);
    }
    display.value = contents.firstChild.getAttribute('data-value');
    display.innerHTML = contents.firstChild.firstChild.textContent +
                        '<span class="caret"></span>';
  }

  setUpUserPage(userPage);
  setUpPrivacy(privacy);

  app.dom.errorText.style.display = 'none';
};


cef.serviceProviderLogOut.onResponse = function(error) {
  if (error) {
    // TODO(khpark): TBD
    alert('Logout failed: ' + error);
    return;
  }

  app.service.user = {
    name: '',
    link: '',
    pages: {},
  };

  for (const element of app.dom.loginPagePanel) {
    element.style.display = 'block';
  }
  for (const element of app.dom.mainPagePanel) {
    element.style.display = 'none';
  }

  app.dom.providerUserName.textContent = '';
  app.dom.providerUserNameInSettings.textContent = '';

  const ownPages = app.dom.ownPageSelect.children[1];
  while (ownPages.firstChild) {
    ownPages.removeChild(ownPages.firstChild);
  }
  ncsoft.select.disable(ownPageSelect);

  setUpUserPage('me');
  setUpPrivacy('SELF');
};


cef.streamingStart.onResponse = function(error) {
  console.info(error);
  if (error != '') {
    setUpError('fail streaming');
    app.streaming.start.sourceTitle = null;
    updateStreamingStatus('standby');
  } else {
    updateStreamingStatus('onAir');
  }

  if (remote.startRequestKey) {
    cef.remoteStart.request(remote.startRequestKey, error);
    remote.startRequestKey = null;
  }
};


cef.streamingStop.onResponse = function(error) {
  app.streaming.start.sourceTitle = null;
  updateStreamingStatus('standby');

  if (remote.stopRequestKey) {
    cef.remoteStop.request(remote.stopRequestKey, error);
    remote.stopRequestKey = null;
  }
};


cef.settingsVideoQualityUpdate.onResponse = function(error) {
  if (remote.qualityUpdateRequestKey) {
    cef.remoteQualityUpdate.request(remote.qualityUpdateRequestKey, error);
    remote.qualityUpdateRequestKey = null;
  }
};
