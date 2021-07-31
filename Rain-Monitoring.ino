/*
Copyright (c) 2021 BankAkira

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
*/

#include <SD.h>
#include <RTClib.h>

RTC_DS3231 RTC;


//set multiplier for rain sensor
double rainMuliplier = 0.254;

const byte rainInterruptPin = 3;
int rainCnt = 0;
void rainCount() {
    rainCnt++;
}

const int chipSelect = 53;
File dataFile;
bool fileError = false;
bool sdError = false;
int nowHr = 99;

void logHeader(bool fileError, bool sdError) {
  if (!fileError and !sdError) {
    dataFile.print("Date");
    dataFile.print(',');
    dataFile.print("Time");
    dataFile.print(',');
    dataFile.println("Rain(mm)");
    dataFile.flush();
    dataFile.close();
  }
}

void logData(bool fileError, bool sdError, DateTime now) {
    //write DateTime
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(',');
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(',');
    dataFile.println(rainCnt*rainMuliplier);
    dataFile.flush();
    dataFile.close();
}

void setup() {
    Serial.begin(9600);
    //init rain counter
    pinMode(rainInterruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(rainInterruptPin), rainCount, FALLING);
    // init RTC
    RTC.begin();
    DateTime now = RTC.now();
    Serial.println(now.year());
    String fileLog = String(now.year());
    if (now.month() < 10) {
        fileLog += "-0";
        fileLog += String(now.month());
    }
    else {
        fileLog += "-";
        fileLog += String(now.month());
    }
    fileLog += ".csv";
    Serial.print(fileLog);
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
    SD.begin(chipSelect);
    dataFile = SD.open(fileLog, FILE_WRITE);
    //write log header 
    logHeader(fileError, sdError);
}


void loop() {
    DateTime now = RTC.now();
    // when xx:00 c'clock log data to sd card and reset rain count
    if (nowHr != now.hour()) {
        nowHr = now.hour();
        String fileLog = String(now.year());
        if (now.month() < 10) {
            fileLog += "-0";
            fileLog += String(now.month());
        }
        else {
            fileLog += "-";
            fileLog += String(now.month());
        }
        fileLog += ".csv";
        dataFile = SD.open(fileLog, FILE_WRITE);
        logData(fileError, sdError, now);
        rainCnt = 0;
    }
    delay(10);
}