/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


const ncsoft = {
  klass: {},
  select: {},
};


ncsoft.onDOMContentLoaded = function() {
  var event = new CustomEvent('ncsoftSelectChange');

  // custom select
  $(document).on('click', '.dropdown-menu li a', function(e) {
    var selText = $(this).text();
    $(this).parents('.dropdown').find('.dropdown-toggle').html(
        selText + ' <span class="caret"></span>');
    $(this).parents('.dropdown').find('.dropdown-toggle').val(
        $(this).parent().attr('data-value'));
    document.querySelector(
        '#' + $(this).parents('.dropdown').attr('id')
        ).dispatchEvent(event);
  });

  // tooltip
  $(document).on('click', '.nc-streamer-tooltip .layer-close', function(e) {
    $('.nc-streamer-tooltip').hide();
  });
};


ncsoft.klass.add = function(element, name) {
  element.className += ' ' + name;
};


ncsoft.klass.remove = function(element, name) {
  const check = new RegExp('(\\s|^)' + name + '(\\s|$)');
  element.className = element.className.replace(check, ' ').trim();
};


ncsoft.select.enable = function(element) {
  element.children[0].style.display = 'block';
  element.children[2].style.display = 'none';
};


ncsoft.select.disable = function(element) {
  element.children[0].style.display = 'none';
  element.children[2].style.display = 'block';
};


ncsoft.select.setByIndex = function(select, index) {
  const display = select.children[0];
  const contents = select.children[1];
  const child = contents.children[index];

  display.value = child.getAttribute('data-value');
  display.innerHTML = child.textContent +
                      '<span class="caret"></span>';
};


ncsoft.select.setByValue = function(select, value) {
  const display = select.children[0];
  const contents = select.children[1];

  for (let index = 0; index < contents.children.length; ++index) {
    const content = contents.children[index];
    if (content.getAttribute('data-value') == value) {
      ncsoft.select.setByIndex(select, index);
      return;
    }
  }
};


ncsoft.select.setByText = function(select, text) {
  const display = select.children[0];
  const contents = select.children[1];

  for (let index = 0; index < contents.children.length; ++index) {
    const content = contents.children[index];
    if (content.textContent == text) {
      ncsoft.select.setByIndex(select, index);
      return;
    }
  }
};
