#include <Arduino.h>
#include <Wire.h>
//#include <Serial.h>

int wireRead;
int p = 467;  // value of pulldown resistor
int q = 1020; // value of pot max
float y;
int digitalLoadSize = 0;
int analogLoadSize = 0;
bool initilize = false;
int slaveAssignID = 1;
int maxSlaves = 1;
int keyAssignment[42 * 1] = {}; // put ascii ID numbers in this array, this will then be accessed when sending keyboard commands
byte connectionAttempts = 0;
bool firstContact = false;
byte runs = 0;


int wireReadInt()
{
  int storageInt = 0;
  storageInt = Wire.read();     // reads the first byte of the input
  storageInt = storageInt << 8; // shifts the variable accross by a 8 bits to make room for the next one
  storageInt |= Wire.read();    // reads the second byte of the input and appends to the end of the variable sets bits to 1 unless 0
  return storageInt;
}

void readbutton()
{

  byte buttonState;
  int keyData;
  buttonState = Wire.read();
  keyData = Wire.read();
  keyData = keyData << 8;
  keyData |= Wire.read();

  Serial.println(String(keyData) + ":  " + String(buttonState)); // replace with key press later.
}

void getconfig()
{
}

void setconfig(int boardID)
{
}

void setup()
{

  Serial.begin(9600); // setup serial
  Serial.println("running");

  delay(3000);

  Wire.begin();

  while (connectionAttempts < 10)
  {
    delay(100);

    Serial.println("Attempting Connection: " + String(connectionAttempts));
    Wire.requestFrom(253, 1);

    wireRead = Wire.read();
    Serial.println(wireRead);
    while (wireRead > 0 && wireRead != 255)
    {
      // recieve info here from the board, liek the confiugration and layout.
      Serial.println("Slave Responce:" + String(wireRead));

      //*****chnage this so it only happens whent the pc component is connected and asking for the config, then serve up one by one

      Wire.requestFrom(253, 28); // requesting the moduleID array from the slave. //was 84
      for (int l = 0; l < 14; l++)
      {
        // keyAssignment[l+((slaveAssignID-1)*42)] = wireReadInt(); // reads each int from the Module ID array and stors it in the key asssignment array
        // Serial.println(String(keyAssignment[l+((slaveAssignID-1)*42)])); // prints each entry to Serial
      }
      /*
      Wire.requestFrom(253,28);// requesting the moduleID array from the slave. //was 84
      for (int l = 14; l < 28; l++){
        keyAssignment[l+((slaveAssignID-1)*42)] = wireReadInt(); // reads each int from the Module ID array and stors it in the key asssignment array
        Serial.println(String(keyAssignment[l+((slaveAssignID-1)*42)])); // prints each entry to Serial
      }

      Wire.requestFrom(253,28);// requesting the moduleID array from the slave. //was 84
      for (int l = 28; l < 42; l++){
        keyAssignment[l+((slaveAssignID-1)*42)] = wireReadInt(); // reads each int from the Module ID array and stors it in the key asssignment array
        Serial.println(String(keyAssignment[l+((slaveAssignID-1)*42)])); // prints each entry to Serial
      }
*/

      wireRead = 0;
      firstContact = true;
    }

    if (firstContact == true)
    { // sending the next avalible ID number to the slave
      Wire.beginTransmission(253);
      Serial.println("Sending Slave ID: " + String(slaveAssignID));
      Wire.write(slaveAssignID);
      slaveAssignID++;
      Wire.endTransmission();
      firstContact = false; // end first contact mode
    }

    connectionAttempts++;
  }

  maxSlaves = slaveAssignID; // increases the max slaves to the highest number of slaves added
                             // max slaves will be used later on
}

void loop()
{

  // put trigger in for setting config mode = true
  // call setconfig(boardID)  function boardID is sent from PC
  // sequencily set each element of the array

  delay(20);
  Serial.println("running");
  
  for (int currentSlave = 0; currentSlave < maxSlaves; currentSlave++)
  {

    Wire.beginTransmission(currentSlave); // start transmission to first slave board
    Wire.write(10);                       // tell sllave to prepare load size
    Serial.println("Prep Load Size");
    Wire.endTransmission(); // end transmission

    Wire.requestFrom(currentSlave, 2); // address, quantity -- requesting load size // request digital and analog load size

    while (Wire.available())
    {
      digitalLoadSize = Wire.read(); // read the load size frrom the slave
      analogLoadSize = Wire.read();  // read the load size frrom the slave
      Serial.println("Load Size Digital:" + String(digitalLoadSize));
      Serial.println("Load Size Analog:" + String(analogLoadSize));

    }

    // Wire.requestFrom(currentSlave,digitalLoadSize + analogLoadSize); // address, quantity --reqesting load from slave

    while (digitalLoadSize / 32 > 1) // start generating runs
    {
      runs++;
      digitalLoadSize - 32;
    }

    if (runs != 0)
    {
      Wire.requestFrom(currentSlave, 32);
      Serial.println("Requesting Load: 32");
      Serial.println("runs left: " + String(runs));
      runs--;
    }
    else
    {
      if (runs == 0 || digitalLoadSize > 0)
      {
        Wire.requestFrom(currentSlave, digitalLoadSize);
        Serial.println("Requesting Load: " + String(digitalLoadSize));
      }
    }

    while (Wire.available())
    { // read the button section of the load
      for (int i = 0; i < digitalLoadSize / 3; i++)
      { // read the analog section of the load.
        readbutton();
      }

      /*
           Serial.println("row1:" + String(Wire.read()));
           Serial.println("row2:" + String(Wire.read()));
           Serial.println("row3:" + String(Wire.read()));
           Serial.println("row4:" + String(Wire.read()));
           Serial.println("row5:" + String(Wire.read()));
           Serial.println("row6:" + String(Wire.read()));
      */

      for (int i = 0; i < analogLoadSize / 3; i++)
      { // read the analog section of the load.
        int potID;
        potID = Wire.read();
        wireRead = Wire.read();   // reads the first byte of the input
        wireRead = wireRead << 8; // shifts the variable accross by a 8 bits to make room for the next one
        wireRead |= Wire.read();  // reads the second byte of the input and appends to the end of the variable sets bits to 1 unless 0
        Serial.println("Pot " + String(potID) + ":" + String(wireRead));
      }
    

    }

  }
}
