#define size 50
char inData[size]; // Allocate some space for the string
char inChar = -1;  // Where to store the character read
byte index = 0;    // Index into array; where to store the character
bool read = false; // Indicates if something was read from serial input
bool final = false;// Indicates if a '\n' was read at the end of the message


void setup() {
  Serial.begin(9600);
  Serial.write("Power On\n");
}


int validMessage() {

  final = false;

  if (Serial.available() > 0) {

    while (!final) {

      if (index < size - 1 && Serial.available() > 0) {

        inChar = Serial.read(); // Read a character
        inData[index] = inChar; // Store it
        index++;                // Increment where to write next

        if (index >= 3 && inData[index - 3] == 'E' && inData[index - 2] == 'N' && inData[index - 1] == 'D') {
          final = true;
          index = index - 3;
        } else {
          delay(5);
        }
      }
    }
    read = true;
  }

  if (read) {
    Serial.print("PI:MSG:");


    read = false;
    if (startsWith("AR:", inData, index)) {
      for (int i = 3; i < index; i++) {
        Serial.print(inData[i]);
      }
      Serial.println(":END");
      index = 0;
      return 1;
    }
    else {
      for (int i = 0; i < index; i++) {
        Serial.print(inData[i]);
      }
      Serial.println(":END");
      index = 0;
      return 2;
    }
  }
  else {
    return 3;
  }
}


bool startsWith(const char *pre, const char *str, int lengthStr)
{
  size_t lenpre = strlen(pre),
         lenstr = lengthStr;
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}


void loop()
{
  int tmp = validMessage();
  if (tmp == 1) {
    Serial.write("PI:INFO:VALID:END\n");
  } else if (tmp == 2) {
    Serial.write("PI:INFO:NOT VALID:END\n");
  }
}
