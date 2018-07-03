#include <Adafruit_GFX.h>    // Core graphics library
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

// Size of the color selection boxes and the paintbrush size
#define BOXSIZEX 70
#define BOXSIZEY 70
#define SMALLBOXSIZE 10
#define INTERVAL 5

#define MINPRESSURE 0
#define MAXPRESSURE 1000

//The square where user touched
int numSquare;
int counter = 0;
int number = 0;
#define BUFFPIXEL 20

long code = 0;

void setup(void) {

  Serial.begin(9600);
  yield();

  if (!SD.begin(SD_CS)) {
    Serial.println("PI:SD NOK");
  }
  Serial.println("PI:SD OK");

  tft.begin();

  if (!ts.begin()) {
    Serial.println("PI:TS NOK");
    while (1);
  }
  Serial.println("PI:TS OK");

  printSquares();
  printText();
  Serial.println("PI:ARDUINO OK");
}


void loop()
{
  if (ts.bufferEmpty()) {
    return;
  }

  // Retrieve a point
  TS_Point p = ts.getPoint();

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {

    number = 0;

    // Scale from ~0->4000 to tft.width using the calibration #'s
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());


    if (p.y < BOXSIZEX) {
      if (p.x < BOXSIZEY) {
        numSquare = 7;
        drawSmallSquare(numSquare);
        printText();
        counter++;
        number = 1;
      } else if (p.x > BOXSIZEY + INTERVAL && p.x < BOXSIZEY * 2 + INTERVAL) {
        numSquare = 4;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      } else if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
        numSquare = 1;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      }
    } else if (p.y > BOXSIZEX + INTERVAL && p.y < BOXSIZEX * 2 + INTERVAL) {
      if (p.x < BOXSIZEY) {
        numSquare = 8;
        drawSmallSquare(numSquare);
        printText();
        counter++;
        number = 1;
      } else if (p.x > BOXSIZEY + INTERVAL && p.x < BOXSIZEY * 2 + INTERVAL) {
        numSquare = 5;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      } else if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
        numSquare = 2;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      }
    } else if (p.y > BOXSIZEX * 2 + INTERVAL * 2 && p.y < BOXSIZEX * 3 + INTERVAL * 2) {
      if (p.x < BOXSIZEY) {
        numSquare = 9;
        drawSmallSquare(numSquare);
        printText();
        counter++;
        number = 1;
      } else if (p.x > BOXSIZEY + INTERVAL && p.x < BOXSIZEY * 2 + INTERVAL) {
        numSquare = 6;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      } else if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
        numSquare = 3;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      }
    } else if (p.y > BOXSIZEX * 3 + INTERVAL * 3 && p.y < BOXSIZEX * 4 + INTERVAL * 3) {
      if (p.x > BOXSIZEY * 2 + INTERVAL * 2 && p.x < BOXSIZEY * 3 + INTERVAL * 2) {
        numSquare = 0;
        drawSmallSquare(numSquare);
        counter++;
        number = 1;
      }
    }
    if (p.y > BOXSIZEX * 3 + INTERVAL * 8) {
      if (p.x < BOXSIZEY) {
        if (counter > 0) {
          deleteSmallSquare();
          number = 0;
        }
      }
    }

    if (number) {
      code = (code * 10) + numSquare;
      
      //repainting top bar
      if (counter == 5) {

        if (code == 0) {
        bmpDraw("one.bmp", 0, 0);
        //convertImage("one.bmp","one.txt");
        delay(1000);
        printSquares();
        printText();
      }
      
        Serial.print("PI:PIN:"); Serial.print(code); Serial.print("\n");
        delay(1000);
        counter = 0;
        code = 0;
        tft.fillRect(BOXSIZEY * 3 + INTERVAL * 4, 30, 10, 320, ILI9341_BLACK);
      }
    }

  }
  delay(125);
  while (!ts.bufferEmpty())
    ts.getPoint();
}


void drawSmallSquare(int numSquare) {
  tft.fillRect(BOXSIZEY * 3 + INTERVAL * 4, 30 + (counter + counter - 1)*SMALLBOXSIZE + SMALLBOXSIZE, SMALLBOXSIZE, SMALLBOXSIZE + 1, ILI9341_WHITE);
  tft.setRotation(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setCursor(30 + 3  + (counter + counter - 1)*SMALLBOXSIZE + SMALLBOXSIZE, 1);
  tft.println(numSquare);
  tft.setRotation(0);
}

void deleteSmallSquare() {
  tft.fillRect(BOXSIZEY * 3 + INTERVAL * 4, 30 + ((counter - 1) + (counter - 1) - 1)*SMALLBOXSIZE + SMALLBOXSIZE, SMALLBOXSIZE, SMALLBOXSIZE + 1, ILI9341_BLACK);
  counter--;

  int rest = code % 10;
  code = code - rest;
  code = code / 10;
}

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
  tft.fillRect(0                          , BOXSIZEX * 3 + INTERVAL * 8, BOXSIZEY, BOXSIZEX, ILI9341_WHITE);

  tft.fillRect(0, BOXSIZEX * 3 + INTERVAL * 3, BOXSIZEY * 2 + INTERVAL, INTERVAL * 4, ILI9341_YELLOW);
  tft.fillRect(BOXSIZEY + INTERVAL, BOXSIZEX * 3 + INTERVAL * 7, BOXSIZEY, BOXSIZEX + INTERVAL, ILI9341_YELLOW);
  tft.fillRect(BOXSIZEY * 2 + INTERVAL, BOXSIZEX * 4 + INTERVAL * 4, BOXSIZEY + INTERVAL, INTERVAL*4, ILI9341_YELLOW);
}


void printText() {

  tft.setRotation(1);

  tft.setCursor(0, 2);
  tft.setTextColor(ILI9341_WHITE);
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

  tft.setCursor(BOXSIZEX / 2 - INTERVAL * 2 + BOXSIZEX * 3 + INTERVAL * 8, 192);
  tft.println("X");

  tft.setRotation(0);

}


void bmpDraw(char *filename, int16_t x, int16_t y) {
  tft.setRotation(1);
  File     bmpFile;
  int      bmpWidth, bmpHeight;        // W+H in pixels
  uint8_t  bmpDepth;                   // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;             // Start of image data in file
  uint32_t rowSize;                    // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL];    // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;            // Set to true on valid header parse
  boolean  flip    = true;             // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if ((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.println("Reading ...");

    Serial.print(F("File size: "));
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: "));
    Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: "));
    Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if (bmpDepth == 24) {
        if (read32(bmpFile) == 0) { // 0 = uncompressed

          goodBmp = true; // Supported BMP format -- proceed!
          Serial.print(F("Image size: "));
          Serial.print(bmpWidth);
          Serial.print('x');
          Serial.println(bmpHeight);

          // BMP rows are padded (if needed) to 4-byte boundary
          rowSize = (bmpWidth * 3 + 3) & ~3;

          // If bmpHeight is negative, image is in top-down order.
          // This is not canon but has been observed in the wild.
          if (bmpHeight < 0) {
            bmpHeight = -bmpHeight;
            flip      = false;
          }

          // Crop area to be loaded
          x2 = x + bmpWidth  - 1; // Lower-right corner
          y2 = y + bmpHeight - 1;
          if ((x2 >= 0) && (y2 >= 0)) { // On screen?
            w = bmpWidth; // Width/height of section to load/display
            h = bmpHeight;
            bx1 = by1 = 0; // UL coordinate in BMP file
            if (x < 0) { // Clip left
              bx1 = -x;
              x   = 0;
              w   = x2 + 1;
            }
            if (y < 0) { // Clip top
              by1 = -y;
              y   = 0;
              h   = y2 + 1;
            }
            if (x2 >= tft.width())  w = tft.width()  - x; // Clip right
            if (y2 >= tft.height()) h = tft.height() - y; // Clip bottom

            // Set TFT address window to clipped image bounds
            tft.startWrite(); // Requires start/end transaction now
            tft.setAddrWindow(x, y, w, h);

            for (row = 0; row < h; row++) { // For each scanline...

              // Seek to start of scan line.  It might seem labor-
              // intensive to be doing this on every line, but this
              // method covers a lot of gritty details like cropping
              // and scanline padding.  Also, the seek only takes
              // place if the file position actually needs to change
              // (avoids a lot of cluster math in SD library).
              if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
              else     // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + (row + by1) * rowSize;
              pos += bx1 * 3; // Factor in starting column (bx1)
              if (bmpFile.position() != pos) { // Need seek?
                tft.endWrite(); // End TFT transaction
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
                tft.startWrite(); // Start new TFT transaction
              }
              for (col = 0; col < w; col++) { // For each pixel...
                // Time to read more pixel data?
                if (buffidx >= sizeof(sdbuffer)) { // Indeed
                  tft.endWrite(); // End TFT transaction
                  bmpFile.read(sdbuffer, sizeof(sdbuffer));
                  buffidx = 0; // Set index to beginning
                  tft.startWrite(); // Start new TFT transaction
                }
                // Convert pixel from BMP to TFT format, push to display
                b = sdbuffer[buffidx++];
                g = sdbuffer[buffidx++];
                r = sdbuffer[buffidx++];
                tft.writePixel(tft.color565(r, g, b));
              } // end pixel
            } // end scanline
            tft.endWrite(); // End last TFT transaction
          } // end onscreen
          Serial.print(F("Loaded in "));
          Serial.print(millis() - startTime);
          Serial.println(" ms");
        } // end goodBmp
      }
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));
  tft.setRotation(0);

}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


/*
void convertImage(char *filenameBmp, char *filenameTxt){

    File     bmpImage;
    File     textFile;
    // Open
    bmpImage = SD.open(filenameBmp, FILE_READ);
    textFile = SD.open(filenameTxt, FILE_WRITE);

    int32_t dataStartingOffset = readNbytesInt(&bmpImage, 0x0A, 4);

    // Change their types to int32_t (4byte)
    int32_t width = readNbytesInt(&bmpImage, 0x12, 4);
    int32_t height = readNbytesInt(&bmpImage, 0x16, 4);
    Serial.print("width:");
    Serial.println(width);
    Serial.print("height:");
    Serial.println(height);

    int16_t pixelsize = readNbytesInt(&bmpImage, 0x1C, 2);

    Serial.print("pixelsize:");
    Serial.println(pixelsize);

    if (pixelsize != 24)
    {
        Serial.println("Image is not 24 bpp");
        while (1);
    }

    bmpImage.seek(dataStartingOffset);//skip bitmap header
    Serial.print("skiped!");

    // 24bpp means you have three bytes per pixel, usually B G R

    byte R, G, B;

    for(int32_t i = 0; i < height; i ++) {
        for (int32_t j = 0; j < width; j ++) {
            B = bmpImage.read();
            G = bmpImage.read();
            R = bmpImage.read();
            textFile.print("R");
            textFile.print(R);
            textFile.print("G");
            textFile.print(G);
            textFile.print("B");
            textFile.print(B);
            textFile.print(" ");

            Serial.print("R");
            Serial.print(R);
            Serial.print("G");
            Serial.print(G);
            Serial.print("B");
            Serial.print(B);
            Serial.print(" ");
            Serial.println();
        }
        textFile.print("\n");
    }

    bmpImage.close();
    textFile.close();

    Serial.println("done write");
}

int32_t readNbytesInt(File *p_file, int position, byte nBytes)
{
    if (nBytes > 4)
        return 0;

    p_file->seek(position);

    int32_t weight = 1;
    int32_t result = 0;
    for (; nBytes; nBytes--)
    {
        result += weight * p_file->read();
        weight <<= 8;
    }
    return result;
}
*/
