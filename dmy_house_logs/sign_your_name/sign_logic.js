function toggle_cntr(event, target_id)
{
  var target = document.getElementById(target_id);
  if (target.style.cssText == '') {
    target.style.cssText = 'display: none';
  } else {
    target.style.cssText = '';
  }
}

function init_cntrs()
{
  var cntrs = document.getElementsByClassName('cntr-system');

  var i = 0;
  for (i = 0; i < cntrs.length; i++) {
    var elmt = cntrs[i];
    var cntr_id = elmt.id;
    var button = document.getElementById('show-' + cntr_id);
    button.onclick = function (event) {
      toggle_cntr(event, this.cntr);
    };
    button.cntr = cntr_id;
    elmt.style.cssText = 'display: none';
  }

  /* Automatically fill in today's date in the custom date and time
     field if it is empty, but do not overwrite an existing value.
     Also, only set it if the user navigates into it, otherwise leave
     the field blank so that the default time of posting is used
     instead.  */
  document.getElementById('data-entry-date-time').onfocus = function (event) {
    if (this.value === '') {
      var today = new Date();
      var year = today.getYear() + 1900;
      var month = today.getMonth() + 1;
      var day = today.getDate();
      var str_year = ('0000' + year).slice(-4);
      var str_month = ('00' + month).slice(-2);
      var str_day = ('00' + day).slice(-2);
      this.value = '' + str_year + '-' + str_month + '-' + str_day + ' ';
    }
  };
}

function config_login()
{
  var sKey = 'dmy_log_name';
  /* https://developer.mozilla.org/en-US/docs/Web/API/Document/cookie/Simple_document.cookie_framework */
  var cookie = decodeURIComponent(document.cookie.replace(new RegExp("(?:(?:^|.*;)\\s*" + encodeURIComponent(sKey).replace(/[\-\.\+\*]/g, "\\$&") + "\\s*\\=\\s*([^;]*).*$)|^.*$"), "$1")) || null;
  var cntr_login = document.getElementById('cntr-login');
  if (cookie) {
    var name_el = document.getElementById('sign-' + cookie.toLowerCase());
    if (name_el) {
      name_el.checked = true;
      cntr_login.style.cssText = 'display: none';
    } else {
      cntr_login.style.cssText = '';
    }
  } else {
    cntr_login.style.cssText = '';
  }
}

init_cntrs();
config_login();
