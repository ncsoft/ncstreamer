/**
 * Copyright (C) 2017 NCSOFT Corporation
 */


const ncsoft = {
  klass: {},
  select: {},
  textarea: {},
  heading: {},
  modal: {},
  slider: {},
};


ncsoft.onDOMContentLoaded = function() {
  var event = new CustomEvent('ncsoftSelectChange');

  // custom select
  $(document).on('click', '.dropdown-menu li a', function(e) {
    var selText = $(this).html();
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

  // range slider
  var _range = $('.form-checkbox.mic input[type="range"]');
  adjustRange(_range);

  $(document).on('click', '.form-checkbox.mic #mic-checkbox', function() {
    adjustRange(_range);
    if ($(this).is(':checked') !== true) {
      $('.range-slider-range').attr(
          'value', _range.value).removeAttr(
          'style').attr('disabled', 'disabled');
    }
    else {
      $('.range-slider-range').attr(
          'value', _range.value).removeAttr('disabled');
    }
  });

  $(document).on(
      'mousemove', '.form-checkbox.mic input[type="range"]', function() {
        adjustRange(_range);
        document.querySelector(
            '#' + $(this).attr('id')
        ).dispatchEvent(event);
      });

  function adjustRange(el) {
    var val = (el.val() - el.attr('min')) / (el.attr('max') - el.attr('min'));
    el.attr('value', this.value);
    el.css('background-image',
           '-webkit-gradient(linear, left top, right top,' +
           'color-stop(' + val + ', #2f76ad),' +
           'color-stop(' + val + ', #22252f)' +
           ')'
    );
  }
};


ncsoft.slider.adjustRange = function(element) {
  const range = $('#' + element.getAttribute('id'));
  const val = (range.val() - range.attr('min')) /
              (range.attr('max') - range.attr('min'));
  range.attr('value', range.value);
  range.css('background-image',
            '-webkit-gradient(linear, left top, right top,' +
            'color-stop(' + val + ', #2f76ad),' +
            'color-stop(' + val + ', #22252f)' +
            ')'
  );
};


ncsoft.klass.add = function(element, name) {
  element.className += ' ' + name;
};


ncsoft.klass.remove = function(element, name) {
  const check = new RegExp('(\\s|^)' + name + '(\\s|$)');
  element.className = element.className.replace(check, ' ').trim();
};


ncsoft.select.enable = function(element) {
  element.children[0].removeAttribute('disabled');
};


ncsoft.select.disable = function(element) {
  element.children[0].setAttribute('disabled', '');
};


ncsoft.heading.enable = function(element) {
  ncsoft.klass.remove(element, 'disabled');
};


ncsoft.heading.disable = function(element) {
  ncsoft.klass.add(element, 'disabled');
};


ncsoft.textarea.enable = function(element) {
  element.removeAttribute('disabled');
};


ncsoft.textarea.disable = function(element) {
  element.setAttribute('disabled', '');
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
  display.innerHTML = child.innerHTML +
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


ncsoft.select.setText = function(select, text) {
  const display = select.children[0];
  display.innerHTML = text + '<span class="caret"></span>';
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
