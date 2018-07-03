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

int position_y = 4;  //"lines"
int position_x = 20; //"column"
int clicks     = 0;
int no_clicks  = 0;
int writed     = 0;

//INICIAL SET
void setup(void) {

  Serial.begin(9600);
  yield();

  tft.begin();

  if (!ts.begin()) {
    Serial.println("PI:TS NOK");
    while (1);
  }
  Serial.println("PI:TS OK");

  printWhiteArea();
  printDotLine();
  Serial.println("PI:ARDUINO OK");
}


void loop() {

  //LONG DETECTED
  if (clicks >= 3) {
    if (!writed) {
      printDeleteBtn(ILI9341_RED);
    }
    writed = 1;
    printLine(ILI9341_WHITE);

    no_clicks = 0;
    clicks = 0;

    delay(250);
    while (!ts.bufferEmpty()) {
      ts.getPoint();
    }
    return;
  }
  //SPACE DETECTED AFTER SMALL TOUCH
  else if (no_clicks >= 3 && clicks > 0) {
    if (!writed) {
      printDeleteBtn(ILI9341_RED);
    }
    writed = 1;
    printBall(ILI9341_WHITE);

    no_clicks = 0;
    clicks = 0;

    delay(50);
    while (!ts.bufferEmpty()) {
      ts.getPoint();
    }
    return;
  }
  //NEW LINE
  else if (no_clicks >= 30 && clicks == 0 && writed == 1) {
    writed = 0;
    no_clicks = 0;
    position_y += 2;
    position_x = WIDTH;
    printDotLine();
    printDeleteBtn(ILI9341_BLACK);
  }

  if (ts.bufferEmpty()) {
    no_clicks++;
    delay(85);
    return;
  }

  TS_Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  if (p.x > 0 && p.x < BOXSIZE) {
    if (p.y > TS_X - WIDTH * 4 + 10) {
      no_clicks++;
      delay(50);
      deleteLine();
    } else if (p.y < TS_X - WIDTH * 4) {
      clicks++;
      no_clicks = 0;
      delay(50);
    }
    while (!ts.bufferEmpty()) {
      ts.getPoint();
    }
  }
}


/*
   Print the white box that accpets touches
   just print at the beginning
*/
void printWhiteArea() {
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, BOXSIZE, 240, ILI9341_WHITE);

  tft.setRotation(1);

  tft.setCursor(WIDTH, 185);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.println("TOUCH HERE");

  tft.setCursor(WIDTH, 0);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  tft.println("MORSE");


  tft.setCursor(WIDTH + 70, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("CODE");

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
    writed = 0;
  }
}


/*
   Print the delete button
*/
void printDeleteBtn(int color) {
  tft.fillRect(0, 240, BOXSIZE, 80, color);

  if (color == ILI9341_RED) {
    tft.setRotation(1);

    tft.setCursor(245, 185);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("DELETE");

    tft.setRotation(0);
  }
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
  if (position_y >= 14) {
    position_y = 4;
    tft.fillRect(BOXSIZE, WIDTH, TS_Y - BOXSIZE - 20, TS_X, ILI9341_BLACK);
  }
  tft.fillRect(BOXSIZE, 0, TS_Y - BOXSIZE, WIDTH, ILI9341_BLACK); //clean the area
  tft.fillRect(TS_Y - position_y * HEIGHT, 0, HEIGHT + 2, WIDTH / 4, ILI9341_RED); //print a dot in the current line
}


/*
   Analyze the number of touches
   and if it's a new symbol (new line when 3 or more time intervals with no touches)
*/
void analyze() {

  if ((position_x > TS_X - WIDTH)) {
    position_y += 2;
    position_x = WIDTH;
    no_clicks  = 0;
    writed = 0;
    printDeleteBtn(ILI9341_BLACK);
    printDotLine();
  }

  if (position_y >= 14) {
    position_y = 4;
    tft.fillRect(BOXSIZE, WIDTH, TS_Y - BOXSIZE - 20, TS_X, ILI9341_BLACK); //clean the area with old symbols
    printDotLine();
  }

}


