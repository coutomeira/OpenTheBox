#include <Servo.h>

//BUTTON
int buttonPin = 7;

//MOTOR
Servo myServo;
int motorPin = 10;

//LED RGB
int redPin   = 11;
int greenPin = 6;
int bluePin  = 5;

//READ MODE
#define size 50
char inData[size]; // Allocate some space for the string
char inChar = -1;  // Where to store the character read
byte indexRead = 0;// Index into array; where to store the character 'inChar'

//VARIABLES
int motorClosed;   //Indica se a rotação do motor bloqueia a tampa
int boxClosed;     //Indica se a tampa está em baixo
int buttonState;   //Butão pressionado ou não
int wantClose;     //Se recebi ordem para fechar

void setup() {

  Serial.begin(9600);

  yield();

  pinMode(buttonPin, INPUT );
  pinMode(redPin,    OUTPUT);
  pinMode(greenPin,  OUTPUT);
  pinMode(bluePin,   OUTPUT);

  myServo.attach(motorPin);
  myServo.write(0);

  //START OPENED
  motorClosed = 0;
  boxClosed = 0;
  wantClose = 0;
  setColor(0, 255, 0);
  //Serial.println("PI:TS:OK");
  //Serial.println("PI:ARDUINO:2:OK");
  //Serial.println("PI:LED:G");

}

void loop() {

  if (Serial.available() > 0) {
    readMessage();
    return;
  }

  buttonState = digitalRead(buttonPin);

  if (boxClosed == buttonState) {
    //Serial.println("No changes verified.");
    return;
  }
  else {
    //Serial.print("Change Occured");
    if (!motorClosed && !boxClosed && buttonState) {
      rotateMotor(0);
      turnLed(0);
      motorClosed = 1;
      wantClose = 0;
    }
    boxClosed = buttonState;
  }


  if (wantClose && boxClosed) {
    rotateMotor(0);
    turnLed(0);
    motorClosed = 1;
    wantClose = 0;
    Serial.println("PI:BOX:C");
  } else if (wantClose && !boxClosed) {
    Serial.println("PI:NOTPOSSIBLE");
  }

}


/********************************************
 * **************************************** *
 * **************************************** *
                  LED RGB
 * **************************************** *
 * **************************************** *
 ********************************************/


void turnLed(int flag) {

  if (flag) {
    setColor(0, 255, 0);    //OPENED green
    Serial.println("PI:LED:G");
  }

  else {
    setColor(255, 0, 0);    //CLOSED red
    Serial.println("PI:LED:R");
  }

}


void setColor(int red, int green, int blue) {
#ifdef COMMON_ANODE
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
#endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}


/********************************************
 * **************************************** *
 * **************************************** *
                   MOTOR
 * **************************************** *
 * **************************************** *
 ********************************************/


void rotateMotor(int flag) {

  if (flag) {
    myServo.write(0); //OPENED
    Serial.println("PI:MOTOR:O");
  }

  else {
    myServo.write(90);//CLOSED
    Serial.println("PI:MOTOR:C");
  }

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
      } else {
        delay(5);
      }
    }
    unsigned long end = micros() - start;
    if (end > 2500000) {
      finished = false;
      final = true;
    }
  }

  if (finished) {
    if (startsWith("AR:", inData, indexRead)) {

      char action = inData[3];

      if ((action == 'C') || (action == 'O') || (action == 'H')) {
        Serial.println("PI:MSG:OK");
      }
      else {
        Serial.println("PI:MSG:NOK:BADMSG");
        return;
      }
      //DEPRECATED
      if (action == 'C') {

        if (!motorClosed) {
          wantClose = 1;
        } else {
          Serial.println("PI:BOX:C");
        }

      } else if (action == 'O') {

        wantClose = 0;

        if (motorClosed) {
          rotateMotor(1);
          turnLed(1);
          motorClosed = 0;
          Serial.println("PI:BOX:O");
        } else {
          Serial.println("PI:BOX:O");
        }
      } else if (action == 'H') {
        if (motorClosed) {
          Serial.println("PI:BOX:C");
        } else {
          Serial.println("PI:BOX:O");
        }
      } 
    }
    else {
      indexRead = 0;
      Serial.println("PI:MSG:NOK:BADSTART");
    }
  }
  else {
    Serial.println("PI:MSG:NOK:NOTEND");
  }
}


bool startsWith(const char *pre, const char *str, int lengthStr) {
  size_t lenpre = strlen(pre),
         lenstr = lengthStr;
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
