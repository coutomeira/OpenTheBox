#include <Adafruit_GFX.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define SD_CS 4
// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define BOXSIZE 100
#define TS_X 320
#define TS_Y 240
#define HEIGHT 10
#define WIDTH 20
#define BUFFPIXEL 20

int position_y  = 4;  //lines
int position_x  = 20; //"column"
int clicks      = 0;
int no_clicks   = 0;
int loops       = 0;
int no_input    = 0; //when a interval time have no input
int writed      = 0;

//INICIL SET
void setup(void) {

  Serial.begin(9600);
  yield();

  tft.begin();

  if (!ts.begin()) {
    Serial.println("PI:TS NOK");
    while (1);
  }
  Serial.println("PI:TS OK");

  printSquare();
  printDotLine();
  printDeleteBtn(ILI9341_BLACK);
  Serial.println("PI:ARDUINO OK");
}


void loop() {

  if (loops == 10) {

    Serial.println();

    delay(500);
    printClock(0);

    Serial.print("Clicks      ->"); Serial.println(clicks   );
    Serial.print("No Clicks   ->"); Serial.println(no_clicks);
    Serial.println("");

    if (clicks == 0) {
      no_input++;
      analyze();
    } else {
      if (clicks < 5) {
        writed = 1;
        no_input = 0;
        printBall(ILI9341_WHITE);
        printDeleteBtn(ILI9341_RED);
      } else {
        writed = 1;
        no_input = 0;
        printLine(ILI9341_WHITE);
        printDeleteBtn(ILI9341_RED);
      }
    }

    no_clicks = 0;
    clicks = 0;
    loops = 0;

    while (!ts.bufferEmpty()) {
      ts.getPoint();
    }
    return;
  }

  loops++;
  printClock(1);

  if (ts.bufferEmpty()) {
    no_clicks++;
    delay(85);
    return;
  }

  TS_Point p = ts.getPoint();

  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  Serial.print("x: "); Serial.println(p.x);
  Serial.print("y: "); Serial.println(p.y);

  if (p.x > 0 && p.x < BOXSIZE) {
    Serial.println(TS_X - WIDTH * 4);
    Serial.println(TS_X - WIDTH * 4 + 10);

    if (p.y > TS_X - WIDTH * 4 + 10) {
      Serial.println("Entrei no primeiro");
      no_clicks++;
      delay(50);
      deleteLine();
      while (!ts.bufferEmpty()) {
        ts.getPoint();
      }
    } else if (p.y < TS_X - WIDTH * 4) {
      Serial.println("Entrei no segundo");
      clicks++;
      if (clicks == 5)
        delay(50);
      while (!ts.bufferEmpty()) {
        ts.getPoint();
      }
    }
  }
}


/*
   Print the yellow box that accpets touchs
*/
void printSquare() {
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, BOXSIZE, TS_X - WIDTH * 4, ILI9341_YELLOW);

  tft.setRotation(1);

  tft.setCursor(60, 185);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("TOUCH HERE");

  tft.setRotation(0);
}


/*
  Delete the current sequence of symbols
*/
void deleteLine() {
  if (writed) {
    tft.fillRect(TS_Y - position_y * HEIGHT, WIDTH, HEIGHT + 2, TS_X - WIDTH, ILI9341_BLACK);
    printDeleteBtn(ILI9341_BLACK);
    position_x = WIDTH;
    no_input = 0;
    writed = 0;
  }

}


/*
   Print the delete button
*/
void printDeleteBtn(int color) {
  tft.fillRect(0, TS_X - WIDTH * 4 + 10, BOXSIZE, WIDTH * 4, color);
}


/*
   Printing the bar in top that gives each time interval
*/
void printClock(int flag) {
  if (flag) //printing the bar during the time
    tft.fillRect(TS_Y - 10, TS_X / 2 - 75 + (loops - 1) * 15, HEIGHT, 15, ILI9341_WHITE);
  else //reset the bar
    tft.fillRect(TS_Y - 10, TS_X / 2 - 75, HEIGHT, 149, ILI9341_BLACK);
}


/*
   Print line symbol
*/
void printBall(int color) {
  tft.fillCircle(TS_Y - position_y * HEIGHT + 5, position_x + HEIGHT / 2, HEIGHT / 2, color);
  position_x = position_x + 15;
  analyze();
}


/*
   Print ball symbol
*/
void printLine(int color) {
  tft.fillRect(TS_Y - position_y * HEIGHT + 3, position_x, HEIGHT / 2, HEIGHT * 2, color);
  position_x = position_x + 23;
  analyze();
}


/*
   Print dot in the current line
*/
void printDotLine() {
  tft.fillRect(BOXSIZE, 0, TS_Y - BOXSIZE, WIDTH, ILI9341_BLACK); //clean the area
  tft.fillRect(TS_Y - position_y * HEIGHT + 3, WIDTH / 2, 2, 2, ILI9341_YELLOW); //print a dot in the current line
}


/*
   Analyze the number of touches
   and if it's a new symbol (new line when 3 or more time intervals with no touches)
*/
void analyze() {

  if ((writed && no_input >= 3) || (position_x > TS_X - WIDTH)) {
    position_y += 2;
    position_x = WIDTH;
    no_clicks  = 0;
    writed = 0;
    printDeleteBtn(ILI9341_BLACK);

    if (position_y >= 14) {
      position_y = 4;
      tft.fillRect(BOXSIZE, 20, TS_Y - BOXSIZE, TS_X, ILI9341_BLACK); //clean the area with old symbols
    }
    printDotLine();
  }
}


