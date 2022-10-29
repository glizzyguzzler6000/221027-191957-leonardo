#include <Arduino.h>
#include <Wire.h>
//#include <Serial.h>

byte wireRead;
int p = 467;  // value of pulldown resistor
int q = 1020; // value of pot max
float y;
int digitalLoadSize = 0;
int analogLoadSize = 0;
bool initilize = false;
byte slaveAssignID = 1;
int maxSlaves = 1;
int keyAssignment[42 * 1] = {}; // put ascii ID numbers in this array, this will then be accessed when sending keyboard commands
byte connectionAttempts = 0;
bool firstContact = false;
byte runs = 0;
byte recieveingSlaveID = 0;
byte slaveRuns = 0;

byte digitalChangeWeight = 5;


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

  recieveingSlaveID = Wire.read();

  buttonState = Wire.read();
  buttonState = keyData << 8;
  buttonState |= Wire.read();

  keyData = Wire.read();
  keyData = keyData << 8;
  keyData |= Wire.read();

  Serial.println("Button press recieved: " + String(recieveingSlaveID) + ", " + String(buttonState) + ", " + String(keyData)); // replace with key press later.
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

  delay(4000);

  Wire.begin();

  while (connectionAttempts < 10)
  {
    delay(100);

    Serial.println("Attempting Connection: " + String(connectionAttempts));
    Wire.requestFrom(253, 1);

    wireRead = Wire.read();
    Serial.println("Initial Responce" + String(wireRead));
    while (wireRead > 0 && wireRead != 255)
    {
      // recieve info here from the board, liek the confiugration and layout.
      Serial.println("Slave Responce:" + String(wireRead));

      //*****chnage this so it only happens whent the pc component is connected and asking for the config, then serve up one by one

      // Wire.requestFrom(253, 28); // requesting the moduleID array from the slave. //was 84
      // for (int l = 0; l < 14; l++)
      // {
      //   // keyAssignment[l+((slaveAssignID-1)*42)] = wireReadInt(); // reads each int from the Module ID array and stors it in the key asssignment array
      //   // Serial.println(String(keyAssignment[l+((slaveAssignID-1)*42)])); // prints each entry to Serial
      // }
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
      Wire.endTransmission();
      firstContact = false; // end first contact mode
      Wire.requestFrom(slaveAssignID, 2);
      wireRead = Wire.read();
      Serial.println("slave ASSIGN ID: " + String(slaveAssignID));
      Serial.println("Confirm slave 67: " + String(wireRead));
      wireRead = Wire.read();
      Serial.println("Confirm slave id: " + String(wireRead));
      slaveAssignID++;
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
  Serial.println("\r");
  delay(1000);
  Serial.println("running");

  for (int currentSlave = 1; currentSlave < maxSlaves; currentSlave++)
  {
    Serial.println("Requesing Data from: Slave " + String(currentSlave));

    // Wire.beginTransmission(currentSlave); // start transmission to first slave board
    // Wire.write(10);                       // tell sllave to prepare load size
    // Serial.println("Prep Load Size");
    // Wire.endTransmission(); // end transmission

    Wire.requestFrom(currentSlave, 2); // address, quantity -- requesting load size // request digital and analog load size
    Serial.println("Current wire avalible: " + String(Wire.available()));

    if (Wire.available() > 0)
    {
      digitalLoadSize = Wire.read(); // read the load size from the slave
      analogLoadSize = Wire.read();  // read the load size from the slave
      //slaveRuns = Wire.read();
      Serial.println("Load Size Digital:" + String(digitalLoadSize));
      Serial.println("Load Size Analog:" + String(analogLoadSize));
      //Serial.println("slave runs: " + String(slaveRuns));

      if (digitalLoadSize > 0 || analogLoadSize > 0)
      {
        if (digitalLoadSize != 255 || analogLoadSize != 255)
        {

          // Wire.requestFrom(currentSlave,digitalLoadSize + analogLoadSize); // address, quantity --reqesting load from slave

          while (digitalLoadSize / 30 > 1) // start generating runs
          {
            runs++;
            digitalLoadSize = digitalLoadSize - 30;
          }

          if (runs != 0)
          {
            Wire.requestFrom(currentSlave, 30);
            Serial.println("Requesting Load: 30");
            Serial.println("runs left: " + String(runs));
            runs--;
          }
          if (runs == 0)
          {
            if (runs == 0 && digitalLoadSize > 0)
            {
              Wire.requestFrom(currentSlave, digitalLoadSize);
              Serial.println("Requesting Load: " + String(digitalLoadSize));
            }
          }


          Serial.println("Wire avalible button press: " + String(Wire.available()));
         // while (Wire.available() > 0)
          //{ // read the button section of the load
            for (int i = 0; i < digitalLoadSize / digitalChangeWeight; i++)
            { // read the analog section of the load.
              readbutton();
            }

            // for (int i = 0; i < analogLoadSize / 3; i++)
            // { // read the analog section of the load.
            //   int potID;
            //   potID = Wire.read();
            //   wireRead = Wire.read();   // reads the first byte of the input
            //   wireRead = wireRead << 8; // shifts the variable accross by a 8 bits to make room for the next one
            //   wireRead |= Wire.read();  // reads the second byte of the input and appends to the end of the variable sets bits to 1 unless 0
            //   Serial.println("Pot " + String(potID) + ":" + String(wireRead));
            // }
          //}
        }
      }
    }
  }
}
