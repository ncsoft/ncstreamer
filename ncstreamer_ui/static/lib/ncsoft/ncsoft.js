/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


const ncsoft = {
  klass: {},
  select: {},
  modal: {},
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


ncsoft.select.clear = function(element) {
  const display = element.children[0];
  const contents = element.children[1];

  display.value = '';
  while (contents.firstChild) {
    contents.removeChild(contents.firstChild);
  }
};


ncsoft.select.setByIndex = function(select, index) {
  const display = select.children[0];
  const contents = select.children[1];
  const child = contents.children[index];
  if (!child) {
    return false;
  }

  display.value = child.getAttribute('data-value');
  display.innerHTML = child.textContent +
                      '<span class="caret"></span>';
  return true;
};


ncsoft.select.setByValue = function(select, value) {
  const display = select.children[0];
  const contents = select.children[1];

  for (let index = 0; index < contents.children.length; ++index) {
    const content = contents.children[index];
    if (content.getAttribute('data-value') == value) {
      return ncsoft.select.setByIndex(select, index);
    }
  }
  return false;
};


ncsoft.select.setByText = function(select, text) {
  const display = select.children[0];
  const contents = select.children[1];

  for (let index = 0; index < contents.children.length; ++index) {
    const content = contents.children[index];
    if (content.textContent == text) {
      return ncsoft.select.setByIndex(select, index);
    }
  }
  return false;
};


ncsoft.select.getValue = function(select) {
  const display = select.children[0];
  return display.value;
};


ncsoft.select.getText = function(select) {
  const display = select.children[0];
  return display.textContent;
};


ncsoft.modal.show = function(elementId) {
  $(elementId).modal('show');
};
