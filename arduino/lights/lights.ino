#include <math.h>

// this may be different on your board
#define RED 3
#define GREEN 5
#define BLUE 6

#define MESSAGE_LENGTH 7
#define BAUD_RATE 115200

// color messages start with a '#'
const int COLOR_MESSAGE_START = 35;

// mode change messages start with a '*'
const int MODE_MESSAGE_START = 42;

unsigned long time;
int atByte = 0;

boolean doStep = true;
int msg[MESSAGE_LENGTH];

unsigned long currentStep = 0;
int stepTime = 200;

// rgb color values
int rgb[3] = {0,0,0};

// the program to run
void (*stepProgram)(int[]);

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  makeItSo(rgb);

  Serial.begin(BAUD_RATE);

  stepProgram = throughTheHues;
}

void loop() {
  // if enough time has passed step the current program and display the color
  if (doStep && millis() - time >= stepTime) {
    stepProgram(rgb);
    makeItSo(rgb);
    printColor(rgb);
    time = millis();
    currentStep++;
  }

  // if enough bytes have been recevied for a complete message
  if (atByte == MESSAGE_LENGTH) {
    // message starts with a star so it's a mode
    if (msg[0] == MODE_MESSAGE_START) {
      switch(msg[1]) {
        case 114:
          stepProgram = rave;
          break;
        case 97:
          stepProgram = alarm;
          break;
        case 115:
          stepProgram = threesins;
          break;
        case 104:
        default:
          stepProgram = throughTheHues;
          break;
      }
      doStep = true;
    }
    // message starts with a hash so it's a color
    else if (msg[0] == COLOR_MESSAGE_START) {
      hexToRgb(msg, rgb);
      makeItSo(rgb);
      printColor(rgb);
      doStep = false;
    }
    resetStream();
  }

  // read serial if available
  if (Serial.available() > 0) {
    msg[atByte] = Serial.read();
    if (msg[0] == COLOR_MESSAGE_START || msg[0] == MODE_MESSAGE_START) {
      atByte++;
    } 
    else {
      // reset if the message doesn't start with a valid character
      resetStream();
    }
  }
}

void rave(int rgb[]) {
  stepTime = 60;
  hslToRgb(random(0, 256), 255, 128, rgb);
}

// go through the hues but flash the opposite hue half the time
void raveOld(int rgb[]) {
  stepTime = 80;
  hslToRgb(currentStep % 255, 255, 128, rgb);
  if (currentStep % 2 == 0) hslToRgb((currentStep + 128) / 255, 255, 128, rgb);
}

void black(int rgb[]) {
  stepTime = 100; // could be anything
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
}

// gently cycle through the hues
void throughTheHues(int rgb[]) {
  stepTime = 300;
  hslToRgb(currentStep % 255, 255, 128, rgb);
}

// pulse only red
void alarm(int rgb[]) {
  stepTime = 80;
  rgb[0] = 10 + (int)ceil(sinsq(deg2rad(currentStep % 180)) * 245);
  rgb[1] = 0;
  rgb[2] = 0;
}

// pulse red then green then blue
void threesins(int rgb[]) {
  stepTime = 50;
  int minval = 0;
  rgb[0] = minval;
  rgb[1] = minval;
  rgb[2] = minval;

  int color = currentStep % 540;
  if (color > 360) {
    color = 2;
  } 
  else if (color > 180) {
    color = 1;
  } 
  else {
    color = 0;
  }

  rgb[color] = 1 + (int)ceil(sinsq(deg2rad(currentStep % 360)) * 254);
}

// each color is a sin wave offset evenly from the others
void threesinsoverlap(int rgb[]) {
  stepTime = 100;
  rgb[0] = 1 + (int)ceil(sinsq(deg2rad(currentStep % 360)) * 254);
  rgb[1] = 1 + (int)ceil(sinsq(deg2rad((currentStep % 360) - (2 * (M_PI / 3)))) * 254);
  rgb[2] = 1 + (int)ceil(sinsq(deg2rad(currentStep % 360 + (2 * (M_PI / 3)))) * 254);
}

// reset message
void resetStream() {
  for (int j = 0; j < MESSAGE_LENGTH; j++) {
    msg[j] = 0;
  }
  atByte = 0;
}

// actually change the color of the leds
void makeItSo(int rgb[]) {
  analogWrite(RED, rgb[0]);
  analogWrite(GREEN, rgb[1]);
  analogWrite(BLUE, rgb[2]);
}

void printColor(int rgb[]) {
  Serial.print("rgb(");
  Serial.print(rgb[0]);
  Serial.print(",");
  Serial.print(rgb[1]);
  Serial.print(",");
  Serial.print(rgb[2]);
  Serial.print(")\n");
}

void hexToRgb(int msg[], int* rgb) {
  rgb[0] = 16 * val(msg[1]) + val(msg[2]);
  rgb[1] = 16 * val(msg[3]) + val(msg[4]);
  rgb[2] = 16 * val(msg[5]) + val(msg[6]);
}

// the decimal value of a hex number character
int val(int val) {
  if (val >= 48 && val <= 57) { // numbers
    return val - 48;
  }
  if (val >= 65 && val <= 70) { // uppercase
    return val - 54;
  }
  if (val >= 97 && val <= 102) { // lowercase
    return val - 87;
  }
  return 0;
}

double deg2rad (double x) {
  if(x>=0 && x<=180) {
    return -x*M_PI/180;
  } 
  else {
    return (360-x)*M_PI/180;
  }
}

double sinsq(double x) {
  return sin(x) * sin(x);
}

void hslToRgb(int _h, int _sl, int _l, int* rgb) {
  double h = (double)_h / 255;
  double sl = (double)_sl / 255;
  double l = (double)_l / 255;
  double v;
  double r, g, b;
  r = l;   // default to gray
  g = l;
  b = l;
  v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
  if (v > 0) {
    double m;
    double sv;
    int sextant;
    double fract, vsf, mid1, mid2;

    m = l + l - v;
    sv = (v - m) / v;
    h *= 6.0;
    sextant = (int)h;
    fract = h - sextant;
    vsf = v * sv * fract;
    mid1 = m + vsf;
    mid2 = v - vsf;
    switch (sextant) {
      case 0:
        r = v;
        g = mid1;
        b = m;
        break;
      case 1:
        r = mid2;
        g = v;
        b = m;
        break;
      case 2:
        r = m;
        g = v;
        b = mid1;
        break;
      case 3:
        r = m;
        g = mid2;
        b = v;
        break;
      case 4:
        r = mid1;
        g = m;
        b = v;
        break;
      case 5:
        r = v;
        g = m;
        b = mid2;
        break;
    }
  }
  rgb[0] = (int)(r * 255);
  rgb[1] = (int)(g * 255);
  rgb[2] = (int)(b * 255);
}

