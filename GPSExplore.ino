#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <PROGMEM_readAnything.h>
#include <MD_YX5300.h>

/*
   GPS data Notes:
      S and W are negative values
      I am using decimal degrees notation
      To fetch country GPS boundary relation number: https://nominatim.openstreetmap.org/
      To fetch country GPS boundary data polygon:http://polygons.openstreetmap.fr/index.py

   MP3 setup notes:
      The micro SD card should be formatted as FAT16 or FAT32
      Songs must be prefixed with a unique 3 digit index number
      MD_YX5300 library documentation: https://majicdesigns.github.io/MD_YX5300/class_m_d___y_x5300.html

   UPDATES:
      (B)This version has stripped code: no time data, only scans when first powered up to save energy.
      (C)This verion was tested around the block and works seamlessly! (Detects if inside or outside polygon)
      (D)This version includes 3 area near my place, it detects what location you are in and displays it.
      (E)This version has succesfully moved the gps area data structure into flash memory using PROGMEM
      (F)Stable Backup - minor bug fixes
      (G)Converted syntax to lat/lng for clarity / included manual coordinate injection for testing / all countries working
      (H)Added mp3 player code, few bugs
      (I)Added Ethel house data, bug fixes
      (J)Added KLO Creek and Pebble Beach
      (K)Now tracks are repeated
      (L)Added pandosy house area

   I am using the point inclusion in polygon test, following is the copyright licence to give due credit to the original creator.
   https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html - Point Inclusion in Polygon Test W. Randolph Franklin (WRF)

        "Copyright (c) 1970-2003, Wm. Randolph Franklin

        Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
        documentation files (the "Software"), to deal in the Software without restriction, including without limitation
        the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
        to permit persons to whom the Software is furnished to do so, subject to the following conditions:

        Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
        Redistributions in binary form must reproduce the above copyright notice in the documentation and/or other materials provided with the distribution.
        The name of W. Randolph Franklin may not be used to endorse or promote products derived from this Software without specific prior written permission.
        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
        IN THE SOFTWARE."
*/
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// Connections for serial interface to the YX5300 module
const uint8_t ARDUINO_MP3_RX = 5;    // connect to TX of MP3 Player module
const uint8_t ARDUINO_MP3_TX = 6;    // connect to RX of MP3 Player module

// Define global variables
MD_YX5300 mp3(ARDUINO_MP3_RX, ARDUINO_MP3_TX);

struct areaData {
  char areaName[24];
  int numVertices;
  float verticesX[64];
  float verticesY[64];
};

const areaData areaDataArray[15] PROGMEM = {
  {"egypt", 64, { 30.07, 30.11, 30.69, 30.82, 31.14, 31.36, 31.66, 31.71, 31.84, 31.64, 31.51, 31.49, 31.41, 31.33, 31.13, 31.25, 31.55, 31.74, 31.84, 31.79, 31.76, 31.76, 31.56, 31.49, 31.37, 31.48, 31.36, 31.45, 31.51, 30.89, 30.55, 30.44, 29.77, 29.66, 29.47, 29.26, 29.09, 28.89, 28.57, 28.19, 28.01, 27.7, 27.17, 26.82, 25.83, 25.48, 24.73, 24.39, 24.28, 23.75, 22.97, 22.67, 22.45, 22.13, 21.98, 21.95, 21.95, 21.96, 22.01, 21.96, 21.98, 29.49, 29.78, 30.07} , { 24.71, 24.61, 24.92, 24.95, 24.82, 24.8, 25.06, 25.17, 26.3, 27.16, 27.58, 27.84, 28.07, 28.49, 29.05, 29.42, 29.93, 30.42, 30.99, 31.37, 31.76, 32.01, 32.26, 32.52, 32.82, 33.14, 33.6, 33.95, 34.1, 34.43, 34.56, 34.58, 34.89, 34.92, 34.96, 34.87, 34.81, 34.77, 34.72, 34.58, 34.5, 34.56, 34.26, 34.35, 34.82, 35.09, 35.45, 35.87, 35.95, 36.51, 36.59, 36.84, 36.99, 37.15, 37.15, 36.88, 34.34, 31.38, 31.36, 31.33, 24.96, 24.84, 24.77, 24.71} },
  {"france", 63,  { 48.47, 49.12, 48.96, 49.4, 49.96, 49.59, 50.17, 50.83, 51.32, 50.87, 50.77, 50.57, 50.4, 50.12, 50.03, 50.07, 49.84, 49.57, 49.49, 49.38, 49.19, 49.18, 49.01, 48.72, 48.3, 47.92, 47.53, 47.46, 47.19, 47, 46.67, 46.41, 46.32, 45.84, 45.55, 45.23, 45.1, 44.85, 44.55, 44.24, 44.22, 43.75, 43.43, 43.2, 42.81, 43.22, 42.96, 42.42, 42.35, 42.45, 42.63, 42.76, 42.65, 42.76, 42.93, 42.99, 43.21, 43.7, 45.29, 46.04, 47.04, 47.45, 48.09}, { -5.49, -3.64, -2.49, -2.1, -1.45, -0.27, 1, 1.22, 2.17, 2.65, 3.23, 3.45, 4.04, 4.24, 4.59, 4.91, 5.02, 5.54, 6.01, 6.64, 7.07, 7.54, 8.21, 8.03, 7.78, 7.64, 7.57, 7.04, 6.96, 6.68, 6.36, 6.19, 6.89, 7.03, 7.03, 7.17, 6.7, 7.06, 6.97, 7.21, 7.69, 7.58, 7.45, 7.01, 5.76, 4.29, 3.37, 2.87, 2.41, 1.8, 1.38, 0.7, 0.25, -0.24, -0.77, -1.34, -1.56, -1.76, -1.49, -1.8, -3.03, -4.19, -5.36} },
  {"india", 64, { 23.51, 24.31, 24.43, 26.53, 27.95, 30.06, 30.93, 32.09, 32.69, 33.74, 34.47, 34.73, 35.36, 35.53, 34.65, 34.05, 33.17, 32.42, 31.7, 31.28, 30.76, 29.96, 29.07, 28.3, 27.54, 27.19, 26.71, 26.46, 27, 28, 27.41, 26.78, 26.85, 27.74, 27.87, 28.56, 29.27, 29.39, 28.66, 27.83, 27.16, 26.79, 25.81, 24.68, 23.93, 22.61, 22.06, 23.45, 22.99, 23.55, 24.26, 25.09, 25.23, 26.01, 26.32, 25.7, 25.14, 24.19, 23.21, 21.93, 18.6, 9.66, 13.81, 22.62}, { 68.09, 69.13, 70.96, 70.14, 70.71, 73.58, 74.37, 74.86, 74.61, 73.92, 73.84, 76.03, 76.83, 77.66, 78.31, 78.75, 79.21, 78.77, 78.84, 79.29, 80.29, 80.73, 80.17, 81.27, 82.94, 84.72, 86.02, 87.42, 88.01, 88.28, 88.9, 89.69, 91.71, 91.6, 92.46, 93.33, 94.53, 95.86, 96.62, 97.35, 96.84, 95.56, 95.09, 94.64, 93.56, 93.18, 92.72, 92.28, 91.8, 91.16, 91.9, 92.19, 89.99, 89.62, 88.63, 88.48, 88.54, 88.79, 89.04, 89.12, 84.92, 79.42, 74.29, 68.81} },
  {"israel", 49, { 31.21, 31.31, 31.54, 31.78, 32.01, 32.3, 32.8, 33.03, 33.13, 33.1, 33.13, 33.33, 33.32, 33.35, 33.14, 32.87, 32.69, 32.64, 32.36, 32.37, 32.47, 32.46, 32.34, 32.24, 32.02, 31.89, 31.92, 31.88, 31.67, 31.63, 31.47, 31.41, 31.53, 31.33, 31.19, 31.03, 30.88, 30.73, 30.64, 30.43, 30.28, 30.02, 29.86, 29.75, 29.53, 29.5, 29.67, 30.4, 30.67}, { 34.23, 34.33, 34.5, 34.29, 34.44, 34.56, 34.68, 34.81, 35.1, 35.36, 35.47, 35.55, 35.66, 35.84, 35.87, 35.88, 35.76, 35.64, 35.59, 35.41, 35.27, 35.15, 35.09, 35.07, 35.05, 35.08, 35.19, 35.28, 35.26, 35.07, 34.98, 35.22, 35.5, 35.49, 35.48, 35.46, 35.38, 35.33, 35.3, 35.2, 35.19, 35.15, 35.12, 35.06, 35.02, 34.85, 34.82, 34.5, 34.46}  },
  {"jordan", 55, { 29.43, 29.55, 29.66, 29.79, 29.97, 30.12, 30.31, 30.46, 30.59, 30.92, 30.97, 31.11, 31.19, 31.39, 31.7, 31.87, 31.95, 32.07, 32.31, 32.52, 32.66, 32.8, 32.76, 32.7, 32.62, 32.56, 32.54, 32.41, 32.36, 32.48, 32.6, 32.63, 32.67, 32.8, 32.9, 32.95, 33.17, 33.21, 33.41, 32.47, 32.35, 32.39, 31.95, 31.7, 31.48, 30.47, 29.96, 29.75, 29.63, 29.49, 29.39, 29.32, 29.3, 29.35, 29.43}, { 34.87, 34.94, 34.98, 34.99, 35.04, 35.12, 35.11, 35.12, 35.16, 35.31, 35.38, 35.42, 35.37, 35.43, 35.47, 35.5, 35.5, 35.48, 35.51, 35.51, 35.52, 35.8, 35.95, 36.04, 36.07, 36.11, 36.24, 36.43, 36.72, 36.94, 37.2, 37.36, 37.41, 37.72, 37.73, 37.82, 38.32, 38.4, 38.82, 39.11, 39.07, 39.29, 38.99, 37.99, 37.09, 38.03, 37.52, 36.76, 36.67, 36.56, 36.47, 36.34, 35.11, 34.85, 34.87}  },
  {"morocco", 60, { 21.31, 22.61, 24.63, 25.9, 26.74, 27.9, 28.29, 29.54, 30.44, 31.89, 32.95, 34.07, 35.87, 35.96, 35.64, 35.36, 34.95, 34.64, 34.24, 33.71, 33.41, 33.03, 32.63, 32.31, 32.08, 32.1, 32.12, 31.94, 31.37, 31.19, 30.84, 30.55, 29.96, 29.47, 29.55, 29.53, 29.46, 29.34, 29.14, 28.75, 27.22, 26.93, 26.82, 26.71, 26.8, 26.71, 26.44, 25.96, 25.46, 24.94, 24.71, 24.4, 23.94, 23.86, 23.41, 22.97, 22.14, 21.65, 21.32, 21.34} , { -17.26, -16.72, -15.42, -14.87, -14.05, -13.33, -12.34, -10.39, -10.07, -9.88, -9.22, -7.2, -6.2, -4.99, -3.14, -2.17, -1.93, -1.78, -1.67, -1.62, -1.59, -1.42, -1.11, -1.2, -1.12, -1.79, -2.23, -2.84, -3.61, -3.6, -3.58, -3.8, -5.11, -5.68, -5.85, -6.44, -6.92, -7.42, -7.89, -8.44, -8.74, -9.07, -9.62, -9.85, -10.43, -11.22, -11.45, -11.66, -11.98, -12.29, -12.55, -12.89, -13.07, -13.42, -13.88, -14.01, -14.31, -14.46, -14.83, -16.68} },
  {"napal", 61, { 28.88, 29.18, 29.4, 29.7, 29.86, 30.04, 30.24, 30.09, 30.46, 30.43, 30.4, 30.12, 29.92, 29.73, 29.61, 29.38, 29.25, 29.37, 29.2, 28.91, 28.76, 28.63, 28.46, 28.37, 28.25, 27.95, 28.13, 27.95, 28.13, 27.99, 27.89, 27.88, 27.96, 27.89, 27.59, 27.37, 27.11, 26.73, 26.33, 26.34, 26.31, 26.48, 26.42, 26.58, 26.61, 26.6, 26.74, 26.83, 26.93, 27.23, 27.44, 27.32, 27.32, 27.41, 27.65, 27.83, 27.99, 28.15, 28.46, 28.53, 28.72}, { 80.02, 80.2, 80.2, 80.33, 80.5, 80.72, 81.01, 81.25, 81.39, 81.76, 82.1, 82.25, 82.63, 82.86, 83.3, 83.46, 83.67, 83.98, 84.24, 84.31, 84.67, 84.95, 85.18, 85.4, 85.79, 86.07, 86.26, 86.42, 86.64, 86.98, 87.3, 87.68, 87.95, 88.22, 88.15, 88.1, 88.06, 88.23, 87.97, 87.62, 87.32, 86.97, 86.63, 86.27, 86.01, 85.73, 85.46, 85.14, 84.93, 84.64, 84.17, 83.82, 83.4, 83.01, 82.58, 81.96, 81.58, 81.26, 80.85, 80.56, 80.19} },
  {"turkey", 63, { 40.08, 40.92, 41.37, 41.86, 42.11, 41.97, 41.96, 41.49, 41.38, 41.45, 41.95, 42.26, 42.34, 41.91, 41.59, 41.38, 41.2, 41.35, 41.36, 41.52, 41.51, 41.6, 41.25, 40.82, 40.28, 39.99, 39.71, 39.67, 39.34, 38.97, 38.41, 37.87, 37.47, 37.22, 37.04, 37.21, 37.21, 37.33, 37.07, 37.04, 37.08, 36.82, 36.67, 36.75, 36.63, 36.64, 36.22, 35.81, 36.13, 36.29, 35.88, 35.8, 36.25, 36.6, 35.93, 36.07, 36.26, 36.43, 36.6, 37.29, 37.93, 38.72, 39.36} , { 25.6, 26.19, 26.59, 26.52, 26.99, 27.55, 28.36, 29.16, 30.04, 31.14, 32.22, 33.29, 34.91, 35.72, 36.48, 37.29, 38.18, 39.41, 40.62, 41.67, 42.41, 42.88, 43.24, 43.76, 43.72, 44.52, 44.84, 44.52, 44.26, 44.23, 44.41, 44.44, 44.63, 44.8, 44.5, 44.22, 43.56, 42.79, 42.33, 41.54, 40.77, 40.08, 38.72, 37.92, 37.13, 36.62, 36.73, 36.21, 35.57, 34.36, 33.58, 32.69, 31.95, 31.03, 30.47, 29.66, 28.85, 28.02, 27.27, 27.12, 26.91, 26.19, 26.12} },
  {"hospital block", 4, {49.874889, 49.874783, 49.872290, 49.872383}, { -119.498496, -119.491263, -119.491139, -119.498397}},
  {"KLO Creek", 4, {49.833676, 49.834238, 49.807634, 49.807126}, { -119.364500, -119.372813, -119.371577, -119.344358}},
  {"lake Ave Block", 4, {49.880595, 49.880588, 49.878882, 49.878875}, { -119.499214, -119.496789, -119.496817, -119.497616}},
  {"ethel house", 4, {49.891108, 49.890850, 49.890837, 49.891099}, {-119.482801, -119.482802, -119.483241, -119.483237}},
  {"knox Mtn", 4, {49.906499, 49.947743, 49.947437, 49.896545}, {-119.505323, -119.465053, -119.428319, -119.468466}},
  {"pebble Beach", 4, {49.802095, 49.806502, 49.808501, 49.806057}, {-119.657115, -119.655268, -119.641431, -119.638582}},
  {"Pandosy house", 4, {49.877727, 49.877966, 49.877966, 49.877727}, {-119.491452, -119.491452, -119.491000, -119.491000}}
};
float testLat;
float testLng;

int areaIndex = 100;
int lastAreaIndex = 100;

float verticesX[64];
float verticesY[64];

int numberVertices;
boolean nothingPlaying = true;

unsigned long timer = -5000;

boolean coordinatesFound = false;
boolean newCoordinatesFound = false;

// for testing set injection mode to true, this will bypass the gps data and use the custom set data below
boolean injectionMode = false;
float injectionLat =  49.890938;
float injectionLng = -119.483102;


int numberOfAreas;  // stores the total number of areas that the alorythm must loop through, no need to update
areaData currentArea;  // define structure to use in sram

void setup()
{
  Serial.begin(9600);

  // initialize mp3 first
  mp3.begin();
  mp3.setSynchronous(false);  // dont want to interrupt gps messages (only one softSer can listen at a time)
  mp3.volume(mp3.volumeMax()/2);  // set the volume to half for now, may incorperate pot in the future depending on case

  // then initialize GPS because the last initialized serial port will be listening, and we need to listen for GPS data right away
  ss.begin(GPSBaud);
  numberOfAreas = sizeof(areaDataArray) / sizeof(areaDataArray[0]);

}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0 && (millis() - timer) > 1000)  // gets number of bytes available to read from serial port
    if (gps.encode(ss.read()))
      displayGPSInfo();

  if (newCoordinatesFound) {
    searchGPSData();
  }

  mp3.check();        // run the mp3 receiver - repeadedly call to process device messages
  if ( (areaIndex != lastAreaIndex) || nothingPlaying){
    if(areaIndex == 0) mp3.playTrack(1);  // egypt
    else if(areaIndex == 1) mp3.playTrackRepeat(2);  // france
    else if(areaIndex == 2) mp3.playTrackRepeat(3);  // india
    else if(areaIndex == 3) mp3.playTrackRepeat(4);  // israel
    else if(areaIndex == 4) mp3.playTrackRepeat(5);  // jordan
    else if(areaIndex == 5) mp3.playTrackRepeat(6);  // morocco
    else if(areaIndex == 6) mp3.playTrackRepeat(7);  // napal
    else if(areaIndex == 7) mp3.playTrackRepeat(8);  // turkey
    else if(areaIndex == 8) mp3.playTrackRepeat(0);  // hospital block
    else if(areaIndex == 9) mp3.playTrackRepeat(1);  // KLO Creek
    else if(areaIndex == 10) mp3.playTrackRepeat(2); // lake ave block
    else if(areaIndex == 11) mp3.playTrackRepeat(1); // ethel house
    else if(areaIndex == 12) mp3.playTrackRepeat(4); // knox
    else if(areaIndex == 13) mp3.playTrackRepeat(3); // pebble beach
    else if(areaIndex ==14) mp3.playTrackRepeat(2); //pandosy House
    nothingPlaying = false;
  }
  if (millis() > 10000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}


void searchGPSData() {
  //search array of gps polygons! Here we try to find if we are within a defined area.
  boolean areaMatch = false;
  lastAreaIndex = areaIndex;
  // iterate through all the defined gps areas one by one
    for ( int i = 0; i < numberOfAreas; i++) {
    PROGMEM_readAnything(&areaDataArray[i], currentArea);
    numberVertices = currentArea.numVertices;
    memcpy(verticesX, currentArea.verticesX, sizeof(areaDataArray[i].verticesX[i]) * 64);
    memcpy(verticesY, currentArea.verticesY, sizeof(areaDataArray[i].verticesY[i]) * 64);
    int result = pnpoly(numberVertices, verticesX, verticesY, testLat, testLng);
    if (result == 1) {
      Serial.print(" // inside ");
      for (int j = 0; j < 24; j++) {
        Serial.print(currentArea.areaName[j]);
      } Serial.println(" // ");
      areaIndex = i; 
      areaMatch = true;
      break;
    }
  }
  // if we cant find a polygon which we are inside we must be in the upside down...
  if (!areaMatch) Serial.println("in nowhereland!");
  newCoordinatesFound = false;  // reset flag so this algorythm doesnt continuously run
}

void displayGPSInfo()
{
  if (gps.location.isValid())
  {
    // coordinates found!
    if (injectionMode) {
      testLng = injectionLng;
      testLat = injectionLat;
      Serial.print(injectionLng, 6);
      Serial.print(F(","));
      Serial.print(injectionLat, 6);
    } else {
      Serial.print(gps.location.lat(), 6);
      Serial.print(F(","));
      Serial.print(gps.location.lng(), 6);
      testLat = gps.location.lat();
      testLng = gps.location.lng();
    }
    coordinatesFound = true;
    newCoordinatesFound = true;
  }
  else {
    Serial.println(F("Searching for location..."));
    coordinatesFound = false;
    newCoordinatesFound = false;
  }
  timer = millis();
  
}



//run the point inclusion polygon test usiing the jordan curve theorem
//nvert =  Number of vertices in the polygon
//vertx verty = arrays containing the x and y coordinates of the polygons vertices
//testLat, testLng = x and y coordinates of the test point
int pnpoly(int nvert, float vertx[], float verty[], float testLat, float testLng)
{
  int i, j, c = 0;
  for (i = 0, j = nvert - 1; i < nvert; j = i++) {
    if ( ((verty[i] > testLng) != (verty[j] > testLng)) &&
         (testLat < (vertx[j] - vertx[i]) * (testLng - verty[i]) / (verty[j] - verty[i]) + vertx[i]) )
      c = !c;
  }
  return c;
}
