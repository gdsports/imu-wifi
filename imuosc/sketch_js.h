/****************************************************************************
MIT License

Copyright (c) 2017-2018 gdsports625@gmail.com

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
var Euler = {heading: 0.0, pitch: 0.0, roll: 0.0};

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
    Euler = JSON.parse(evt.data);
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

function setup() {
  createCanvas(300, 300, WEBGL);
}

function draw() {
  background(64);

  push();
  // draw main body in red
  fill(255, 0, 0);

  rotateY(radians(-Euler.heading));
  rotateX(radians(Euler.pitch));
  rotateZ(radians(-Euler.roll));

  box(10, 10, 200);

  // draw wings in green
  fill(0, 255, 0);
  beginShape(TRIANGLES);
  vertex(-100, 2, 30);
  vertex(0, 2, -80);
  vertex(100, 2, 30);  // wing top layer

  vertex(-100, -2, 30);
  vertex(0, -2, -80);
  vertex(100, -2, 30);  // wing bottom layer
  endShape();

  // draw wing edges in slightly darker green
  fill(0, 192, 0);
  beginShape(TRIANGLES);
  vertex(-100, 2, 30);  // No quads so use 2 triangles to cover wing edges
  vertex(-100, -2, 30);
  vertex(  0, 2, -80);

  vertex(  0, 2, -80);
  vertex(  0, -2, -80);
  vertex(-100, -2, 30); // Left wing edge

  vertex( 100, 2, 30);
  vertex( 100, -2, 30);
  vertex(  0, -2, -80);

  vertex(  0, -2, -80);
  vertex(  0, 2, -80);
  vertex( 100, 2, 30);  // Right wing edge

  vertex(-100, 2, 30);
  vertex(-100, -2, 30);
  vertex(100, -2, 30);

  vertex(100, -2, 30);
  vertex(100, 2, 30);
  vertex(-100, 2, 30);  // Back wing edge
  endShape();

  // draw tail in green
  fill(0, 255, 0);
  beginShape(TRIANGLES);
  vertex(-2, 0, 98);
  vertex(-2, -30, 98);
  vertex(-2, 0, 70);  // tail left layer

  vertex( 2, 0, 98);
  vertex( 2, -30, 98);
  vertex( 2, 0, 70);  // tail right layer
  endShape();

  // draw tail edges in slightly darker green
  fill(0, 192, 0);
  beginShape(TRIANGLES);
  vertex(-2, 0, 98);
  vertex(2, 0, 98);
  vertex(2, -30, 98);

  vertex(2, -30, 98);
  vertex(-2, -30, 98);
  vertex(-2, 0, 98);  // tail back edge

  vertex(-2, 0, 98);
  vertex(2, 0, 98);
  vertex(2, 0, 70);

  vertex(2, 0, 70);
  vertex(-2, 0, 70);
  vertex(-2, 0, 98);  // tail front edge

  vertex(-2, -30, 98);
  vertex(2, -30, 98);
  vertex(2, 0, 70);

  vertex(2, 0, 70);
  vertex(-2, 0, 70);
  vertex(-2, -30, 98);
  endShape();

  pop();
}
)=====";
