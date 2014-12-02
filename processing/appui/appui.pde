import themidibus.*;
MidiBus myBus;

// SETTINGS ////////////////////////////////////////////////
// activate/deactivate key
char TOGGLE_KEY = 'm';
int TOGGLE_CHANNEL = 1;
int FX_CHANNEL = 1;
int SLIDER_CHANNEL = 1;
color inactive_veil_color;

// gradient settings
int Y_AXIS = 1;
int X_AXIS = 2;
color gradient_bot_color, gradient_top_color;

// myo in settings
int diameter = 50;
int myoin_max = 127;
int myoin_min = 0;

// menu of effects choices
int fxrow_h = 50;
int text_left_indent = 10;
int text_top_indent = 5;
int text_size = 28;
int menu_h;
color text_color, row_color, chosen_row_color;
///////////////////////////////////////////////////////////////

// INTERNAL APP STATE /////////////////////////////////////////
ArrayList<String> fx;
int fx_chosen = 0; // index in fx
int[] myoins = new int[100];
boolean active = true;

void setup() {
  size(300, 800);
  
  // MORE SETTINGS ///////////////////////////////////////////
  inactive_veil_color = color(100, 100, 100, 100);

  // gradient
  gradient_bot_color = color(255);
  gradient_top_color = color(150, 0, 255);
  
  // effects menu
  fx = new ArrayList<String>();
  fx.add("Reverb");
  fx.add("Distort");
  fx.add("Flanger");
  fx.add("Phaser");
  /////////////////////////////////////////////////////////////
  
  menu_h = fx.size() * fxrow_h;
  text_color = color(225, 225, 225);
  row_color = color(50, 50, 50);
  chosen_row_color = color(100, 100, 100);
  
  // MIDI setup
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
  
  //myBus = new MidiBus(this, "pd to processing", "processing to pd"); // Create a new MidiBus with no input device and the default Java Sound Synthesizer as the output device.
  myBus = new MidiBus(this, "Myo to Processing", "Processing to Myo"); // Create a new MidiBus with no input device and the default Java Sound Synthesizer as the output device.
}

void draw() {
  int h = height - menu_h;
  
  setGradient(0, 0, width, h, gradient_top_color, gradient_bot_color, Y_AXIS);
  
  // slider position
  int y = int(map(myoins[fx_chosen], 0, 127, h, 0));
  strokeWeight(0);
  fill(0);
  rect(0, 0, width, y);
  
  // menu
  int row_y = h;
  strokeWeight(0);
  textSize(20);
  for (int i = 0; i < fx.size(); i++) {
    if (fx_chosen == i) {
      fill(chosen_row_color);
    } else {
      fill(row_color);
    }
    rect(0, row_y, width, row_y + fxrow_h);
    fill(text_color);
    textSize(text_size);
    text(str(i + 1) + " - " + fx.get(i), text_left_indent, row_y + fxrow_h/2 + text_top_indent);

    row_y += fxrow_h;
  }
  
  if (!active) {
    strokeWeight(0);
    fill(inactive_veil_color);
    rect(0, 0, width, height); 
  }
}

void keyPressed() {
  // pressing number keys chooses an effect
  int i = int(str(key)) - 1;
  if (i >= 0 && i < fx.size()) {
     fx_chosen = i;
     myBus.sendNoteOn(FX_CHANNEL, fx_chosen + 1, 100);
     return;
  }
  
  if (key == TOGGLE_KEY) {
    active = !active;
    if(active)
      myBus.sendNoteOn(TOGGLE_CHANNEL, 0, 100);
    else
      myBus.sendNoteOff(TOGGLE_CHANNEL, 0, 100);
    return;
  } 
}

void noteOn(int channel, int pitch, int velocity) {
  println("Note On channel " + channel + " pitch " + pitch + " velocity " + velocity);
  toggleActiveFromMidi(channel, pitch, velocity);
}

void noteOff(int channel, int pitch, int velocity) {
  println("Note Off channel " + channel + " pitch " + pitch + " velocity " + velocity);
  toggleActiveFromMidi(channel, pitch, velocity);
}

void toggleActiveFromMidi(int channel, int pitch, int velocity) {
  if (channel == TOGGLE_CHANNEL && pitch == 0) {
    active = !active;
  }
}

void controllerChange(int channel, int number, int value) {
    println("Controller Change channel " + channel + " number " + number + " value " + value);
  if (channel == SLIDER_CHANNEL && active) {
    myoins[fx_chosen] = value;
  }
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
