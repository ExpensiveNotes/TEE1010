/* Testing Hardware for TEE1010
   By John Melki-Wegner (aka Expensive Notes)

*/
// === MIDI ==================================================================== MIDI
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial7, MIDI);
int channel = 1;

// === Variables =============================================================== Variables

int reading;           // the current reading from the input pin
//Pots :  Analogue pins
const int potPin[] = {A0, A8, A14, A15, A16, A17};   // Input pins from the potentiometers
int potVals[6]; //input values from pots

const int foot[] = {0, 1, 2, 3, 4, 5, 30, 31, 32, 33, 36, 37, 26, 27, 34}; // Note that the last five positions represent footUp, footDown and the three additional switches respectively
const int additionalSwitch[] = {26, 27, 34};

const char footPrint[][12] = {" foot1 ", " foot2 ", " foot3 ", " foot4 ", " foot5 ", " foot6 ", " foot7 ", " foot8 ", " foot9 ", " foot10 ", " footUp ", " footDown ", " addSwitch1 ", " addSwitch2 ", " addSwitch3 "};

// #define numberSwitches 15 // This appears to be unused, just commenting up

//=== Oled Screen ================================================================ Oled Screen
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # 
#define SCREEN_ADDRESS 0x3C // Found using scanner

//Changed Wire to Wire2 for SCL2 and SDA2 Yay!
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, OLED_RESET);

// === MIDI ========================================================================= MIDI
byte stopMIDI = 0xfc; //MIDI stop
byte midi_clock = 0xf8;
int note = 60;
int BPM = 250;//Beats per Minute maximum?
int waitTime = 40; //Time between sending MIDI clock
int spareTime = 40; //Time left after processes are done that needs to be padded/wasted while waiting for mext beat/note

// === Setup ======================================================================== Setup

void setup()
{
  //USB Serial for debugging
  Serial.begin(9600);

  for (int cFoot : foot) pinMode(cFoot, INPUT_PULLUP); // Sets every pin in foot to

  //MIDI Serial in and out
  Serial7.begin(31250); //MIDI baud rate
  Serial7.write(stopMIDI); // just in case

  MIDI.begin(MIDI_CHANNEL_OMNI);                      // Launch MIDI and listen to channel 1

  //--- Screen ------------------------------------------------
  //   SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
}

// === Loop ======================================================================== Loop

void loop()
{
  int t1 = millis();
  readPots();
  readSwitches();
  sendMIDInote();
  showScreen();

  int t2 = millis();
  Serial.print("\tTime to Read Switches etc. :\t");
  Serial.print(t2 - t1);
  Serial.print("\tSpare time:\t");
  spareTime = waitTime - (t2 - t1);
  Serial.print(spareTime);
  Serial.print("\tMIDI in?? ");
  while (t2 - t1 < waitTime) {
    if (MIDI.read()) {
      byte type = MIDI.getType();
      Serial.print("\t");
      Serial.print(type);
    }
    t2 = millis();
  }
  Serial7.write(midi_clock);
  Serial.print("\t");
  Serial.println();
}

// === ============================================================================

void showScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(15, 0);
  display.println(BPM);
  display.setCursor(15, 20);
  display.println(spareTime);
  display.display();
}

void sendMIDInote() {
  MIDI.sendNoteOff(note, 100, channel);
  note++;
  if (note > 65) note = 60;
  MIDI.sendNoteOn(note, 100, channel);
}

void readPots() {
  for (int i = 0; i < 6; i++) {
    potVals[i] = analogRead(potPin[i]);
    Serial.print(potPin[i]);
    Serial.print("=");
    Serial.print(potVals[i]);
    Serial.print("  ");
  }
  BPM = map( potVals[0] , 0, 1023, 20, 250);
  waitTime = int(60000 / BPM / 6); //Time between MIDI clock out
}

void readSwitches() {
  for (int iter = 0; iter < 16; iter++) if (digitalRead(foot[iter]) == LOW) Serial.print(footPrint[iter]);
}