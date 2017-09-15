/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  dom: {},
  streaming: {
    // ['standby', 'setup', 'starting', 'onAir', 'stopping']
    status: 'standby',
    startInfo: {},
    popupBrowserId: 0,
    postUrl: null,
    mic: {
      use: true,
      volume: {
        max: 1,
        min: 0,
        step: 0.1,
        value: 0.5,
      }
    },
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
    provider: null,
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
    'connect-info-button',
    'setting-button',
    'minimize-button',
    'close-button',
    'login-button-facebook',
    'login-button-twitch',
    'provider-user-name',
    'connect-info-user-name',
    'connect-info-disconnect-button',
    'connect-info-msg',
    'connect-info-confirm-button',
    'provider-page-link',
    'facebook-division',
    'me-page-select',
    'own-page-select',
    'privacy-select',
    'twitch-division',
    'stream-server-select',
    'game-select',
    'feed-description',
    'mic-checkbox',
    'mic-volume',
    'error-text',
    'caution-text',
    'live-image',
    'control-button',
    'quality-select',
    'modal-close-button',
  ].forEach(function(domId) {
    app.dom[toCamel(domId)] = document.getElementById(domId);
  });

  app.dom.connectInfoButton.addEventListener(
      'click', onConnectInfoButtonClicked);
  app.dom.settingButton.addEventListener(
      'click', onSettingButtonClicked);
  app.dom.minimizeButton.addEventListener(
      'click', onMinimizeButtonClicked);
  app.dom.closeButton.addEventListener(
      'click', onCloseButtonClicked);
  app.dom.loginButtonFacebook.addEventListener(
      'click', onLoginButtonFacebookClicked);
  app.dom.loginButtonTwitch.addEventListener(
      'click', onLoginButtonTwitchClicked);
  app.dom.connectInfoDisconnectButton.addEventListener(
      'click', onConnectInfoDisconnectButtonClicked);
  app.dom.providerPageLink.addEventListener(
      'click', onProviderPageLinkClicked);
  app.dom.mePageSelect.addEventListener(
      'ncsoftSelectChange', onMePageSelectChanged);
  app.dom.ownPageSelect.addEventListener(
      'ncsoftSelectChange', onOwnPageSelectChanged);
  app.dom.privacySelect.addEventListener(
      'ncsoftSelectChange', onPrivacySelectChanged);
  app.dom.streamServerSelect.addEventListener(
      'ncsoftSelectChange', onStreamServerSelectChanged);
  app.dom.gameSelect.addEventListener(
      'ncsoftSelectChange', onGameSelectChanged);
  app.dom.micVolume.addEventListener(
      'ncsoftSelectChange', onMicVolumeChanged);
  app.dom.micCheckbox.addEventListener(
      'change', onMicCheckboxChanged);
  app.dom.controlButton.addEventListener(
      'click', onControlButtonClicked);
  app.dom.qualitySelect.addEventListener(
      'ncsoftSelectChange', onQualitySelectChanged);
  app.dom.modalCloseButton.addEventListener(
      'click', onModalCloseClicked);

  ncsoft.select.disable(app.dom.privacySelect);
  ncsoft.select.disable(app.dom.gameSelect);
  setUpSteamingQuality();
  setUpMic();
});


function toCamel(str) {
  return str.replace(/(\-[a-z])/g, function(match) {
    return match.toUpperCase().replace('-', '');
  });
}


function updateStreamingStatus(status) {
  console.info(JSON.stringify({status: status}));

  app.streaming.status = status;
  app.dom.cautionText.style.display = 'none';
  app.dom.liveImage.style.display = 'none';
  const button = app.dom.controlButton;
  switch (status) {
    case 'standby':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = false;
      ncsoft.klass.remove(app.dom.providerPageLink, 'live');
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
      ncsoft.klass.add(app.dom.providerPageLink, 'live');
      break;
    case 'stopping':
      ncsoft.klass.add(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = true;
      break;
  }
}


function setUp(args) {
  app.options.hidesSettings = (args.hidesSettings == 'true');

  if (app.options.hidesSettings == true) {
    app.dom.settingButton.style.display = 'none';
  }

  ncsoft.select.setByValue(app.dom.qualitySelect, args.videoQuality);
}


function setProviderUserName(userName) {
  app.dom.providerUserName.textContent = userName;
  app.dom.connectInfoUserName.textContent = userName;
}


function getCurrentUserPage() {
  return (app.dom.mePageSelect.children[0].value == 2) ?
      app.dom.ownPageSelect.children[0].value : 'me';
}


function setUpUserPage(userPages, userPage) {
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


function getTitleFromSource(source) {
  return source.split(':')[0];
}


function updateStreamingSources(obj) {
  if (!obj.hasOwnProperty('sources')) {
    return;
  }
  const sources = obj.sources;

  stopInvalidSource(sources);

  const gameSelect = app.dom.gameSelect;
  ncsoft.select.clear(gameSelect);

  if (sources.length == 0) {
    ncsoft.select.disable(gameSelect);
  } else {
    ncsoft.select.enable(gameSelect);

    const display = gameSelect.children[0];
    const contents = gameSelect.children[1];

    for (const source of sources) {
      const li = document.createElement('li');
      const aTag = document.createElement('a');
      aTag.textContent = getTitleFromSource(source);
      li.setAttribute('data-value', source);
      li.appendChild(aTag);
      contents.appendChild(li);
    }
    display.value = contents.firstChild.getAttribute('data-value');
    display.innerHTML = contents.firstChild.firstChild.textContent +
                        '<span class="caret"></span>';

    if (app.errorType == 'game select empty') {
      app.dom.errorText.style.display = 'none';
    }
  }
}


function stopInvalidSource(sources) {
  if (app.streaming.status != 'onAir') {
    return;
  }

  const currentSource = app.streaming.startInfo.source;
  if (sources.includes(currentSource) == true) {
    return;
  }

  console.info('stop invalid source: ' + currentSource);
  app.dom.controlButton.click();
}


function onConnectInfoButtonClicked() {
  console.info('click connectInfoButton');

  app.dom.connectInfoMsg.textContent = '';
}


function onSettingButtonClicked() {
  console.info('click settingButton');
}


function onMinimizeButtonClicked() {
  console.info('click minimizeButton');
  cef.windowMinimize.request();
}


function onCloseButtonClicked() {
  console.info('click closeButton');
  if (app.streaming.status == 'onAir') {
    ncsoft.modal.show('#close-check-modal');
  } else {
    cef.windowClose.request();
  }
}


function onLoginButtonFacebookClicked() {
  console.info('click loginButtonFacebook');
  app.service.provider = 'Facebook Live';
  cef.serviceProviderLogIn.request(app.service.provider);
}


function onLoginButtonTwitchClicked() {
  console.info('click loginButtonTwitch');
  app.service.provider = 'Twitch';
  cef.serviceProviderLogIn.request(app.service.provider);
}


function onProviderPageLinkClicked() {
  console.info('click providerPageLink');

  if (app.streaming.status != 'onAir') {
    return;
  }

  cef.externalBrowserPopUp.request(app.streaming.postUrl);
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


function onStreamServerSelectChanged() {
  console.info('change streamServerSelect');

  const server = app.dom.streamServerSelect.children[0].value;
  cef.storageStreamServerUpdate.request(server);
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
    const volume = app.dom.micVolume.value;
    cef.settingsMicOn.request(volume);
  } else {
    console.info('mic off');
    cef.settingsMicOff.request();
  }
}


function onMicVolumeChanged() {
  console.info('change micVolume');
  cef.settingsMicVolumeUpdate.request(app.dom.micVolume.value);
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
      const errorType = checkSelectValueValidation();
      if (errorType) {
        setUpError(errorType);
        return errorType;
      } else {
        app.dom.errorText.style.display = 'none';
      }

      const source = app.dom.gameSelect.children[0].value;
      const userPage = getCurrentUserPage();
      const privacy = app.dom.privacySelect.children[0].value;
      const description = app.dom.feedDescription.value;

      cef.streamingStart.request(
          source, userPage, privacy, '' /* title */, description);
      app.streaming.startInfo = {
        source: source,
        userPage: userPage,
        privacy: privacy,
        description: description,
      };
      updateStreamingStatus('starting');
      return /*no error*/ '';
    },
    'starting': function() {
      return /*no error*/ '';
    },
    'onAir': function() {
      cef.streamingStop.request();
      updateStreamingStatus('stopping');
      return /*no error*/ '';
    },
    'stopping': function() {
      return /*no error*/ '';
    },
  })[app.streaming.status]();
}


function onConnectInfoDisconnectButtonClicked() {
  console.info('click connectInfoDisconnectButton');

  if (app.streaming.status != 'standby') {
    app.dom.connectInfoMsg.textContent = '%STOP_STREAMING_FIRST%';
    return;
  }

  app.dom.connectInfoConfirmButton.click();
  cef.serviceProviderLogOut.request('Facebook Live');
}


function onQualitySelectChanged() {
  updateQualitySelect();
}


function updateQualitySelect() {
  const curValue = app.dom.qualitySelect.children[0].value;
  const curQuality = app.streaming.quality[curValue];
  console.info(JSON.stringify({ streamingQuality: curValue }));
  cef.settingsVideoQualityUpdate.request(
      curQuality.resolution.width,
      curQuality.resolution.height,
      curQuality.fps,
      curQuality.bitrate);
  return true;
}


function onModalCloseClicked() {
  console.info('click modalClose');
  cef.windowClose.request();
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
      'bitrate: ' + quality.bitrate,
    ].join(', ');
    li.setAttribute('data-value', level);
    li.appendChild(aTag);
    contents.appendChild(li);
  }
  display.value = contents.firstChild.getAttribute('data-value');
  display.innerHTML = contents.firstChild.firstChild.textContent +
                      '<span class="caret"></span>';
  onQualitySelectChanged();
}


function setUpMic() {
  const mic = app.streaming.mic;
  app.dom.micCheckbox.checked = mic.use;
  app.dom.micVolume.max = mic.volume.max;
  app.dom.micVolume.min = mic.volume.min;
  app.dom.micVolume.step = mic.volume.step;
  app.dom.micVolume.value = mic.volume.value;
  ncsoft.slider.adjustRange(app.dom.micVolume);
}


function setUpProviderUI(userPages, streamServers, userPage, privacy) {
  const userName = app.dom.providerUserName;
  const connectInfo = app.dom.connectInfoUserName;
  switch (app.service.provider) {
    case 'Facebook Live':
      app.dom.facebookDivision.style.display = 'block';
      app.dom.twitchDivision.style.display = 'none';
      ncsoft.klass.remove(userName, 'twitch');
      ncsoft.klass.add(userName, 'fb');
      ncsoft.klass.remove(connectInfo, 'twitch');
      ncsoft.klass.add(connectInfo, 'fb');
      setUpUserPage(userPages, userPage);
      setUpPrivacy(privacy);
      break;
    case 'Twitch':
      app.dom.facebookDivision.style.display = 'none';
      app.dom.twitchDivision.style.display = 'block';
      ncsoft.klass.remove(userName, 'fb');
      ncsoft.klass.add(userName, 'twitch');
      ncsoft.klass.remove(connectInfo, 'fb');
      ncsoft.klass.add(connectInfo, 'twitch');
      setUpStreamServers(streamServers);
      break;
    default:
      break;
  }
}


function setUpStreamServers(streamServers) {
  const serverSelect = app.dom.streamServerSelect;
  const display = serverSelect.children[0];
  const contents = serverSelect.children[1];
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }

  if (streamServers.length == 0) {
    return;
  }

  streamServers.sort(function(a, b) {
    const nameA = a.name.toUpperCase();
    const nameB = b.name.toUpperCase();
    if (nameA < nameB) {
      return -1;
    }
    if (nameA > nameB) {
      return 1;
    }

    return 0;
  });

  for (const server of streamServers) {
    const li = document.createElement('li');
    li.setAttribute('data-value', server.url);

    if (server.availability == 0.0) {
      li.innerHTML = '<span class="serverdown">' + server.name + '</span>';
    } else {
      const aTag = document.createElement('a');
      aTag.innerHTML = server.name;
      li.appendChild(aTag);
    }
    contents.appendChild(li);
  }
  display.value = contents.firstChild.getAttribute('data-value');
  display.innerHTML = contents.firstChild.firstChild.innerHTML +
                      '<span class="caret"></span>';
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
    return 'mePage select empty';
  }

  if (app.dom.mePageSelect.children[0].value == 1 &&
      app.dom.privacySelect.children[0].value == '') {
    return 'privacy select empty';
  }

  if (app.dom.mePageSelect.children[0].value == 2 &&
      app.dom.ownPageSelect.children[0].value == '') {
    return 'ownPage select empty';
  }

  if (app.dom.gameSelect.children[0].value == '') {
    return 'game select empty';
  }
  return '';
}


cef.serviceProviderLogIn.onResponse = function(
    userName, userPages, streamServers, userPage, privacy) {
  app.service.user = {
    name: userName,
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

  app.dom.connectInfoButton.style.display = 'inline';
  if (app.options.hidesSettings == false) {
    app.dom.settingButton.style.display = 'inline';
  }
  app.dom.minimizeButton.style.display = 'inline';

  setProviderUserName(userName);
  setUpProviderUI(userPages, streamServers, userPage, privacy);

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
    pages: {},
  };

  for (const element of app.dom.loginPagePanel) {
    element.style.display = 'block';
  }
  for (const element of app.dom.mainPagePanel) {
    element.style.display = 'none';
  }

  app.dom.connectInfoButton.style.display = 'none';
  app.dom.settingButton.style.display = 'none';
  app.dom.minimizeButton.style.display = 'none';

  setProviderUserName('');

  const ownPages = app.dom.ownPageSelect.children[1];
  while (ownPages.firstChild) {
    ownPages.removeChild(ownPages.firstChild);
  }
  ncsoft.select.disable(ownPageSelect);

  setUpUserPage('me');
  setUpPrivacy('SELF');
};


cef.streamingStart.onResponse =
    function(error, serviceProvider, streamUrl, postUrl) {
  console.info(error);
  if (error != '') {
    setUpError('fail streaming');
    app.streaming.startInfo = {};
    updateStreamingStatus('standby');
  } else {
    onMicCheckboxChanged();
    app.streaming.postUrl = postUrl;
    updateStreamingStatus('onAir');
  }

  (function notifyRemote() {
    const startInfo = app.streaming.startInfo;

    cef.remoteStart.request(
        remote.startRequestKey,
        error,
        startInfo.source,
        startInfo.userPage,
        startInfo.privacy,
        startInfo.description,
        app.dom.micCheckbox.checked,
        serviceProvider,
        streamUrl,
        postUrl);

    remote.startRequestKey = '';
  })();
};


cef.streamingStop.onResponse = function(error) {
  const source = app.streaming.startInfo.source;
  app.streaming.startInfo = {};
  updateStreamingStatus('standby');

  (function notifyRemote() {
    cef.remoteStop.request(remote.stopRequestKey, error, source);
    remote.stopRequestKey = '';
  })();
};


cef.settingsVideoQualityUpdate.onResponse = function(error) {
  (function notifyRemote() {
    cef.remoteQualityUpdate.request(remote.qualityUpdateRequestKey, error);
    remote.qualityUpdateRequestKey = '';
  })();
};


cef.settingsMicOn.onResponse = function(error, volume) {
  if (error != '') {
    console.info(error);
    return;
  }
  app.streaming.mic.use = true;
  app.streaming.mic.volume.value = volume;
};


cef.settingsMicOff.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
  console.info('start volume' + volume);
  app.streaming.mic.use = false;
};


cef.settingsMicVolumeUpdate.onResponse = function(error, volume) {
  if (error != '') {
    console.info(error);
    return;
  }
  app.streaming.mic.volume.value = volume;
};
