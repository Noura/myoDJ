import themidibus.*;
MidiBus myBus;

int Y_AXIS = 1;
int X_AXIS = 2;
color active_bottom, active_top, inactive_bottom, inactive_top;

int diameter = 50;
int myoin_max = 127;
int myoin_min = 0;
int myoin = 0;

void setup() {
  size(300, 800);
  active_bottom = color(255);
  active_top = color(150, 0, 255);
  inactive_bottom = color(230);
  inactive_top = color(50, 0, 50);

  MidiBus.list(); // List all available Midi devices on STDOUT. This will show each device's index and name.

  // Either you can
  //                   Parent In Out
  //                     |    |  |
  //myBus = new MidiBus(this, 0, 1); // Create a new MidiBus using the device index to select the Midi input and output devices respectively.

  // or you can ...
  //                   Parent         In                   Out
  //                     |            |                     |
  //myBus = new MidiBus(this, "IncomingDeviceName", "OutgoingDeviceName"); // Create a new MidiBus using the device names to select the Midi input and output devices respectively.

  // or for testing you could ...
  //                 Parent  In        Out
  //                   |     |          |
<<<<<<< HEAD
  
  myBus = new MidiBus(this, "To Processing", -1); // Create a new MidiBus with no input device and the default Java Sound Synthesizer as the output device.
  //myBus = new MidiBus(this, "LoopBe Internal MIDI", -1); // Create a new MidiBus with no input device and the default Java Sound Synthesizer as the output device.
}

void draw() {
  setGradient(0, 0, width, height, active_top, active_bottom, Y_AXIS);
  
  
  strokeWeight(5);
  stroke(0);
  fill(0, 0, 0, 0);
  int y = int(map(myoin, 0, 127, height, 0));
  line(0, y, width/2 - diameter/2, y);
  ellipse(width/2, y, diameter, diameter);
  line(width/2 + diameter/2, y, width, y);


}

void noteOn(int channel, int pitch, int velocity) {
  // Receive a noteOn
  println();
  println("Note On:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
}

void noteOff(int channel, int pitch, int velocity) {
  // Receive a noteOff
  println();
  println("Note Off:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
}

void controllerChange(int channel, int number, int value) {
  myoin = value;
  // Receive a controllerChange
  println();
  println("Controller Change:");
  println("--------");
  println("Channel:"+channel);
  println("Number:"+number);
  println("Value:"+value);
}

void setGradient(int x, int y, float w, float h, color c1, color c2, int axis ) {

  noFill();

  if (axis == Y_AXIS) {  // Top to bottom gradient
    for (int i = y; i <= y+h; i++) {
      float inter = map(i, y, y+h, 0, 1);
      color c = lerpColor(c1, c2, inter);
      stroke(c);
      line(x, i, x+w, i);
    }
  }  
  else if (axis == X_AXIS) {  // Left to right gradient
    for (int i = x; i <= x+w; i++) {
      float inter = map(i, x, x+w, 0, 1);
      color c = lerpColor(c1, c2, inter);
      stroke(c);
      line(i, y, i, y+h);
    }
  }
}

void delay(int time) {
  int current = millis();
  while (millis () < current+time) Thread.yield();
}
