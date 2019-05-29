/*

Copyright (C) 2019 Dr. Boris Neubert <omega@online.de>

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, version 3.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.

*/

/*

commands:

   v    - get version
   s i  - get presence of sensor i= 0..3
   t 0  - get temperature of sensor i= 0..3
   h 0  - get humidity of sensor i= 0..3
  
return values:

   v n   - version number n
   s i p - sensor i= 0..3 is present (p=1) or not (p=0)
   t i T - sensor i= 0..3 temperature T (float)
   h i H - sensor i= 0..3 humidity H (float)
   e m   - error msg m (string)

version information is sent at startup
presence information for sensors 0..3 is sent at startup
temperature and humidity for all present sensors is sent every 5 seconds

*/

#include <Arduino.h>
#include <dht.h>

#define VERSION (1)
#define BAUDRATE (115200)
#define LINEENDING "\r\n"
#define DEFAULTINTERVAL (60)

// ---------------------------------------------------------------------------

byte dataPin[4] = {12, 11, 10, 9};
bool present[4];
int result[4];
double T[4];
double H[4];
unsigned long int interval = DEFAULTINTERVAL * (unsigned long int) 1000;
dht DHT;

// ---------------------------------------------------------------------------
//
// measurement
//
// ---------------------------------------------------------------------------

void findSensors() { //
  int i;
  // set data pins for input
  for (i = 0; i < 4; i++) {
    pinMode(dataPin[i], INPUT);
  }
  // wait for sensors to become ready
  delay(2000);
  // detect sensors
  for (i = 0; i < 4; i++) {
    present[i] = (DHT.read(dataPin[i]) == DHTLIB_OK);
  }
}

bool measureTH(int i) { //

  if (!present[i]) {
    result[i] = DHTLIB_ERROR_CONNECT;
    return false;
  }

  result[i] = DHT.read(dataPin[i]);
  if (result[i] == DHTLIB_OK) {
    T[i] = DHT.temperature;
    H[i] = DHT.humidity;
    return true;
  } else
    return false;
}

String errorMsg(int i) {
  switch (result[i]) {
  case DHTLIB_OK:
    return "no error";
  case DHTLIB_ERROR_CHECKSUM:
    return "checksum error";
  case DHTLIB_ERROR_TIMEOUT:
    return "timeout";
  case DHTLIB_ERROR_CONNECT:
    return "connect error";
  case DHTLIB_ERROR_ACK_L:
    return "ack low error";
  case DHTLIB_ERROR_ACK_H:
    return "ack high error";
  default:
    return "unknown error";
  }
}

// ---------------------------------------------------------------------------
//
// commands
//
// ---------------------------------------------------------------------------

void output(char cmd, const char *arg) { //
  char txt[128];
  snprintf(txt, 128, "%c %s%s", cmd, arg, LINEENDING);
  Serial.print(txt);
}

void output(char cmd, String arg) { //
  char txt[128];
  snprintf(txt, 128, "%c %s%s", cmd, arg.c_str(), LINEENDING);
  Serial.print(txt);
}

void output(char cmd, int arg) { //
  char txt[128];
  snprintf(txt, 128, "%c %d%s", cmd, arg, LINEENDING);
  Serial.print(txt);
}

void output(char cmd, int arg1, int arg2) { //
  char txt[128];
  snprintf(txt, 128, "%c %d %d%s", cmd, arg1, arg2, LINEENDING);
  Serial.print(txt);
}

void output(char cmd, int arg1, double arg2) { //
  char txt[128];
  // The %f format specifier is not supported on the Arduino.
  int wholePart = arg2;
  int fractPart = (arg2 - wholePart) * 10;
  snprintf(txt, 128, "%c %d %d.%d%s", cmd, arg1, wholePart, fractPart, LINEENDING);
  Serial.print(txt);
}

void msgError(String msg) { //
  output('e', msg);
}

void cmdVersion() { //
  output('v', VERSION);
}

void cmdSensor(int i) { //
  output('s', i, present[i]);
}

void cmdT(int i) { //
  if (present[i])
    output('t', i, T[i]);
  else
    msgError("sensor not present");
}

void cmdH(int i) { //
  if (present[i])
    output('h', i, H[i]);
  else
    msgError("sensor not present");
}

int exec(String command) {

  command.trim();
  if (command.length() == 0)
    return 0;
  char c;
  int i;
  sscanf(command.c_str(), "%c %d", &c, &i);
  if ((i < 0) || (i > 3))
    i = 0;
  switch (c) {
  case 'v':
    cmdVersion();
    break;
  case 's':
    cmdSensor(i);
    break;
  case 't':
    cmdT(i);
    break;
  case 'h':
    cmdH(i);
    break;
  default:
    msgError("unknown command");
    break;
  }

  return 1;
}

// ---------------------------------------------------------------------------
//
// main
//
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(BAUDRATE);
  cmdVersion();
  findSensors();
  for (int i = 0; i < 4; i++)
    cmdSensor(i);
}

void loop() {

  String command = "";
  unsigned long int t0, t;

  t0 = millis();
  while (1) {
    // periodically output readings
    t = millis();
    if (t < t0)
      t0 = t; // counter wraparound
    if (t - t0 > interval) {
      // measure and output
      for (int i = 0; i < 4; i++)
        if (present[i]) {
          if (measureTH(i)) {
            cmdT(i);
            cmdH(i);
          } else
            msgError(errorMsg(i));
        }
      t0 = t;
    }

    if (Serial.available() > 0) {
      char inByte = Serial.read();
      // Serial.print(inByte); // no echo
      switch (inByte) {
      case 13:
        break;
      case 10:
        exec(command);
        command = "";
        break;
      default:
        command.concat(inByte);
        break;
      }
    }
  }
}
