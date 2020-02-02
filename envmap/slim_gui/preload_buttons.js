/*

@licstart  The following is the entire license notice for the
JavaScript code in this page.

Copyright (C) 2014 University of Minnesota

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@licend  The above is the entire license notice for the JavaScript
code in this page.

*/

/* GUI Event Handling and Management */

// Preload the button images.
function loadImages() {
  var files = [ 'outer_light.png', 'layer_down_light.png',
    'layer_up_light.png', 'angle_prev_light.png',
    'angle_next_light.png', 'linked_data_light.png',
    'raw_scan_light.png', 'dig_positive_light.png',
    'text_view_light.png', 'vector_view_light.png',
    'overlays_light.png', 'thumbnails_light.png',
    '3d_view_light.png', 'step_back_light.png',
    'step_fwd_light.png', 'minimize_light.png' ];

  var i = 0;
  for (i = 0; i < files.length; i++) {
    btnImages[i] = new Image(); btnImages[i].src = files[i];
  }
}

function onunhide(event) {
  document.getElementById('bottomBar').style.cssText = '';
}

function onhide(event) {
  document.getElementById('bottomBar').style.cssText = 'display: none';
}

function initGUIHooks() {
  var btn_minimize = document.getElementById('btn-minimize');
  btn_minimize.ondblclick = btn_minimize.onclick = function(event) {
    var bbwrap = document.getElementById('bbwrap');
    if (this.style.cssText == '') {
      this.style.cssText = 'background-image: url("minimize_light.png")';
      bbwrap.onmouseover = onunhide;
      bbwrap.onmouseout = onhide;
    } else {
      this.style.cssText = '';
      bbwrap.onmouseover = null;
      bbwrap.onmouseout = null;
      document.getElementById('bottomBar').style.cssText = '';
    }
  };
}

/* This function can be useful to prevent the user from triggering
   more internal errors if an internal error has been caught.  */
function dehookGUI() {
  var elmts = [ 'btn-minimize' ];

  for (var i = 0, elmts_len = elmts.length; i < elmts_len; i++) {
    var elmt = document.getElementById(elmts[i]);
    elmt.onchange = elmt.ondblclick = elmt.onclick = null;
  }

  /* Force the bottom bar to be visible once the CSS GUI has been
     immobilized.  */
  var btn_minimize = document.getElementById('btn-minimize');
  btn_minimize.style.cssText = '';
  var bbwrap = document.getElementById('bbwrap');
  bbwrap.onmouseover = null;
  bbwrap.onmouseout = null;
  document.getElementById('bottomBar').style.cssText = '';
}

function bodyInit() {
  initGUIHooks();
  // return OEV.Compositor.init();
}

var btnImages = [];
loadImages();

/* Enable tab indexes on CSS controls.  */
document.getElementById('btn-outer').tabIndex = 1;

/* Script Main Body Loader */

/* Note: These feature detection functions came from
   <http://michaux.ca/articles/feature-detection-state-of-the-art-browser-scripting> */

var isHostMethod = function(object, property) {
  var t = typeof(object[property]);
  return t=='function' ||
         (!!(t=='object' && object[property])) ||
         t=='unknown';
};

if (isHostMethod(window, 'addEventListener'))
  window.addEventListener("load", bodyInit, false);
else if (isHostMethod(window, 'attachEvent'))
  window.attachEvent("onload", bodyInit);
else window.onload = bodyInit;
