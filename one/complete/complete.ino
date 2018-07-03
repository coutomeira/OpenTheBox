#include <Adafruit_GFX.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define SD_CS 4
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//MORSE
#define WHITEHEIGHT 100
#define TS_X 320
#define TS_Y 240
#define HEIGHT 10 //Height of ball
#define WIDTH 20  //Width of line (long symbol)
#define BUFFPIXEL 20

//PIN
#define BOXSIZEX 70
#define BOXSIZEY 70
#define SMALLBOXSIZE 10
#define INTERVAL 5

//INTEGRATION
#define BOXSIZE 100

//MORSE
int position_y = 4;  //"lines"
int position_x = 20; //"column"
int clicks     = 0;
int no_clicks  = 0;
int writed     = 0;
char morseCode[100];
int index      = 0;

//PIN
int  numSquare    ; //The square where user touched
int  counter   = 0; //How many digits
long code      = 0; //Concatenation of digits

//INTEGRATION
int actualMode = 0; //0-Menu,1-Pin,2-Morse,3-QR
int opened     = 0;
int waiting    = 0;
int blocked    = 0;
int y          = 0; //Animation Line
int colorAnim  = ILI9341_RED; //Animation

//READ MODE
#define size 50
char inData[size]; // Allocate some space for the string
char inChar = -1;  // Where to store the character read
byte indexRead = 0;// Index into array; where to store the character 'inChar'
bool final = false;// Indicates if a 'X' was read as the end of the message


/********************************************
 * **************************************** *
 * **************************************** *
              IMPLEMENTATION
 * **************************************** *
 * **************************************** *
 ********************************************/


void setup(void) {

  Serial.begin(9600);
  yield();

  tft.begin();

  if (!ts.begin()) {
    //Serial.println("PI:TS:NOK");
    while (1); // BLOCK
  }
  //Serial.println("PI:TS:OK");
  //Serial.println("PI:ARDUINO:1:OK");

  setupIntegration();

}


void loop() {

  if (actualMode == 0) {
    loopIntegration();
  } else if (actualMode == 1) {
    loopPin();
  } else if (actualMode == 2) {
    loopMorse();
  } else if (actualMode == 3) {
    loopQR();
  }

}


/********************************************
 * **************************************** *
 * **************************************** *
             INTEGRATION   CODE
 * **************************************** *
 * **************************************** *
 ********************************************/


void setupIntegration() {

  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(70, 0               , BOXSIZE, BOXSIZE, ILI9341_WHITE);
  tft.fillRect(70, BOXSIZE + 10    , BOXSIZE, BOXSIZE, ILI9341_WHITE);
  tft.fillRect(70, BOXSIZE * 2 + 20, BOXSIZE, BOXSIZE, ILI9341_WHITE);

  tft.setRotation(1);

  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);

  tft.setCursor(30, 115);
  tft.println("PIN");

  tft.setCursor(130, 115);
  tft.println("MORSE");

  tft.setCursor(260, 115);
  tft.println("QR");

  tft.setRotation(0);

}


void loopIntegration() {

  if (Serial.available() > 0) {
    readMessage();
    return;
  }

  int tmpColor;
  if (opened) {
    tft.fillScreen(ILI9341_WHITE);
    tmpColor = ILI9341_GREEN;
  }

  while (opened == 1) {

    for (int i = 0; i < 17; i++) {

      if (tmpColor == ILI9341_GREEN)
        tmpColor = ILI9341_WHITE;
      else
        tmpColor = ILI9341_GREEN;


      tft.fillRect(0, i * 20, 240, 20, tmpColor);
      delay(500);

      if (Serial.available() > 0) {
        readMessage();
        if (!opened)
          break;
      }

    }
  }

  animationMenu();

  if (ts.bufferEmpty()) {
    return;
  }

  // Retrieve a point
  TS_Point p = ts.getPoint();

  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  if (p.x > 70 && p.x < BOXSIZE + 70) {
    if (p.y < BOXSIZE) {
      Serial.println("PI:MODE:PIN");
      setupPin();
      delay(500);
      while (!ts.bufferEmpty())
        ts.getPoint();
      actualMode = 1;
    } else if (p.y > BOXSIZE + 10 && p.y < BOXSIZE * 2 + 10) {
      Serial.println("PI:MODE:MORSE");
      setupMorse();
      delay(500);
      while (!ts.bufferEmpty())
        ts.getPoint();
      actualMode = 2;
    } else if (p.y > BOXSIZE * 2 + 20) {
      Serial.println("PI:MODE:QR");
      setupQR();
      delay(500);
      while (!ts.bufferEmpty())
        ts.getPoint();
      actualMode = 3;
    }
  }

}

void animationMenu() {

  if (y > 240) {
    y = 0;
    if (colorAnim == ILI9341_BLACK)
      colorAnim = ILI9341_RED;
    else
      colorAnim = ILI9341_BLACK;
  }
  if (y >= 70 && y <= 170) {
    tft.drawLine(y, 100, y, 109, colorAnim);
    tft.drawLine(y, 210, y, 219, colorAnim);
  }
  else
    tft.drawLine(y, 0, y, 320, colorAnim);
  y = y + 5;
  delay(250);

}


void backToMenu(int flag) {

  colorAnim = ILI9341_RED;
  y = 0;
  actualMode = 0;
  if (flag)
    setupIntegration();
  delay(500);
  while (!ts.bufferEmpty())
    ts.getPoint();

}


void waitingMsg() {

  tft.fillScreen(ILI9341_BLACK);
  colorAnim  = ILI9341_WHITE;
  y = 0;

  while (waiting) {
    animationWaiting();
    delay(250);
    if (Serial.available() > 0) {
      readMessage();
    }
  }

}


void animationWaiting() {

  tft.fillCircle(TS_Y / 2 + 5, 60 + y * 40, HEIGHT / 2, colorAnim);

  y++;
  if (y == 6) {
    y = 0;
    if (colorAnim == ILI9341_WHITE)
      colorAnim = ILI9341_RED;
    else
      colorAnim = ILI9341_WHITE;
  }

}


int blockedTft() {

  if (opened) return;

  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(3);
  tft.setCursor(10, 10);

  for (int i = 60; i >= 0; i--) {

    tft.setCursor(10, 10);
    tft.println(i);
    delay(1000);
    tft.fillRect(0, 0, 50, 50, ILI9341_BLACK);

    if (Serial.available() > 0) {
      readMessage();
    }

    if (blocked == 0) {
      tft.setRotation(0);
      return 0;
    }
  }

  tft.setRotation(0);
  blocked = 0;
  while (!ts.bufferEmpty())
    ts.getPoint();
  return 1;

}


/********************************************
 * **************************************** *
 * **************************************** *
                  QR  CODE
 * **************************************** *
 * **************************************** *
 ********************************************/


void setupQR() {

  tft.fillScreen(ILI9341_BLACK);

  tft.setRotation(1);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);

  tft.setCursor(0, 115);
  tft.println("set the    code");

  tft.setCursor(15, 145);
  tft.println("for the camera");

  tft.setTextSize(4);
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(140, 108);
  tft.println("QR");

  tft.setRotation(0);

}


void setupQR2() {

  tft.setRotation(1);

  tft.fillRect(135, 100, 55, 45, ILI9341_BLACK);
  delay(500);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(140, 108);
  tft.println("QR");

  tft.setRotation(0);

}


void loopQR() {

  if (Serial.available() > 0) {
    readMessage();
    return;
  }
  setupQR2();
  delay(500);

}


/********************************************
 * **************************************** *
 * **************************************** *
                  PIN   CODE
 * **************************************** *
 * **************************************** *
 ********************************************/


void setupPin() {

  counter = 0;
  code = 0;
  printSquares();
  printText();

}


void loopPin() {

  if (Serial.available() > 0) {
    readMessage();
    return;
  }

  if (ts.bufferEmpty()) {
    return;
  }

  // Retrieve a point
  TS_Point p = ts.getPoint();

  int isNumber = 0;

  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  if (p.y < BOXSIZEX) {
    if (p.x < BOXSIZEY) {
      numSquare = 7;
      drawSmallSquare(numSquare);
      printText();
      counter++;
      isNumber = 1;
    } else if (p.x > BOXSIZEY + INTERVAL && p.x < BOXSIZEY * 2 + INTERVAL) {
      numSquare = 4;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    } else if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
      numSquare = 1;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    }
  } else if (p.y > BOXSIZEX + INTERVAL && p.y < BOXSIZEX * 2 + INTERVAL) {
    if (p.x < BOXSIZEY) {
      numSquare = 8;
      drawSmallSquare(numSquare);
      printText();
      counter++;
      isNumber = 1;
    } else if (p.x > BOXSIZEY + INTERVAL && p.x < BOXSIZEY * 2 + INTERVAL) {
      numSquare = 5;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    } else if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
      numSquare = 2;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    }
  } else if (p.y > BOXSIZEX * 2 + INTERVAL * 2 && p.y < BOXSIZEX * 3 + INTERVAL * 2) {
    if (p.x < BOXSIZEY) {
      numSquare = 9;
      drawSmallSquare(numSquare);
      printText();
      counter++;
      isNumber = 1;
    } else if (p.x > BOXSIZEY + INTERVAL && p.x < BOXSIZEY * 2 + INTERVAL) {
      numSquare = 6;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    } else if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
      numSquare = 3;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    }
  } else if (p.y > BOXSIZEX * 3 + INTERVAL * 3 && p.y < BOXSIZEX * 4 + INTERVAL * 3) {
    if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
      numSquare = 0;
      drawSmallSquare(numSquare);
      counter++;
      isNumber = 1;
    }
  }
  if (p.y > BOXSIZEX * 3 + INTERVAL * 3) {
    if (p.x < 145) {
      if (counter > 0) {
        deleteSmallSquare();
        isNumber = 0;
      }
    }
  }
  if (p.y > BOXSIZEX * 4 + INTERVAL * 4) {
    if (p.x > 145 && p.x < 220) {
      if (counter > 0) {
        deleteSmallSquare();
        isNumber = 0;
      }
    }
  }

  if (isNumber) {
    code = (code * 10) + numSquare;

    if (counter == 4) {
      Serial.print("PI:PIN:"); Serial.print(code); Serial.print("\n");
      delay(500);
      waiting = 1;
      counter = 0;
      code = 0;

      waitingMsg();
    }
  }

  delay(125);
  while (!ts.bufferEmpty())
    ts.getPoint();

}


/*
   Print the number inserted
*/
void drawSmallSquare(int numSquare) {

  tft.fillRect(BOXSIZEY * 3 + INTERVAL * 4, 30 + (counter + counter - 1)*SMALLBOXSIZE + SMALLBOXSIZE, SMALLBOXSIZE, SMALLBOXSIZE + 1, ILI9341_WHITE);
  tft.setRotation(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setCursor(30 + 3  + (counter + counter - 1)*SMALLBOXSIZE + SMALLBOXSIZE, 1);
  tft.println(numSquare);
  tft.setRotation(0);

}


/*
   Delete the last number inserted
*/
void deleteSmallSquare() {

  tft.fillRect(BOXSIZEY * 3 + INTERVAL * 4, 30 + ((counter - 1) + (counter - 1) - 1)*SMALLBOXSIZE + SMALLBOXSIZE, SMALLBOXSIZE, SMALLBOXSIZE + 1, ILI9341_BLACK);
  counter--;

  int rest = code % 10;
  code = code - rest;
  code = code / 10;

}


/*
   Print all the squares
*/
void printSquares() {

  tft.fillScreen(ILI9341_BLACK);

  tft.fillRect(0                          ,                           0, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);
  tft.fillRect(0                          ,       BOXSIZEX +   INTERVAL, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);
  tft.fillRect(0                          , BOXSIZEX * 2 + INTERVAL * 2, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);

  tft.fillRect(BOXSIZEY + INTERVAL        ,                           0, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);
  tft.fillRect(BOXSIZEY + INTERVAL        ,       BOXSIZEX +   INTERVAL, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);
  tft.fillRect(BOXSIZEY + INTERVAL        , BOXSIZEX * 2 + INTERVAL * 2, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);

  tft.fillRect(BOXSIZEY * 2 + INTERVAL * 2,                           0, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);
  tft.fillRect(BOXSIZEY * 2 + INTERVAL * 2,       BOXSIZEX +   INTERVAL, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);
  tft.fillRect(BOXSIZEY * 2 + INTERVAL * 2, BOXSIZEX * 2 + INTERVAL * 2, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);

  tft.fillRect(BOXSIZEY * 2 + INTERVAL * 2, BOXSIZEX * 3 + INTERVAL * 3, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);


  tft.fillRect(0, BOXSIZEX * 3 + INTERVAL * 3, 145 , 100, ILI9341_RED);
  tft.fillRect(BOXSIZEY * 2 + INTERVAL, BOXSIZEX * 4 + INTERVAL * 4, BOXSIZEY + INTERVAL, INTERVAL * 4, ILI9341_RED);

}


/*
   Print all the numbers in the squares
*/
void printText() {

  tft.setRotation(1);

  tft.setCursor(0, 2);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(1);
  tft.println("CODE:");

  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2, 42);
  tft.println("1");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX + INTERVAL, 42);
  tft.println("2");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX * 2 + INTERVAL * 2, 42);
  tft.println("3");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2, 117);
  tft.println("4");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX + INTERVAL, 117);
  tft.println("5");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX * 2 + INTERVAL * 2, 117);
  tft.println("6");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2, 192);
  tft.println("7");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX + INTERVAL, 192);
  tft.println("8");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX * 2 + INTERVAL * 2, 192);
  tft.println("9");

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX * 3 + INTERVAL * 3, 42);
  tft.println("0");

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(238, 220);
  tft.println("DELETE");

  tft.setRotation(0);

}


/********************************************
 * **************************************** *
 * **************************************** *
                 MORSE  CODE
 * **************************************** *
 * **************************************** *
 ********************************************/


void setupMorse() {

  position_y = 4;
  position_x = 20;
  clicks     = 0;
  no_clicks  = 0;
  writed     = 0;
  index      = 0;
  printWhiteArea();
  printDotLine();

}


void loopMorse() {

  if (Serial.available() > 0) {
    readMessage();
    return;
  }

  //LONG DETECTED
  if (clicks >= 3) {
    if (!writed) {
      printDeleteBtn(ILI9341_RED);
    }
    writed = 1;
    morseCode[index] = 'T';
    index++;
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
    morseCode[index] = 'B';
    index++;
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

    if (position_y >= 14) {
      printAnswer();
      return;
    }

    position_x = WIDTH;
    morseCode[index] = ':';
    index++;
    printDeleteBtn(ILI9341_BLACK);
    printDotLine();
  }

  if (ts.bufferEmpty()) {
    no_clicks++;
    delay(85);
    return;
  }

  TS_Point p = ts.getPoint();
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  if (p.x > 0 && p.x < WHITEHEIGHT) {
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
  tft.fillRect(0, 0, WHITEHEIGHT, 240, ILI9341_WHITE);

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

  tft.fillRect(0, 240, WHITEHEIGHT, 80, color);

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
  analyzePosition();

}


/*
     Print ball symbol
*/
void printLine(int color) {

  tft.fillRect(TS_Y - position_y * HEIGHT + 3, position_x, HEIGHT / 2, HEIGHT * 2, color);
  position_x = position_x + 23;
  analyzePosition();

}


/*
     Print indicator in the current line
*/
void printDotLine() {

  if (position_y >= 14) {
    printAnswer();
    return;
  }

  tft.fillRect(WHITEHEIGHT, 0, TS_Y - WHITEHEIGHT, WIDTH, ILI9341_BLACK); //clean the area
  tft.fillTriangle(TS_Y - position_y * HEIGHT, 0, TS_Y + 10 - position_y * HEIGHT, 0, TS_Y + 5 - position_y * HEIGHT, 10, ILI9341_RED);
}


/*
   Analyze the current position of the cursor
   to know if it's necessary to change the line
*/
void analyzePosition() {

  if ((position_x > TS_X - WIDTH)) {
    position_y += 2;

    if (position_y >= 14) {
      printAnswer();
      return;
    }

    position_x = WIDTH;
    no_clicks  = 0;
    writed = 0;
    printDeleteBtn(ILI9341_BLACK);
    printDotLine();
  }

}


/*
   Print the code to the serial port
*/
void printAnswer() {

  Serial.print("PI:MORSE:");
  for (int i = 0; i < index; i++) {
    Serial.print(morseCode[i]);
  }
  index = 0;

  waiting = 1;
  delay(500);
  waitingMsg();

}


/********************************************
 * **************************************** *
 * **************************************** *
                   MESSAGE
 * **************************************** *
 * **************************************** *
 ********************************************/


void readMessage() {

  unsigned long start = micros();
  unsigned long end;
  bool final = false;
  bool finished = true;
  indexRead = 0;

  while (!final) {

    if (indexRead < size - 1 && Serial.available() > 0) {

      inChar = Serial.read();     // Read a character
      inData[indexRead] = inChar; // Store it
      indexRead++;                // Increment where to write next

      if (inData[indexRead - 1] == 'X') {
        final = true;
        index = index - 1;
      } else {
        delay(50);
      }
    }
    end = micros() - start;
    if (end > 2500000) {
      finished = false;
      final = true;
    }
  }

  if (finished) {
    if (startsWith("AR:IMG:", inData, indexRead)) {

      int image = (int) inData[7];
      image = image - 48;
      indexRead = 0;

      if (image >= 0 && image <= 8) {
        Serial.println("PI:MSG:OK");
      }
      else {
        Serial.println("PI:MSG:NOK:BADMSG");
        return;
      }

      if (waiting) waiting = 0;

      if (image >= 1 && image <= 5) {

        if (opened) return;

        showInfo(image);

        while (!ts.bufferEmpty()) {
          ts.getPoint();
        }

        for (int i = 0; i < 50; i++) {
          delay(100);
          if (!ts.bufferEmpty()) {
            break;
          }
        }

        while (!ts.bufferEmpty()) {
          ts.getPoint();
        }
        backToMenu(1);

      } else if (image == 6) {

        if (!opened) {
          blocked = 1;
          int tmp = blockedTft();
          //tmp indicates if the lockout was interrupted
          //when rpi send a 7code during the lockout
          backToMenu(tmp);
        }
        return;

      } else if (image == 7) {

        opened = 1;
        blocked = 0;
        actualMode = 0;
        return;

      } else if (image == 8) {

        if (actualMode != 0 || opened == 1) {
          backToMenu(1);
        }
        opened = 0;
        return;

      }
    } else {
      indexRead = 0;
      Serial.println("PI:MSG:NOK:BADSTART");
    }
  }
  else {
    Serial.println("PI:MSG:NOK:NOTEND");
  }
}


void showInfo(int image) {



  if (image >= 1 && image <= 6) {

    tft.fillScreen(ILI9341_BLACK);
    tft.setRotation(1);

    int tmp, tmp2;
    tft.setTextSize(12);
    if (image <= 3) {
      if (image == 3) {
        tft.fillRect(0, 80, 100, 12, ILI9341_GREEN);
      } else {
        tft.fillRect(80, 152, 240, 12, ILI9341_GREEN);
      }
      tft.setTextColor(ILI9341_GREEN);
      tmp = image;
    } else {
      tft.setTextColor(ILI9341_RED);
      tmp = image - 3;
    }
    if (tmp == 1) {
      tmp2 = 38;
    } else {
      tmp2 = 50;
    }
    tft.setCursor(tmp2, 80);
    tft.println(tmp);

    tft.setTextSize(4);

    if (image >= 4) {
      tft.setTextColor(ILI9341_RED);
      tft.setCursor(0, 170);
      tft.println("in");
    }
    tft.setTextColor(ILI9341_WHITE);

    tft.setCursor(0, 170);
    tft.println("  correct");
    tft.setCursor(0, 200);

    if (image == 1 || image == 4) {
      tft.println("  code");
    } else {
      tft.println("  codes");
    }

  }

  tft.setRotation(0);

}


bool startsWith(const char *pre, const char *str, int lengthStr) {
  size_t lenpre = strlen(pre),
         lenstr = lengthStr;
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
