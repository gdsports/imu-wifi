/****************************************************************************
MIT License

Copyright (c) 2017 gdsports625@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************/
const char SKETCH_JS[] PROGMEM = R"=====(
function enableTouch(objname) {
  console.log('enableTouch', objname);
  var e = document.getElementById(objname);
  if (e) {
    e.addEventListener('touchstart', function(event) {
        event.preventDefault();
        console.log('touchstart', event);
        buttondown(e);
        }, false );
    e.addEventListener('touchend',   function(event) {
        console.log('touchend', event);
        buttonup(e);
        }, false );
  }
  else {
    console.log(objname, ' not found');
  }
}

var websock;
var WebSockOpen=0;  //0=close,1=opening,2=open
var Euler_yaw=0.0, Euler_roll=0.0, Euler_pitch=0.0;

function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  WebSockOpen=1;
  websock.onopen = function(evt) {
    console.log('websock open');
    WebSockOpen=2;
    var e = document.getElementById('webSockStatus');
    e.style.backgroundColor = 'green';
  };
  websock.onclose = function(evt) {
    console.log('websock close');
    WebSockOpen=0;
    var e = document.getElementById('webSockStatus');
    e.style.backgroundColor = 'red';
  };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    //console.log('websock message ' + evt.data);
    var euler_angles = JSON.parse(evt.data);
    Euler_yaw   = -euler_angles.x;
    Euler_roll  = euler_angles.y;
    Euler_pitch = euler_angles.z;
  };

  var allButtons = [
    'bForward',
    'bBackward',
    'bRight',
    'bLeft',
    'bLedon',
    'bLed50',
    'bLedoff'
  ];
  for (var i = 0; i < allButtons.length; i++) {
    enableTouch(allButtons[i]);
  }
}
function buttondown(e) {
  switch (WebSockOpen) {
    case 0:
      window.location.reload();
      WebSockOpen=1;
      break;
    case 1:
    default:
      break;
    case 2:
      websock.send(e.id + '=1');
      break;
  }
}
function buttonup(e) {
  switch (WebSockOpen) {
    case 0:
      window.location.reload();
      WebSockOpen=1;
      break;
    case 1:
    default:
      break;
    case 2:
      websock.send(e.id + '=0');
      break;
  }
}
)=====";
