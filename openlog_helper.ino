int is_cmd_mode = 0;

//Setups up the software Serial1, resets OpenLog so we know what state it's in, and waits
//for OpenLog to come online and report '<' that it is ready to receive characters to record
void setupOpenLog(void) {
  pinMode(resetOpenLog, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  //Reset OpenLog
  digitalWrite(resetOpenLog, LOW);
  delay(5);
  digitalWrite(resetOpenLog, HIGH);
  digitalWrite(LED_BUILTIN, LOW);

  //Wait for OpenLog to respond with '<' to indicate it is alive and recording to a file
  while (1) {
    if (Serial1.available())
      if (Serial1.read() == '<') break;
  }
  Serial.println("Done initializing openlog");
  digitalWrite(LED_BUILTIN, HIGH);
}

//Open a file for writing. Begin writing at an offset
boolean writeFile(char *fileName, int offset) {
  if (!is_cmd_mode) gotoCommandMode();
  Serial1.print("write ");
  Serial1.print(fileName);
  Serial1.print(" ");
  Serial1.print(offset);
  Serial1.write(13); //This is \r

  //The Serial1 echos the commands we send it by default so we have 'write log254.txt 10\r' sitting
  //in the RX buffer. Let's try to ignore this.
  while (1) {
    if (Serial1.available())
      if (Serial1.read() == '\r') break;
  }

  //Serial1 should respond with a < letting us know it's receiving characters
  int counter = 0;
  while (counter++ < 1000) {
    if (Serial1.available())
      if (Serial1.read() == '<') {
        is_cmd_mode = 0;
        // Serial.println("Set to 0 at: " + String(47));
        return (true);
      }
    delay(1);
  }

  // Serial.println("Write offset failed: Does the file exist?");
  return (false);
}

//Open a file for appending. Begin writing at an offset
boolean appendFile(char *fileName) {
  int start = millis();
  if (!is_cmd_mode) gotoCommandMode();
  Serial1.print("append ");
  Serial1.print(fileName);
  Serial1.write(13); //This is \r

  //The Serial1 echos the commands we send it by default so we have 'append log254.txt 10\r' sitting
  //in the RX buffer. Let's try to ignore this.
//  while (1) {
//    if (Serial1.available())
//      if (Serial1.read() == '\r') break;
//  }

  //Serial1 should respond with a < letting us know it's receiving characters
  int counter = 0;
  while (counter++ < 1000) {
    if (Serial1.available())
      if (Serial1.read() == '<') {
        delay(5);
        is_cmd_mode = 0;
        // Serial.println("Set to 0 at: " + String(77));
        Serial.println("APPEND " + (String)fileName + ": " + (millis() - start));
        return (true);
      }
    delay(1);
  }

  // Serial.println("Append file failed: Does the file exist?");
  return (false);
}

//Open a file for appending. Begin writing at an offset
boolean runCommand(char *command, char stop_char) {
  if (!is_cmd_mode) gotoCommandMode();
  // Serial.print("running command: " + (String)command + " ...");
  Serial1.println((String)command);

  //Serial1 should respond with a < letting us know it's receiving characters
  int counter = 0;
  while (counter++ < 1000) {
    if (Serial1.available())
      if (Serial1.read() == stop_char) {
        delay(5);
        // Serial.println(" Success!");
        return (true);
      }
    delay(1);
  }

  // Serial.println("Commmand Failed");
  return (false);
}

//This function creates a given file and then opens it in append mode (ready to record characters to the file)
//Then returns to listening mode
void createFile(char *fileName) {
  if (!is_cmd_mode) gotoCommandMode();
  // Serial.print("Creating file: " + (String)fileName + " ... ");
  //Old way
  //  Serial1.print("new ");
  //  Serial1.print(fileName);
  //  Serial1.write(13); //This is \r

  //New way
  Serial1.print("new ");
  Serial1.println(fileName); //regular println works with Serial1 v2.51 and above

  //Wait for Serial1 to return to waiting for a command
  while (1) {
    if (Serial1.available())
      if (Serial1.read() == '>') break;
  }

  Serial1.print("append ");
  Serial1.println(fileName);

  //Wait for Serial1 to indicate file is open and ready for writing
  while (1) {
    if (Serial1.available())
      if (Serial1.read() == '<') break;
  }

  delay(5);
  is_cmd_mode = 0;
  // Serial.println("Success, set to 0 at: " + String(138));
  //Serial1 is now waiting for characters and will record them to the new file
}

//Reads the contents of a given file and dumps it to the serial terminal
//This function assumes the Serial1 is in command mode
void readFile(char *fileName) {
  if (!is_cmd_mode) gotoCommandMode();
  while (Serial1.available()) Serial1.read(); //Clear incoming buffer

  Serial1.print("read ");
  Serial1.print(fileName);
  Serial1.write(13); //This is \r

  //The Serial1 echos the commands we send it by default so we have 'read log823.txt\r' sitting
  //in the RX buffer. Let's try to not print this.
  while (1) {
    if (Serial1.available())
      if (Serial1.read() == '\r') break;
  }

  // Serial.println("Reading from file:");

  //This will listen for characters coming from Serial1 and print them to the terminal
  //This relies heavily on the SoftSerial buffer not overrunning. This will probably not work
  //above 38400bps.
  //This loop will stop listening after 1 second of no characters received
  for (int timeOut = 0 ; timeOut < 1000 ; timeOut++) {
    while (Serial1.available()) {
      char tempString[100];

      int spot = 0;
      while (Serial1.available()) {
        tempString[spot++] = Serial1.read();
        if (spot > 98) break;
      }
      tempString[spot] = '\0';
      Serial.write(tempString); //Take the string from Serial1 and push it to the Arduino terminal
      timeOut = 0;
    }

    delay(5);
  }

  //This is not perfect. The above loop will print the '.'s from the log file. These are the two escape characters
  //recorded before the third escape character is seen.
  //It will also print the '>' character. This is the Serial1 telling us it is done reading the file.

  //This function leaves Serial1 in command mode
}

//Check the stats of the SD card via 'disk' command
//This function assumes the Serial1 is in command mode
void readDisk() {
  if (!is_cmd_mode) gotoCommandMode();
  //Old way
  Serial1.print("disk");
  Serial1.write(13); //This is \r

  //New way
  //Serial1.print("read ");
  //Serial1.println(filename); //regular println works with Serial1 v2.51 and above

  //The Serial1 echos the commands we send it by default so we have 'disk\r' sitting
  //in the RX buffer. Let's try to not print this.
  while (1) {
    if (Serial1.available())
      if (Serial1.read() == '\r') break;
  }

  //This will listen for characters coming from Serial1 and print them to the terminal
  //This relies heavily on the SoftSerial buffer not overrunning. This will probably not work
  //above 38400bps.
  //This loop will stop listening after 1 second of no characters received
  for (int timeOut = 0 ; timeOut < 1000 ; timeOut++) {
    while (Serial1.available()) {
      char tempString[100];

      int spot = 0;
      while (Serial1.available()) {
        tempString[spot++] = Serial1.read();
        if (spot > 98) break;
      }
      tempString[spot] = '\0';
      Serial.write(tempString); //Take the string from Serial1 and push it to the Arduino terminal
      timeOut = 0;
    }

    delay(5);
  }
  //This is not perfect. The above loop will print the '.'s from the log file. These are the two escape characters
  //recorded before the third escape character is seen.
  //It will also print the '>' character. This is the Serial1 telling us it is done reading the file.

  //This function leaves Serial1 in command mode
}

//This function pushes Serial1 into command mode
void gotoCommandMode(void) {
  //Send three control z to enter Serial1 command mode
  //Works with Arduino v1.0
  if (is_cmd_mode) return;
  // Serial.print("Setting command mode... ");
  Serial1.write(26);
  Serial1.write(26);
  Serial1.write(26);

  //Wait for Serial1 to respond with '>' to indicate we are in command mode
  while (1) {
    if (Serial1.available()) {
      if (Serial1.read() == '>') break;
    }
  }
  delay(5);
  // Serial.println("Success!");
  is_cmd_mode = 1;
}

void openlog_terminal() {
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      //      Serial1.write(13);
      Serial1.println("");
    } else if (c == '^') {
      gotoCommandMode();
    }
    else {
      Serial1.print(c);
    }
  }
  if (Serial1.available()) {
    Serial.print((char)Serial1.read());
  }
}
