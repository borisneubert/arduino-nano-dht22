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

commands (terminated with CR):

   v\r    - get version, also spontaneously sent on initialization

   d\r    - get data

return values (terminated with CR)

   v|arduino-nano-dht22|n\r   - version number n

   d|p|t|h|t|h|t|h|t|h        - data
    p:    binary representation of sensor presence
          Bit 0:  Sensor 0  .. Bit 3: Sensor 3
          e.g. 5 = sensors 0 and 2 present
    |t|h: group of temperature (degrees centigrade) and
          humidity (percent) readings, float;
          as many such groups are output as sensors are present,
          in ascending order

   e|m                        - error msg m (string)

*/

#include <Arduino.h>
#include <dht.h>

#define ID ("arduino-nano-dht22")
#define VERSION (2)
#define BAUDRATE (115200)
#define LINEENDING "\r"

// ---------------------------------------------------------------------------

byte dataPin[4] = {12, 11, 10, 9};
bool present[4];
int result[4];
double T[4];
double H[4];
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

void msgError(String msg) { //
  Serial.println("e|" + msg);
}

void cmdVersion() { //
  char txt[128];
  snprintf(txt, 128, "v|%s|%d%s", ID, VERSION, LINEENDING);
  Serial.print(txt);
}

void cmdData() {
  String result = "";
  int p = 0, b = 1;
  for (int i = 0; i < 4; i++) {
    if (present[i]) {
      p += b;
      if (measureTH(i)) {
        result += "|" + String(T[i]) + '|' + String(H[i]);
      } else {
        msgError(errorMsg(i));
        return;
      }
    }
    b *= 2;
  }
  Serial.print("d|" + String(p, HEX) + result + LINEENDING);
}

int exec(String command) {

  command.trim();
  if (command.length() == 0)
    return 0;
  if (command == "v")
    cmdVersion();
  else if (command == "d")
    cmdData();
  else {
    msgError("unknown command");
    return 0;
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
  findSensors();
  cmdVersion();
}

void loop() {

  String command = "";
  while (1) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      // Serial.print(inByte); // no echo
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
