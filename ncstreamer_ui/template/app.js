/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


'use strict';


const app = {
  dom: {},
  streaming: {
    // ['standby', 'setup', 'starting', 'onAir', 'stopping', 'closing']
    status: 'standby',
    lastStatus: null,
    shutdown: false,
    startInfo: {},
    popupBrowserId: 0,
    postUrl: null,
    nctvUrl: null,
    mic: {
      use: false,
      list: null,
      curDeviceId: null,
      volume: {
        max: 1,
        min: 0,
        step: 0.1,
        value: 0.5,
      }
    },
    webcam: {
      use: false,
      list: null,
      curDeviceId: null,
      size: {
        width: 0.25,
        height: 0.25,
      },
      position: {
        x: 0,
        y: 0,
      },
      chromaKey: {
        use: false,
        color: 0x00FF00,  // 0x00BGR
        similarity: 400,
      },
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
    settingsPage: null,
    authorizationUrlForYouTube: 'https://www.youtube.com/signin?' +
        'next=/live_streaming_signup&app=desktop&' +
        'action_prompt_identity=true',
    youtubeSupportUrl: 'https://support.google.com/youtube/answer/2474026',
    youtubeSettingsUrlTemplate: 'https://www.youtube.com/my_live_events?' +
        'editor_tab=advanced&action_edit_live_event=1&event_id=',
    videoId: null,
  },
  options: {
    hidesSettings: false,
  },
  errorType: null,
  CountryCode: {
    KR: 'Korea',
    US: 'US',
    TW: 'Taiwan',
    JP: 'Japan',
  },
};


document.addEventListener('contextmenu', function(event) {
  event.preventDefault();
});


document.addEventListener('DOMContentLoaded', function(event) {
  ncsoft.onDOMContentLoaded();
});


document.addEventListener('keydown', disableF4);


function disableF4(event) {
  if (event.keyCode != 115) {  // 115 == F4
    return;
  }
  event.returnValue = false;
}


function catchAltF4(event) {
  if (!event.altKey || event.keyCode != 115) {  // 115 == F4
    return;
  }
  onCloseButtonClicked();
  event.returnValue = false;
}


function catchEsc(event) {
  if (event.keyCode != 27) {  // 27 == Esc
    return;
  }
  onCloseButtonClicked();
  event.returnValue = false;
}


function toCamel(str) {
  return str.replace(/(\-[a-z])/g, function(match) {
    return match.toUpperCase().replace('-', '');
  });
}


function enableAllContorls() {
  ncsoft.select.enable(app.dom.youtubePrivacySelect);
  ncsoft.select.enable(app.dom.mePageSelect);
  ncsoft.select.enable(app.dom.ownPageSelect);
  ncsoft.select.enable(app.dom.privacySelect);
  ncsoft.heading.enable(app.dom.streamServerHead);
  ncsoft.select.enable(app.dom.streamServerSelect);
  ncsoft.heading.enable(app.dom.broadcastingGameHead);
  ncsoft.select.enable(app.dom.gameSelect);
  ncsoft.heading.enable(app.dom.broadcastingTitleHead);
  ncsoft.textarea.enable(app.dom.feedDescription);
  ncsoft.checkbox.enable(app.dom.webcamCheckbox);
  ncsoft.checkbox.enable(app.dom.micCheckbox);
}


function disableAllControls() {
  ncsoft.select.disable(app.dom.youtubePrivacySelect);
  ncsoft.select.disable(app.dom.mePageSelect);
  ncsoft.select.disable(app.dom.ownPageSelect);
  ncsoft.select.disable(app.dom.privacySelect);
  ncsoft.heading.disable(app.dom.streamServerHead);
  ncsoft.select.disable(app.dom.streamServerSelect);
  ncsoft.heading.disable(app.dom.broadcastingGameHead);
  ncsoft.select.disable(app.dom.gameSelect);
  ncsoft.heading.disable(app.dom.broadcastingTitleHead);
  ncsoft.textarea.disable(app.dom.feedDescription);
  ncsoft.checkbox.disable(app.dom.webcamCheckbox);
  ncsoft.checkbox.disable(app.dom.micCheckbox);
}


function updateStreamingStatus(status) {
  console.info(JSON.stringify({status: status}));

  app.streaming.status = status;
  app.dom.cautionText.style.display = 'none';
  app.dom.liveImage.style.display = 'none';
  app.dom.nctvText.style.display = 'none';
  const button = app.dom.controlButton;
  switch (status) {
    case 'standby':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%START_BROADCASTING%';
      button.disabled = false;
      ncsoft.klass.remove(app.dom.providerPageLink, 'live');
      enableAllContorls();
      break;
    case 'setup':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%PREPARING_BROADCASTING%';
      button.disabled = true;
      app.dom.cautionText.style.display = 'block';
      disableAllControls();
      break;
    case 'starting':
      ncsoft.klass.add(button, 'loading');
      button.textContent = '%PREPARING_BROADCASTING%';
      button.disabled = true;
      app.dom.cautionText.style.display = 'block';
      disableAllControls();
      break;
    case 'onAir':
      ncsoft.klass.remove(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      button.disabled = false;
      app.dom.liveImage.style.display = 'block';
      ncsoft.klass.add(app.dom.providerPageLink, 'live');
      disableAllControls();
      ncsoft.checkbox.enable(app.dom.webcamCheckbox);
      ncsoft.checkbox.enable(app.dom.micCheckbox);
      if (app.service.provider == 'YouTube' &&
          '%YOUTUBE_GUIDE_MODAL_VISIBILITY%' == '_1') {
        popupYouTubeSettings();
      }
      break;
    case 'stopping':
      ncsoft.klass.add(button, 'loading');
      button.textContent = '%END_BROADCASTING%';
      disableAllControls();
      button.disabled = true;
      break;
    case 'closing':
      disableAllControls();
      button.disabled = true;
      break;
  }
}


function setUpControls(args) {
  [
    'nc-streamer-container',
    'login-page-panel',
    'main-page-panel',
    'nc-streamer-modal',
  ].forEach(function(domCls) {
    app.dom[toCamel(domCls)] = document.getElementsByClassName(domCls);
  });

  [
    'connect-info-button',
    'setting-button',
    'minimize-button',
    'close-button',
    'login-button-youtube',
    'login-button-facebook',
    'login-button-twitch',
    'provider-user-name',
    'connect-info-user-name',
    'connect-info-disconnect-button',
    'connect-info-msg',
    'connect-info-confirm-button',
    'provider-page-link',
    'youtube-division',
    'youtube-privacy-select',
    'facebook-division',
    'me-page-select',
    'own-page-select',
    'privacy-select',
    'twitch-division',
    'stream-server-head',
    'stream-server-select',
    'broadcasting-game-head',
    'game-select',
    'broadcasting-title-head',
    'feed-description',
    'mic-checkbox',
    'mic-volume',
    'webcam-checkbox',
    'error-text',
    'caution-text',
    'live-streaming-img',
    'nctv-text',
    'nctv-link',
    'live-image',
    'control-button',
    'quality-select',
    'chroma-key-checkbox',
    'modal-close-button',
    'twitch-settings-popup',
    'youtube-settings-popup',
    'popup-hide-button',
    'youtube-hide-button',
    'youtube-support-link-button',
    'youtube-link-button',
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
  app.dom.loginButtonYoutube.addEventListener(
      'click', onLoginButtonYoutubeClicked);
  app.dom.loginButtonFacebook.addEventListener(
      'click', onLoginButtonFacebookClicked);
  app.dom.loginButtonTwitch.addEventListener(
      'click', onLoginButtonTwitchClicked);
  app.dom.connectInfoDisconnectButton.addEventListener(
      'click', onConnectInfoDisconnectButtonClicked);
  app.dom.providerPageLink.addEventListener(
      'click', onProviderPageLinkClicked);
  app.dom.youtubePrivacySelect.addEventListener(
      'ncsoftSelectChange', onYoutubePrivacySelectChanged);
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
  app.dom.webcamCheckbox.addEventListener(
      'change', onWebcamCheckboxChanged);
  app.dom.controlButton.addEventListener(
      'click', onControlButtonClicked);
  app.dom.qualitySelect.addEventListener(
      'ncsoftSelectChange', onQualitySelectChanged);
  app.dom.chromaKeyCheckbox.addEventListener(
      'change', onchromaKeyCheckboxChanged);
  app.dom.modalCloseButton.addEventListener(
      'click', onModalCloseClicked);
  app.dom.twitchSettingsPopup.addEventListener(
      'click', onTwitchSettingsPopupClicked);
  app.dom.youtubeSettingsPopup.addEventListener(
      'click', onYoutubeSettingsPopupClicked);
  app.dom.popupHideButton.addEventListener(
      'click', onPopupHideButtonClicked);
  app.dom.youtubeHideButton.addEventListener(
      'click', onYoutubeHideButtonClicked);
  app.dom.youtubeSupportLinkButton.addEventListener(
      'click', onYoutubeSupportLinkButtonClicked);
  app.dom.youtubeLinkButton.addEventListener(
      'click', onYoutubeLinkClicked);
  app.dom.nctvLink.addEventListener(
      'click', onNctvLinkClicked);

  ncsoft.select.disable(app.dom.privacySelect);
  ncsoft.select.setText(app.dom.privacySelect, '%POST_PRIVACY_BOUND%');
  ncsoft.select.disable(app.dom.gameSelect);
  ncsoft.select.setText(app.dom.gameSelect, '%NO_PLAYING_GAME%');

  app.options.hidesSettings = (args.hidesSettings == 'true');
  if (args.deviceSettings.hasOwnProperty('webcam')) {
    setUpWebcam(args.deviceSettings.webcam);
  }
  if (args.deviceSettings.hasOwnProperty('mic') &&
      args.deviceSettings.mic.hasOwnProperty('use')) {
    setUpMic(args.deviceSettings.mic.use == 'true' ? true : false);
  } else {
    setUpMic(false);
  }
  setUpSteamingQuality();
  ncsoft.select.setByValue(app.dom.qualitySelect, args.videoQuality);
  cef.streamingSetUp.request();
}


function setProviderUserName(userName) {
  app.dom.providerUserName.textContent = userName;
  app.dom.connectInfoUserName.textContent = userName;
}


function getCurrentUserPage() {
  return (app.dom.mePageSelect.children[0].value == 2) ?
      app.dom.ownPageSelect.children[0].value : 'me';
}


function setUserSettingPage(userPages) {
  if (userPages.length == 0) {
    return;
  }
  app.service.settingsPage = userPages[0].link;
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
    ncsoft.select.setText(ownPageSelect, '%NO_MANAGING_PAGE%');
  } else {
    ncsoft.select.enable(ownPageSelect);
    for (const ownPage of userPages) {
      const li = document.createElement('li');
      const aTag = document.createElement('a');
      aTag.setAttribute('href', '#');
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
  const streamingStop = stopInvalidSource(sources);

  if (app.streaming.status != 'standby' &&
      streamingStop == false) {
    return;
  }

  const gameSelect = app.dom.gameSelect;
  ncsoft.select.clear(gameSelect);

  if (sources.length == 0) {
    ncsoft.select.disable(gameSelect);
    ncsoft.select.setText(gameSelect, '%NO_PLAYING_GAME%');
  } else {
    ncsoft.select.enable(gameSelect);

    const display = gameSelect.children[0];
    const contents = gameSelect.children[1];

    for (const source of sources) {
      const li = document.createElement('li');
      const aTag = document.createElement('a');
      aTag.textContent = getTitleFromSource(source);
      aTag.setAttribute('href', '#');
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
    return false;
  }

  const currentSource = app.streaming.startInfo.source;
  if (sources.includes(currentSource) == true) {
    return false;
  }

  console.info('stop invalid source: ' + currentSource);
  app.dom.controlButton.click();
  return true;
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


function onLoginButtonYoutubeClicked() {
  console.info('click loginButtonYoutube');
  app.service.provider = 'YouTube';
  cef.serviceProviderLogIn.request(app.service.provider);
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


function onYoutubePrivacySelectChanged() {
  console.info('change youtubePrivacySelect');
  const privacy = app.dom.youtubePrivacySelect.children[0].value;
  cef.storageYoutubePrivacyUpdate.request(privacy);
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
  if (app.errorType == 'select down server') {
    app.dom.errorText.style.display = 'none';
  }

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
    ncsoft.checkbox.disable(app.dom.micCheckbox);
    app.streaming.mic.use = true;
    cef.settingsMicSearch.request();
  } else {
    console.info('mic off');
    cef.settingsMicOff.request();
  }
}


function onWebcamCheckboxChanged() {
  console.info('change webcamCheckbox');
  if (app.dom.webcamCheckbox.checked) {
    console.info('webcam on');
    ncsoft.checkbox.disable(app.dom.webcamCheckbox);
    app.streaming.webcam.use = true;
    cef.settingsWebcamSearch.request();
  } else {
    console.info('webcam off');
    cef.settingsWebcamOff.request();
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
      const streamServer = app.dom.streamServerSelect.children[0].value;
      const description = app.dom.feedDescription.value;
      var privacy;
      if (app.service.provider == 'YouTube') {
        privacy = app.dom.youtubePrivacySelect.children[0].value;
      } else {
        privacy = app.dom.mePageSelect.children[0].value == 2 ?
            'EVERYONE' : app.dom.privacySelect.children[0].value;
      }

      cef.streamingStart.request(source,
                                 streamServer,
                                 userPage,
                                 privacy,
                                 '' /* title */,
                                 description);
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
  cef.serviceProviderLogOut.request(app.service.provider);
}


function onQualitySelectChanged() {
  updateQualitySelect();
}


function onchromaKeyCheckboxChanged() {
  if (app.dom.chromaKeyCheckbox.checked) {
    const color = app.streaming.webcam.chromaKey.color;
    const similarity = app.streaming.webcam.chromaKey.similarity;
    cef.settingsChromaKeyOn.request(color, similarity);
  } else {
    cef.settingsChromaKeyOff.request();
  }
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
  app.streaming.shutdown = true;
  onControlButtonClicked();
}


function onPopupHideButtonClicked() {
  console.info('click popupHideButton');
  ncsoft.storage.add('twitchSettingHide', 30 /*day*/);
}


function onYoutubeHideButtonClicked() {
  console.info('click youtubeHideButton');
  ncsoft.storage.add('youtubeSettingHide', 30 /*day*/);
}


function onTwitchSettingsPopupClicked() {
  console.info('click twitchSettingsPopup');
  cef.externalBrowserPopUp.request(app.service.settingsPage);
}

function onYoutubeSettingsPopupClicked() {
  console.info('click youtubeSettingsPopup');
  cef.externalBrowserPopUp.request(
      app.service.youtubeSettingsUrlTemplate + app.service.videoId);
}


function onYoutubeSupportLinkButtonClicked() {
  console.info('click youtubeSupportLink');
  cef.externalBrowserPopUp.request(app.service.youtubeSupportUrl);
}


function onYoutubeLinkClicked() {
  console.info('click youtubeLink');
  cef.externalBrowserPopUp.request(app.service.authorizationUrlForYouTube);
}


function onNctvLinkClicked() {
  console.info('click nctvLink');

  if (app.streaming.status != 'onAir') {
    return;
  }

  cef.externalBrowserPopUp.request(app.streaming.nctvUrl);
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
    aTag.setAttribute('href', '#');
    li.setAttribute('data-value', level);
    li.appendChild(aTag);
    contents.appendChild(li);
  }
  display.value = contents.firstChild.getAttribute('data-value');
  display.innerHTML = contents.firstChild.firstChild.textContent +
                      '<span class="caret"></span>';
}


function setUpWebcam(webcamSettings) {
  const webcam = app.streaming.webcam;
  if (webcamSettings.hasOwnProperty('use')) {
    app.dom.webcamCheckbox.checked =
        webcamSettings.use == 'true' ? true : false;
  }
  if (webcamSettings.hasOwnProperty('deviceId')) {
    webcam.curDeviceId = webcamSettings.deviceId;
  }
  if (webcamSettings.hasOwnProperty('size')) {
    webcam.size.width = webcamSettings.size.width;
    webcam.size.height = webcamSettings.size.height;
  }
  if (webcamSettings.hasOwnProperty('position')) {
    webcam.position.x = webcamSettings.position.x;
    webcam.position.y = webcamSettings.position.y;
  }
  if (webcamSettings.hasOwnProperty('chromaKey')) {
    const chromaKeySettings = webcamSettings.chromaKey;
    if (chromaKeySettings.hasOwnProperty('use')) {
      app.dom.chromaKeyCheckbox.checked =
          chromaKey.use == 'true' ? true : false;
    }
    if (chromaKeySettings.hasOwnProperty('color')) {
      webcam.chromaKey.color = chromaKeySettings.color;
    }
    if (chromaKeySettings.hasOwnProperty('similarity')) {
      webcam.chromaKey.similarity = chromaKeySettings.similarity;
    }
  }
}


function setUpMic(check) {
  const mic = app.streaming.mic;
  app.dom.micVolume.max = mic.volume.max;
  app.dom.micVolume.min = mic.volume.min;
  app.dom.micVolume.step = mic.volume.step;
  app.dom.micVolume.value = mic.volume.value;
  if (check == true) {
    ncsoft.slider.adjustRange(app.dom.micVolume);
  }
  setMicCheckBox(check);
}


function setMicCheckBox(check) {
  app.dom.micCheckbox.checked = check;
  if (check == true) {
    ncsoft.slider.enable(app.dom.micVolume);
  } else {
    ncsoft.slider.disable(app.dom.micVolume);
  }
}


function setUpProviderUI(
    userPages,
    streamServers,
    userPage,
    privacy,
    youtubePrivacy,
    streamServer,
    location) {
  const userName = app.dom.providerUserName;
  const connectInfo = app.dom.connectInfoUserName;
  switch (app.service.provider) {
    case 'YouTube':
      app.dom.youtubeDivision.style.display = 'block';
      app.dom.facebookDivision.style.display = 'none';
      app.dom.twitchDivision.style.display = 'none';
      ncsoft.klass.remove(userName, 'fb');
      ncsoft.klass.remove(userName, 'twitch');
      ncsoft.klass.add(userName, 'youtube');
      ncsoft.klass.remove(connectInfo, 'fb');
      ncsoft.klass.remove(connectInfo, 'twitch');
      ncsoft.klass.add(connectInfo, 'youtube');
      ncsoft.select.setByValue(app.dom.youtubePrivacySelect, youtubePrivacy);
      break;
    case 'Facebook Live':
      app.dom.youtubeDivision.style.display = 'none';
      app.dom.facebookDivision.style.display = 'block';
      app.dom.twitchDivision.style.display = 'none';
      ncsoft.klass.remove(userName, 'youtube');
      ncsoft.klass.remove(userName, 'twitch');
      ncsoft.klass.add(userName, 'fb');
      ncsoft.klass.remove(connectInfo, 'youtube');
      ncsoft.klass.remove(connectInfo, 'twitch');
      ncsoft.klass.add(connectInfo, 'fb');
      setUpUserPage(userPages, userPage);
      setUpPrivacy(privacy);
      break;
    case 'Twitch':
      app.dom.youtubeDivision.style.display = 'none';
      app.dom.facebookDivision.style.display = 'none';
      app.dom.twitchDivision.style.display = 'block';
      ncsoft.klass.remove(userName, 'youtube');
      ncsoft.klass.remove(userName, 'fb');
      ncsoft.klass.add(userName, 'twitch');
      ncsoft.klass.remove(connectInfo, 'youtube');
      ncsoft.klass.remove(connectInfo, 'fb');
      ncsoft.klass.add(connectInfo, 'twitch');
      setUpStreamServers(streamServers, streamServer, location);
      setUserSettingPage(userPages);
      popupTwitchSettings();
      break;
    default:
      break;
  }
}


function popupYouTubeSettings() {
  const saveDate = ncsoft.storage.get('youtubeSettingHide');
  const curDate = new Date();
  if (saveDate != null && curDate.getTime() < saveDate) {
    return;
  }
  ncsoft.modal.show('#youtube-guide-modal');
}


function popupTwitchSettings() {
  const saveDate = ncsoft.storage.get('twitchSettingHide');
  const curDate = new Date();
  if (saveDate != null && curDate.getTime() < saveDate) {
    return;
  }
  ncsoft.modal.show('#twitch-guide-modal');
}


function setUpStreamServers(streamServers, streamServer, location) {
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
    if (server.name.indexOf('deprecated') != -1 ||
        server.name.indexOf('depracated') != -1) {
      continue;
    }

    const li = document.createElement('li');
    li.setAttribute('data-value', server.url);

    if (server.availability == 0.0) {
      li.innerHTML = '<span class="serverdown">' + server.name + '</span>';
    } else {
      const aTag = document.createElement('a');
      aTag.innerHTML = server.name;
      aTag.setAttribute('href', '#');
      li.appendChild(aTag);
    }
    contents.appendChild(li);
  }

  const defaultServer = (function() {
    var content = app.dom.streamServerSelect.children[1].firstChild;
    const curLocation = location == '' ? 'KR' : location;
    while (content) {
      if (content.innerHTML.indexOf(app.CountryCode[curLocation]) != -1) {
        return content;
      }
      content = content.nextSibling;
    }
    return contents.firstChild;
  })();

  display.value = defaultServer.getAttribute('data-value');
  display.innerHTML = defaultServer.firstChild.innerHTML +
                      '<span class="caret"></span>';

  if (streamServer != '') {
    ncsoft.select.setByValue(serverSelect, streamServer);
  }
}


function setUpError(type) {
  app.errorType = type;
  showErrorText();
}


function showErrorText() {
  const error = app.dom.errorText;
  switch (app.errorType) {
    case 'fail streaming':
      if (app.service.provider == 'Twitch') {
        error.textContent = '%TWITCH_ERROR%';
      } else if (app.service.provider == 'YouTube') {
        error.textContent = '%YOUTUBE_ERROR%';
      } else {
        error.textContent = '%FACEBOOK_ERROR%';
      }
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
    case 'select down server':
      error.textContent = '%SELECT_DOWN_SERVER%';
      break;
    case 'title empty':
      error.textContent = '%EMPTY_TITLE%';
      break;
    case 'obs error':
      error.textContent = '%OBS_ERROR%';
      break;
    case 'obs timeout':
      error.textContent = '%OBS_TIMEOUT%';
      break;
    default:
      error.TextContent = '%OBS_ERROR%';
      break;
  }
  error.style.display = 'block';
}


function checkSelectValueValidation() {
  if (app.service.provider == 'Twitch') {
    const serverUrl = app.dom.streamServerSelect.children[0].value;
    const server = app.service.user.streamServer[serverUrl];
    if (server.availability == 0.0) {
      return 'select down server';
    }
  } else if (app.service.provider == 'YouTube') {
    if (app.dom.feedDescription.value == '') {
      return 'title empty';
    }
  } else {  // app.service.provider == facebook
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
  }

  if (app.dom.gameSelect.children[0].value == '') {
    return 'game select empty';
  }
  return '';
}


function checkCurrentMicExist() {
  for (const mic of app.streaming.mic.list) {
    if (app.streaming.mic.curDeviceId == mic.id) {
      return true;
    }
  }
  return false;
}


function checkCurrentWebcamExist() {
  for (const webcam of app.streaming.webcam.list) {
    if (app.streaming.webcam.curDeviceId == webcam.id) {
      return true;
    }
  }
  return false;
}


cef.serviceProviderLogIn.onResponse = function(
    error, _id, accessToken, userName, userPages, streamServers, userPage,
    privacy, youtubePrivacy, streamServer, description, location) {
  if (error != '') {
    console.info(error);
    if (!error.includes('no channel or streaming service') &&
        !error.includes('could not get channel from')) {
      return;
    }
    ncsoft.modal.show('#youtube-link-modal');
    return;
  }

  app.service.user = {
    id: _id,
    token: accessToken,
    name: userName,
    pages: {},
    streamServer: {},
  };
  for (const userPage of userPages) {
    app.service.user.pages[userPage.id] = userPage;
  }

  for (const server of streamServers) {
    app.service.user.streamServer[server.url] = server;
  }

  app.dom.feedDescription.value = description;

  for (const element of app.dom.loginPagePanel) {
    element.style.display = 'none';
  }
  for (const element of app.dom.mainPagePanel) {
    element.style.display = 'block';
  }

  app.dom.connectInfoButton.style.display = 'inline';

  setProviderUserName(userName);
  setUpProviderUI(
      userPages,
      streamServers,
      userPage,
      privacy,
      youtubePrivacy,
      streamServer,
      location);

  app.dom.errorText.style.display = 'none';
};


cef.serviceProviderLogOut.onResponse = function(error) {
  if (error) {
    // TODO(khpark): TBD
    alert('Logout failed: ' + error);
    return;
  }

  app.service.user = {
    id: '',
    token: '',
    name: '',
    pages: {},
    streamServer: {},
  };

  for (const element of app.dom.loginPagePanel) {
    element.style.display = 'block';
  }
  for (const element of app.dom.mainPagePanel) {
    element.style.display = 'none';
  }

  app.dom.connectInfoButton.style.display = 'none';

  setProviderUserName('');

  const ownPages = app.dom.ownPageSelect.children[1];
  while (ownPages.firstChild) {
    ownPages.removeChild(ownPages.firstChild);
  }
  ncsoft.select.disable(ownPageSelect);

  setUpUserPage('me');
  setUpPrivacy('SELF');
  ncsoft.select.setByValue(app.dom.youtubePrivacySelect, 'public');
};


cef.streamingSetUp.onResponse = function(error) {
  updateQualitySelect();
  app.dom.closeButton.style.display = 'inline';
  if (app.options.hidesSettings == false) {
    app.dom.settingButton.style.display = 'inline';
  }
  app.dom.minimizeButton.style.display = 'inline';
  for (const element of app.dom.ncStreamerContainer) {
    ncsoft.klass.remove(element, 'loading');
  }

  document.removeEventListener('keydown', disableF4);
  document.addEventListener('keydown', catchAltF4);
  document.addEventListener('keydown', catchEsc);
  for (const item of app.dom.ncStreamerModal) {
    item.addEventListener('keydown', function(event) {
      if (event.keyCode != 27) {  // 27 == Esc
        return;
      }
      console.info('close modal');
      event.stopPropagation();
    });
  }
};


cef.streamingStart.onResponse =
    function(error, serviceProvider, streamUrl, videoId, postUrl) {
  console.info(error);
  if (error != '') {
    if (error == 'obs internal') {
      setUpError('obs error');
    } else if (error.includes('no channel or streaming service')) {
      ncsoft.modal.show('#youtube-link-modal');
    } else if (error == 'obs timeout') {
      setUpError('obs timeout');
    } else {
      setUpError('fail streaming');
    }
    app.streaming.startInfo = {};
    updateStreamingStatus('standby');
  } else {
    onMicCheckboxChanged();
    if (app.dom.webcamCheckbox.checked) {
      const deviceId = app.streaming.webcam.curDeviceId;
      const width = app.streaming.webcam.size.width;
      const height = app.streaming.webcam.size.height;
      const x = app.streaming.webcam.position.x;
      const y = app.streaming.webcam.position.y;
      cef.settingsWebcamOn.request(deviceId, width, height, x, y);
    }
    app.streaming.postUrl = postUrl;
    app.service.videoId = videoId;
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
        postUrl,
        app.service.user.id,
        videoId,
        app.service.user.token);

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

  if (app.streaming.shutdown == true) {
    cef.windowClose.request();
  }
};


cef.settingsVideoQualityUpdate.onResponse = function(error) {
  (function notifyRemote() {
    cef.remoteQualityUpdate.request(remote.qualityUpdateRequestKey, error);
    remote.qualityUpdateRequestKey = '';
  })();
};


cef.settingsMicSearch.onResponse = function(error, micList) {
  if (error != '') {
    console.info(error);
    ncsoft.checkbox.enable(app.dom.micCheckbox);
    return;
  }
  if (micList == '') {
    console.info('no devices');
    if (app.streaming.mic.use) {
      ncsoft.modal.show('#no-device-alert-modal');
      app.streaming.mic.use = false;
    }
    app.dom.micCheckbox.checked = false;
    ncsoft.checkbox.enable(app.dom.micCheckbox);
    return;
  }

  app.streaming.mic.list = micList;
  if (checkCurrentMicExist() == false) {
    app.streaming.mic.curDeviceId = 'default';
  }

  const deviceId = app.streaming.mic.curDeviceId;
  const volume = app.dom.micVolume.value;
  cef.settingsMicOn.request(deviceId, volume);
};


cef.settingsMicOn.onResponse = function(error, volume) {
  ncsoft.checkbox.enable(app.dom.micCheckbox);
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
  app.streaming.mic.use = false;
};


cef.settingsMicVolumeUpdate.onResponse = function(error, volume) {
  if (error != '') {
    console.info(error);
    return;
  }
  app.streaming.mic.volume.value = volume;
};


cef.settingsWebcamSearch.onResponse = function(error, webcamList) {
  if (error != '') {
    console.info(error);
    ncsoft.checkbox.enable(app.dom.webcamCheckbox);
    return;
  }
  if (webcamList == '') {
    console.info('no devices');
    if (app.streaming.webcam.use) {
      ncsoft.modal.show('#no-device-alert-modal');
      app.streaming.webcam.use = false;
    }
    app.dom.webcamCheckbox.checked = false;
    ncsoft.checkbox.enable(app.dom.webcamCheckbox);
    return;
  }

  app.streaming.webcam.list = webcamList;
  if (checkCurrentWebcamExist() == false) {
    app.streaming.webcam.curDeviceId = app.streaming.webcam.list[0].id;
  }
  const deviceId = app.streaming.webcam.curDeviceId;
  const width = app.streaming.webcam.size.width;
  const height = app.streaming.webcam.size.height;
  const x = app.streaming.webcam.position.x;
  const y = app.streaming.webcam.position.y;
  cef.settingsWebcamOn.request(deviceId, width, height, x, y);
};


cef.settingsWebcamOn.onResponse = function(error) {
  ncsoft.checkbox.enable(app.dom.webcamCheckbox);

  if (error != '') {
    console.info(error);
    if (error != 'webcam size error' ||
        error != 'webcam position error') {
      return;
    }
  }
  app.streaming.webcam.use = true;
  if (app.dom.chromaKeyCheckbox.checked) {
    const color = app.streaming.webcam.chromaKey.color;
    const similarity = app.streaming.webcam.chromaKey.similarity;
    cef.settingsChromaKeyOn.request(color, similarity);
  }
};


cef.settingsWebcamOff.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
  app.streaming.webcam.use = false;
};


cef.settingsWebcamSizeUpdate.onResponse =
    function(error, normalWidth, normalHeight) {
  if (error != '') {
    console.info(error);
    return;
  }
};


cef.settingsWebcamPositionUpdate.onResponse =
    function(error, normalX, normalY) {
  if (error != '') {
    console.info(error);
    return;
  }
};


cef.settingsChromaKeyOn.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
  app.streaming.webcam.chromaKey.use = true;
};


cef.settingsChromaKeyOff.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
  app.streaming.webcam.chromaKey.use = false;
};


cef.settingsChromaKeyColorUpdate.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
};


cef.settingsChromaKeySimilarityUpdate.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
};


cef.remoteStop.onResponse = function(error) {
  if (error != '') {
    console.info(error);
    return;
  }
  if (app.streaming.status == 'closing') {
    cef.windowClose.request();
  }
};
