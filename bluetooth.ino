//This project uses an arduino MEGA 2560, 3 DC water pumps, 2 L298N dual-H Bridge motor controllers and several momentary push buttons as well as a single latched momentray push button.
//Power is supplied with a 12V power adapter whose cathode and anode cables were stripped to connect with L298N modules as well as the arduino via "vin" pin.
//Disclaimer: As you will see throughout my code, I have created sub-cycles that are either exactly 1 minute long or add up to 1 minute when called in conjunction with another sub-cycle
//I am not planning on using millis() to call methods / calculate ETA with "pseudothreading" yet. I might consider doing so in the future
#include "pitches.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 20
Adafruit_SSD1306 display(OLED_RESET);
int temperature = 20; //default temperature for all programs, it's faux the machine does not have a heating element yet.
int power_counter = 0;// optional pointer for future sequential controlling
int speaker_out = 48;  //49(previous pin)
int power_in = 17;//not used at the moment
int spin_speed = 1200;// default spin speed
int delay_start = 0;// default delay start, the program starts immediately
int motor2pin1 = 6;// pin 1 to be connected to L298N for spinning the drum
int motor2pin2 = 4;// pin 2 to be connected to L298N for spinning the drum
int powerLED = 3;// illumination of the area surrounding program selector dial ring
int fadeAmount = 5;// not used at the moment, future plans include strobing the program selector dial ring light
int temp_in = 14;// input pin from the push button for selecting the temperature (faux, the project does not include a heating element at the moment)
int spin_in = 15;// input pin from the push button for selecting the spin speed(spin functions will ramp up the motor according to the selection)
int delay_in = 16;// input pin from the push button for selecting delayed start (ranging from 0{no delay} to 10 minutes)
int drain_pump1 = 7;// pin 1 to be connected to L298N for draining water(which pin of the drain motor you connect this to does not matter)
int drain_pump2 = 8;// pin 2 to be connected to L298N for draining water(which pin of the drain motor you connect this to does not matter)
int cycle_pump1 = 11;// pin 1 to be connected to L298N for circulating water(which pin of the drain motor you connect this to does not matter)
int cycle_pump2 = 12;// pin 2 to be connected to L298N for circulating water(which pin of the drain motor you connect this to does not matter)
int fill_pump1 = 30;// pin 1 to be connected to L298N for filling water(which pin of the fill motor you connect this to does not matter)
int fill_pump2 = 31;// pin 2 to be connected to L298N for filling water(which pin of the drafillin motor you connect this to does not matter)
int pot = 0;// variable for potentiometer(for selecting programs by turning the program selector dial)
bool program_started = false;// initial boolean variable representing whether the cycle has started or not
int temp_counter = 0;// pointer to display the currently selected temperature (in celcius)
int spin_counter = 0;// pointer to display the currently selected spin speed (in rpm -revolutions per minute-)
int delay_counter = 0;// pointer to display the currently selected timed delay (in minutes)
int end_jingle[] = {
  NOTE_D6, NOTE_A5, NOTE_E6, NOTE_A6, 0, NOTE_E6, NOTE_D6, NOTE_A5, NOTE_A5, NOTE_E6, NOTE_A6
};//tone to be played at the end of a wash cycle
int noteDurations[] = {
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};//integer assignment for how long to play each tone
int start_jingle[] = {
  NOTE_D6, NOTE_A5, NOTE_A6
};//tone to be played when the machine is first turned on
int play_jingle[] = {
  NOTE_D6, NOTE_A6
};//tone to be played when the start/stop button is pressed to start the wash cycle
int off_jingle[] = {
  NOTE_E6, NOTE_A6, NOTE_A5
};//tone to be played when the machine is powered off (currently not being used)
int jingleDurations[] = {
  4, 4, 4, 4, 4
};//integer assignment for how long to play each tone
int temps[] = {
  20, 30, 40, 60
};//array for available temperature selections
int spinspeeds[] = {
  600, 1000, 1200
};//array for available spin speeds
int delays[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};//array for available delay start

//'bosch_logo', 128x34px
const unsigned char myBitmap[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xf0, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0xc0, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x90, 0x04, 0xe0, 0x00, 0x0f, 0xf8, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x3f, 0x01, 0xc0, 0x50,
  0x07, 0x30, 0x0e, 0x70, 0x00, 0x0f, 0xfe, 0x01, 0xff, 0x80, 0xff, 0x80, 0xff, 0xc3, 0xc0, 0xf8,
  0x06, 0x70, 0x0f, 0x30, 0x00, 0x0f, 0xff, 0x03, 0xff, 0xc1, 0xff, 0xc1, 0xff, 0xe3, 0xc0, 0xf8,
  0x0c, 0xf0, 0x0f, 0xb8, 0x00, 0x0f, 0xff, 0x87, 0xff, 0xc3, 0xf7, 0xc1, 0xff, 0xe3, 0xc0, 0xf0,
  0x0c, 0xf0, 0x0f, 0x98, 0x00, 0x0f, 0x0f, 0x87, 0xc3, 0xe3, 0xc1, 0x83, 0xe1, 0xc3, 0xc0, 0xf0,
  0x19, 0xff, 0xfd, 0x98, 0x00, 0x0f, 0x07, 0x8f, 0x81, 0xe3, 0xc0, 0x03, 0xc0, 0x83, 0xc0, 0xf0,
  0x19, 0xbf, 0xfd, 0xc8, 0x00, 0x0f, 0x07, 0x8f, 0x01, 0xe3, 0xe0, 0x07, 0xc0, 0x03, 0xe0, 0xf0,
  0x19, 0xb0, 0x0c, 0xcc, 0x00, 0x0f, 0x9f, 0x0f, 0x00, 0xf3, 0xfc, 0x07, 0x80, 0x03, 0xff, 0xf0,
  0x19, 0xb0, 0x0c, 0xcc, 0x00, 0x0f, 0xfe, 0x0f, 0x00, 0xf1, 0xff, 0x87, 0x80, 0x03, 0xff, 0xf0,
  0x1b, 0xb0, 0x0c, 0xcc, 0x00, 0x0f, 0xff, 0x0f, 0x00, 0xf0, 0xff, 0xc7, 0x80, 0x03, 0xff, 0xf0,
  0x19, 0xb0, 0x0c, 0xcc, 0x00, 0x0f, 0xff, 0x8f, 0x00, 0xf0, 0x1f, 0xe7, 0x80, 0x03, 0xff, 0xf0,
  0x19, 0xbf, 0xfc, 0xcc, 0x00, 0x0f, 0x07, 0xcf, 0x01, 0xf0, 0x03, 0xe7, 0x80, 0x03, 0xc0, 0xf0,
  0x19, 0xbf, 0xfd, 0x88, 0x00, 0x0f, 0x07, 0xcf, 0x81, 0xe0, 0x01, 0xe3, 0xc0, 0x03, 0xc0, 0xf0,
  0x1d, 0xf0, 0x0d, 0x98, 0x00, 0x0f, 0x07, 0xc7, 0x83, 0xe1, 0x81, 0xe3, 0xe0, 0xc3, 0xc0, 0xf0,
  0x0c, 0xf0, 0x0f, 0x98, 0x00, 0x0f, 0x0f, 0xc7, 0xff, 0xc3, 0xc3, 0xe3, 0xff, 0xe3, 0xc0, 0xf0,
  0x0e, 0x70, 0x0f, 0x30, 0x00, 0x0f, 0xff, 0x83, 0xff, 0xc3, 0xff, 0xc1, 0xff, 0xe3, 0xc0, 0xf0,
  0x06, 0x30, 0x0e, 0x70, 0x00, 0x0f, 0xff, 0x01, 0xff, 0x81, 0xff, 0x80, 0xff, 0xc3, 0xc0, 0xf0,
  0x03, 0x10, 0x0c, 0x60, 0x00, 0x0f, 0xfe, 0x00, 0xfe, 0x00, 0xff, 0x00, 0x3f, 0x83, 0xc0, 0xf0,
  0x01, 0xc0, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x7c, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};//array which stores BOSCH logo to be shown upon startup
void setup() {
  pinMode(fill_pump1, OUTPUT);
  pinMode(fill_pump2, OUTPUT);
  pinMode(power_in, INPUT);
  pinMode(delay_in, INPUT);
  pinMode(spin_in, INPUT);
  pinMode(temp_in, INPUT);
  pinMode(A0, INPUT);
  pinMode(10, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(drain_pump1, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(drain_pump2, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//this depends on your particular OLED panel. check your particular models instructions to assign the correct memory address
  display.clearDisplay();//clears up any possible visual artifacts during the setup process
}

void loop() {
  display.clearDisplay();
  while (true) {
    //!program_started
    if (program_started) {
      display.clearDisplay();
      robojaxText("Done", 20, 0, 4, false);
      display.display();
      display.clearDisplay();
      display.display();
      delay(500);
      robojaxText("Done", 20, 0, 4, false);
      display.display();
      display.clearDisplay();
      delay(500);
      display.display();
      robojaxText("Done", 20, 0, 4, false);
      display.display();
    } else {
      pot = 120;
      if (pot == 120) {
        display.clearDisplay();
      display.drawBitmap(0, 0, myBitmap, 128, 32, WHITE);
      display.display();
      for (int thisNote = 0; thisNote < 3; thisNote++) {
        int noteDuration = 1000 / jingleDurations[thisNote];
        tone(speaker_out, start_jingle[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(speaker_out);
      }
        display.clearDisplay();
        robojaxText("Cottons", 86, 25, 1, false);
        robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
        robojaxText((getTemp(tempSelect()) + "   " + getSpin(spinSelect()) + "    " + String(getETA(delaySelect()) + "min")), 1, 11, 1, false);
        robojaxText(" v      v ", 0, 24, 1, true);
        //program_started = true;
        if (true) {
          Cottons();
        }
      }
    }
  }
  if (!program_started) {
    display.clearDisplay();
    display.drawBitmap(0, 0, myBitmap, 128, 32, WHITE);
    display.display();
    for (int thisNote = 0; thisNote < 3; thisNote++) {
      int noteDuration = 1000 / jingleDurations[thisNote];
      tone(speaker_out, start_jingle[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(speaker_out);
    }
    program_started = true;
    display.clearDisplay();
    display.display();
  }
}

void initial_drain() {
  digitalWrite(drain_pump2, HIGH);
  delay(5000);
  digitalWrite(drain_pump2, LOW);
  delay(2000);
}

void fill() {
  digitalWrite(fill_pump1, HIGH);
  digitalWrite(fill_pump2, LOW);
  delay(60000);
  digitalWrite(fill_pump1, LOW);
  digitalWrite(fill_pump2, LOW);
}

void regular_wash() {
  digitalWrite(cycle_pump1, HIGH);
  digitalWrite(cycle_pump2, LOW);
  // 1 minute
  for (int i = 0; i <= 1; i++) {
    analogWrite(10, 55);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    delay(10000);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
    delay(5000);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, HIGH);
    delay(10000);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
    delay(5000);
  }
  digitalWrite(cycle_pump1, LOW);
  digitalWrite(cycle_pump2, LOW);
}

void uni_toss_wash() {
  for (int i = 0; i <= 12; i++) {
    analogWrite(10, 140);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    delay(350);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, HIGH);
    delay(50);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
    delay(150);
  }
  delay(4000);
}

void bi_toss_wash() {
  for (int i = 0; i <= 11; i++) {
    analogWrite(10, 120);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    delay(450);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
    delay(100);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, HIGH);
    delay(420);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
    delay(100);
  }
  delay(4000);
}

void drain() {
  digitalWrite(drain_pump1, HIGH);
  digitalWrite(drain_pump2, LOW);
  delay(10000);
  analogWrite(10, 68);  //ENB pin
  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  delay(2000);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  delay(3000);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, HIGH);
  delay(2000);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  delay(3000);
  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  delay(2000);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  delay(3000);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, HIGH);
  delay(2000);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  delay(3000);
  digitalWrite(drain_pump1, LOW);
  digitalWrite(drain_pump2, LOW);
}
// 1 minute with drain();
void interim_spin() {
  digitalWrite(drain_pump1, HIGH);
  digitalWrite(drain_pump2, LOW);
  for (int i = 40; i <= 255; i++) {
    analogWrite(10, i);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    delay(100);
  }
  delay(19900);
  for (int i = 1; i <= 255; i++) {
    analogWrite(10, 255 - i);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
    delay(100);
  }
  digitalWrite(drain_pump1, LOW);
  digitalWrite(drain_pump2, LOW);
}

void final_spin_speed_up() {
  int spin = spinspeeds[spin_counter];
  digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
  for (int i = 80; i <= 180; i++) {
    analogWrite(10, i);
    delay(100);
  }
  analogWrite(10, 181);
  delay(59000);
}
void final_spin() {
  digitalWrite(drain_pump1, HIGH);
  digitalWrite(drain_pump2, LOW);
  if (true) {
    for (int i = 181; i <= 255; i++) {
      analogWrite(10, i);
      delay(150);
    }

  } else if (spin_counter == 0) {
    for (int i = 182; i <= 210; i++) {
      analogWrite(10, i);
      delay(150);
    }
  } else if (spin_counter == 1) {
    for (int i = 182; i <= 220; i++) {
      analogWrite(10, i);
      delay(150);
    }
  }
}
void final_spin_slow_down() {
  if (true) {
    for (int i = 0; i <= 255; i++) {
      analogWrite(10, 255 - i);
      delay(120);
    }
  } else if (spin_counter == 0) {
    for (int i = 0; i <= 210; i++) {
      analogWrite(10, 255 - i);
      delay(120);
    }
  } else if (spin_counter == 1) {
    for (int i = 0; i <= 220; i++) {
      analogWrite(10, 255 - i);
      delay(120);
    }
  }
  digitalWrite(drain_pump1, LOW);
  digitalWrite(drain_pump2, LOW);
}

void end_program() {
  for (int i = 0; i <= 50; i++) {
    digitalWrite(i, LOW);
  }
  for (int i = 0; 0 < 1; i++) {
    for (int thisNote = 0; thisNote < 19; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(speaker_out, end_jingle[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration;
      delay(pauseBetweenNotes);
      noTone(speaker_out);
    }
    display.clearDisplay();
  }
  program_started = false;
}
void Cottons() {
  for (int thisNote = 0; thisNote < 2; thisNote++) {
    int noteDuration = 1000 / jingleDurations[thisNote];
    tone(speaker_out, play_jingle[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 0.8;
    delay(pauseBetweenNotes);
    noTone(speaker_out);
  }
  if (getETA(delaySelect()) == "0") {
    display.clearDisplay();
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("[            ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "12" + "min")), 1, 11, 1, true);
    fill();

    display.clearDisplay();
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("[=           ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "11" + "min")), 1, 11, 1, true);

    regular_wash();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[==          ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "10" + "min")), 1, 11, 1, true);

    regular_wash();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[===         ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "9" + "min")), 1, 11, 1, true);

    drain();
    interim_spin();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[====        ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "8" + "min")), 1, 11, 1, true);

    fill();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[=====       ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "7" + "min")), 1, 11, 1, true);

    regular_wash();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[======      ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "6" + "min")), 1, 11, 1, true);

    regular_wash();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[=======     ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "5" + "min")), 1, 11, 1, true);

    drain();
    interim_spin();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[========    ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "4" + "min")), 1, 11, 1, true);

    final_spin_speed_up();
    final_spin();
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[=========   ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "3" + "min")), 1, 11, 1, true);
    delay(60000);
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[==========  ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "2" + "min")), 1, 11, 1, true);
    delay(60000);
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[=========== ]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "1" + "min")), 1, 11, 1, true);
    delay(60000);
    display.clearDisplay();
    robojaxText(" ^      ^      ^    +", 0, 1, 1, false);
    robojaxText("Cottons", 86, 25, 1, false);
    robojaxText("[============]", 0, 25, 1, false);
    robojaxText(((getTemp(tempSelect()) + "    " + getSpin(spinSelect()) + "   " + "0" + "min")), 1, 11, 1, true);
    final_spin_slow_down();
    end_program();
    program_started = false;
  }
}
void robojaxText(String text, int x, int y, int size, boolean d) {

  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(text);
  if (d) {
    display.display();
  }
  //delay(100);
}
String getTemp(int selected_temp) {
  return "20C";
}
String getSpin(int selected_spin) {
  return "1200@";
}

String getETA(int selected_delay) {
  return String(delays[selected_delay]);
}
int tempSelect() {
  if (digitalRead(temp_in) == HIGH) {
    temp_counter++;
  }
  if (temp_counter > 4) {
    temp_counter = 0;
  }
  return temp_counter;
}
int spinSelect() {
  if (digitalRead(spin_in) == HIGH) {
    spin_counter++;
  }
  if (spin_counter > 3) {
    spin_counter = 0;
  }
  return spin_counter;
}
int delaySelect() {
  if (digitalRead(delay_in) == HIGH) {
    return 0;
  }
  if (delay_counter > 10) {
    delay_counter = 0;
  }
  return delay_counter;
}
