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

#include <Arduino.h>

#define VERSION (1)
#define BAUDRATE (115200)

int sensorCount = 0;

// ---------------------------------------------------------------------------
//
// measurement
//
// ---------------------------------------------------------------------------

void findSensors() { //
}

// ---------------------------------------------------------------------------
//
// commands
//
// ---------------------------------------------------------------------------

#define LINEENDING "\r\n"

void output(char cmd, const char* arg) { //
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

void cmdVersion() { //
  output('v', VERSION);
}

void cmdConfig() { //
  output('c', sensorCount);
}

void msgError(String msg) { //
  output('e', msg);
}

int exec(String command) {

  // v    - get version
  // n    - get sensor count
  // c    - get config

  command.trim();

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
}

void loop() {

  String command= "";

  while (1) {

    if (Serial.available() > 0) {
      char inByte = Serial.read();
      switch (inByte) {
      case 10:
        break;
      case 13:
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
