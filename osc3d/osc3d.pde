/**
 * MPUTeapot Processing demo for MPU6050 DMP modified for OSC and BNO055
 *   https://gitub.com/jrowberg/i2cdevlib
 *   The demo used serial port I/O which has been replaced with OSC UDP messages in this sketch.
 *   The BNO055 is connected to an ESP8266 with battery so it is completely wire free.
 *   Tested on Processing 3.3.5 running on Ubuntu Linux 14.04
 *
 * Dependencies installed using Library Manager
 *
 * Open Sound Control library
 *   oscP5 website at http://www.sojamo.de/oscP5
 * ToxicLibs
 *   quaternion functions http://toxiclibs.org/
 *
 */

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// Install oscP5 using the IDE library manager.
// From the IDE menu bar, Sketch | Import Library | Add library. In the search box type "osc".
import oscP5.*;
import netP5.*;
// Install ToxicLibs using the IDE library manager
// From the IDE menu bar, Sketch | Import Library | Add library. In the search box type "toxic".
import toxi.geom.*;
import toxi.processing.*;

ToxiclibsSupport gfx;

Quaternion quat = new Quaternion(1, 0, 0, 0);

OscP5 oscP5;

void setup() {
  size(300, 300, P3D);
  frameRate(30);
  gfx = new ToxiclibsSupport(this);

  // setup lights and antialiasing
  lights();
  smooth();

  /* start oscP5, listening for incoming messages at port 9999 */
  oscP5 = new OscP5(this, 9999);

  oscP5.plug(this, "imu", "/bno055-a/imu");
}

public void imu(float euler_x, float euler_y, float euler_z,
  float quant_w, float quant_x, float quant_y, float quant_z,
  int cal_system, int cal_gyro, int cal_accel, int cal_mag) {
  //println(euler_x, euler_y, euler_z, 
  //  quant_w, quant_x, quant_y, quant_z, 
  //  cal_system, cal_gyro, cal_accel, cal_mag);
  quat.set(quant_w, quant_x, quant_y, quant_z);
}

void draw() {
  background(0);

  pushMatrix();
  translate(width/2, height/2, 0);

  // toxiclibs direct angle/axis rotation from quaternion (NO gimbal lock!)
  // (axis order [1, 3, 2] and inversion [-1, +1, +1] is a consequence of
  // different coordinate system orientation assumptions between Processing
  // and InvenSense DMP)
  float[] axis = quat.toAxisAngle();
  rotate(axis[0], -axis[1], axis[3], axis[2]);

  // draw main body in red
  fill(255, 0, 0, 200);
  box(10, 10, 200);

  // draw front-facing tip in blue
  fill(0, 0, 255, 200);
  pushMatrix();
  translate(0, 0, -120);
  rotateX(PI/2);
  drawCylinder(0, 20, 20, 8);
  popMatrix();

  // draw wings and tail fin in green
  fill(0, 255, 0, 200);
  beginShape(TRIANGLES);
  vertex(-100, 2, 30);
  vertex(0, 2, -80);
  vertex(100, 2, 30);  // wing top layer
  vertex(-100, -2, 30);
  vertex(0, -2, -80);
  vertex(100, -2, 30);  // wing bottom layer
  vertex(-2, 0, 98);
  vertex(-2, -30, 98);
  vertex(-2, 0, 70);  // tail left layer
  vertex( 2, 0, 98);
  vertex( 2, -30, 98);
  vertex( 2, 0, 70);  // tail right layer
  endShape();
  beginShape(QUADS);
  vertex(-100, 2, 30);
  vertex(-100, -2, 30);
  vertex(  0, -2, -80);
  vertex(  0, 2, -80);
  vertex( 100, 2, 30);
  vertex( 100, -2, 30);
  vertex(  0, -2, -80);
  vertex(  0, 2, -80);
  vertex(-100, 2, 30);
  vertex(-100, -2, 30);
  vertex(100, -2, 30);
  vertex(100, 2, 30);
  vertex(-2, 0, 98);
  vertex(2, 0, 98);
  vertex(2, -30, 98);
  vertex(-2, -30, 98);
  vertex(-2, 0, 98);
  vertex(2, 0, 98);
  vertex(2, 0, 70);
  vertex(-2, 0, 70);
  vertex(-2, -30, 98);
  vertex(2, -30, 98);
  vertex(2, 0, 70);
  vertex(-2, 0, 70);
  endShape();

  popMatrix();
}

/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  //print("### received an osc message.");
  //print(" addrpattern: "+theOscMessage.addrPattern());
  //println(" typetag: "+theOscMessage.typetag());
}

void drawCylinder(float topRadius, float bottomRadius, float tall, int sides) {
  float angle = 0;
  float angleIncrement = TWO_PI / sides;
  beginShape(QUAD_STRIP);
  for (int i = 0; i < sides + 1; ++i) {
    vertex(topRadius*cos(angle), 0, topRadius*sin(angle));
    vertex(bottomRadius*cos(angle), tall, bottomRadius*sin(angle));
    angle += angleIncrement;
  }
  endShape();

  // If it is not a cone, draw the circular top cap
  if (topRadius != 0) {
    angle = 0;
    beginShape(TRIANGLE_FAN);

    // Center point
    vertex(0, 0, 0);
    for (int i = 0; i < sides + 1; i++) {
      vertex(topRadius * cos(angle), 0, topRadius * sin(angle));
      angle += angleIncrement;
    }
    endShape();
  }

  // If it is not a cone, draw the circular bottom cap
  if (bottomRadius != 0) {
    angle = 0;
    beginShape(TRIANGLE_FAN);

    // Center point
    vertex(0, tall, 0);
    for (int i = 0; i < sides + 1; i++) {
      vertex(bottomRadius * cos(angle), tall, bottomRadius * sin(angle));
      angle += angleIncrement;
    }
    endShape();
  }
}
