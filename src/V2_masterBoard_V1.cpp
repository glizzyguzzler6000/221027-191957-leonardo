#include <Arduino.h>
#include <Wire.h>
//#include <Serial.h>

byte wireRead;
int p = 467;  // value of pulldown resistor
int q = 1020; // value of pot max
float y;
int LoadSize = 0;

bool initilize = false;
byte slaveAssignID = 1;
int maxSlaves = 1;
int keyAssignment[42 * 1] = {}; // put ascii ID numbers in this array, this will then be accessed when sending keyboard commands
byte connectionAttempts = 0;
bool firstContact = false;
byte Runs = 0;

byte recieveingSlaveID = 0;
byte slaveRuns = 0;

byte ChangeWeight = 6;

byte layoutsRecieved = 1;
byte layoutRow = 0;
bool VERBOSE = true;

int wireReadInt()
{
  int storageInt = 0;
  storageInt = Wire.read();     // reads the first byte of the input
  storageInt = storageInt << 8; // shifts the variable accross by a 8 bits to make room for the next one
  storageInt |= Wire.read();    // reads the second byte of the input and appends to the end of the variable sets bits to 1 unless 0
  return storageInt;
}

void readData()
{
  int analogValue;
  byte buttonState;
  int keyData;
  byte moduleType;

  // Serial.println("Reading Data: ");
  moduleType = Wire.read();
  // Serial.println("Module Type: " + String(moduleType));
  if (moduleType == 1)
  {

    recieveingSlaveID = Wire.read();

    buttonState = Wire.read();
    buttonState = buttonState << 8;
    buttonState |= Wire.read();

    keyData = Wire.read();
    keyData = keyData << 8;
    keyData |= Wire.read();

    if(VERBOSE){Serial.println("Button press recieved: " + String(moduleType) + ", " + String(recieveingSlaveID) + ", " + String(buttonState) + ", " + String(keyData));} // replace with key press later.
  }

  if (moduleType == 2)
  {

    recieveingSlaveID = Wire.read();

    analogValue = Wire.read();
    analogValue = analogValue << 8;
    analogValue |= Wire.read();

    Wire.read(); // purge padding from analog signal.
    Wire.read();

    if(VERBOSE){Serial.println("Analog Recieved: " + String(moduleType) + ", " + String(recieveingSlaveID) + ", " + String(analogValue));} // replace with key press later.
  }
  // LoadSize = LoadSize - ChangeWeight;
  return;
}

void readLayout()
{
  Serial.println(" ");
  int moduleID = 0;
  for(int col = 0; col < 7; col++)
  {
    moduleID = Wire.read();// read each module assignment for the row.  X 6
    moduleID = moduleID << 8;
    moduleID |= Wire.read();
    Serial.print(String(moduleID) + " ");
  }
 
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
  

  delay(6000);
Serial.println("Start");
  Wire.begin();

  while (connectionAttempts < 10)
  {
    if(VERBOSE){delay(10);}

    if(VERBOSE){Serial.println("Attempting Connection: " + String(connectionAttempts));}
    Wire.requestFrom(253, 1);

    wireRead = Wire.read();
    if(VERBOSE){Serial.println("Initial Responce" + String(wireRead));}
    while (wireRead > 0 && wireRead != 255)
    {
      // recieve info here from the board, liek the confiugration and layout.
      if(VERBOSE){Serial.println("Slave Responce:" + String(wireRead));}

      wireRead = 0;
      firstContact = true;
    }

    if (firstContact == true)
    { // sending the next avalible ID number to the slave
      Wire.beginTransmission(253);
      if(VERBOSE){Serial.println("Sending Slave ID: " + String(slaveAssignID));}
      Wire.write(slaveAssignID);
      Wire.endTransmission();
      firstContact = false; // end first contact mode
      Wire.requestFrom(slaveAssignID, 2);
      wireRead = Wire.read();
      if(VERBOSE){Serial.println("slave ASSIGN ID: " + String(slaveAssignID));}
      if(VERBOSE){Serial.println("Confirm slave 67: " + String(wireRead));}
      wireRead = Wire.read();
      if(VERBOSE){Serial.println("Confirm slave id: " + String(wireRead));}
      slaveAssignID++;
    }

    connectionAttempts++;
  }

  maxSlaves = slaveAssignID; // increases the max slaves to the highest number of slaves added
                             // max slaves will be used later on

  
  while (layoutsRecieved < maxSlaves)
  {
  Serial.println("----PRINTING SLAVE-"+String(layoutsRecieved)+" LAYOUT----");
    for (int row = 0; row < 6; row++)
    {
      Wire.requestFrom(layoutsRecieved, 14);

      if (Wire.available() > 0)
      {
        readLayout();
        layoutRow++;
      }


    }
    layoutsRecieved++;
 Serial.println(" ");
  
  }

  
}

void loop()
{
  if(VERBOSE){delay(1000);}
  // put trigger in for setting config mode = true
  // call setconfig(boardID)  function boardID is sent from PC
  // sequencily set each element of the array
  if(VERBOSE){Serial.println("\r");}
  //delay(10);
  //Serial.println("running");

  for (int currentSlave = 1; currentSlave < maxSlaves; currentSlave++)
  {
    if(VERBOSE){Serial.println("Requesing Data from: Slave " + String(currentSlave));}

    // Wire.beginTransmission(currentSlave); // start transmission to first slave board
    // Wire.write(10);                       // tell sllave to prepare load size
    // Serial.println("Prep Load Size");
    // Wire.endTransmission(); // end transmission

    Wire.requestFrom(currentSlave, 1); // address, quantity -- requesting load size
    if(VERBOSE){Serial.println("Current wire avalible: " + String(Wire.available()));}

    if (Wire.available() > 0)
    {
      LoadSize = Wire.read(); // read the load size from the slave
      // slaveRuns = Wire.read();
      if(VERBOSE){Serial.println("Load Size Digital:" + String(LoadSize));}
      // Serial.println("slave runs: " + String(slaveRuns));

      if (LoadSize > 0)
      {
        if (LoadSize != 255)
        {

          // Wire.requestFrom(currentSlave,digitalLoadSize + analogLoadSize); // address, quantity --reqesting load from slave
          // DIGITAL SECTION
          while (LoadSize / 30 > 1) // start generating runs
          {
            Runs++;
            LoadSize = LoadSize - 30;
          }
          while (Runs > 0 || LoadSize > 0)
          {
            if (Runs < 1 && LoadSize > 0)
            {
              // Wire.requestFrom(currentSlave, LoadSize);
              if(VERBOSE){Serial.println("Requesting Load: " + String(LoadSize));}
              for (int i = 0; i < LoadSize / ChangeWeight; i++)
              { // read the analog section of the load.
                // Serial.println("Load Size: " + String(LoadSize));
                Wire.requestFrom(currentSlave, ChangeWeight); // requests have to be put in individualy
                // Serial.println("i Value: " + String(i));
                if (Wire.available() > 0)
                {
                  readData();
                }
              }
              LoadSize = 0;
            }

            if (Runs > 0) // refills the runs with another 30 and takes a round off the runs
            {
              // Wire.requestFrom(currentSlave, 30);
              if(VERBOSE){Serial.println("Requesting Load: 30");}
              if(VERBOSE){Serial.println("Runs left: " + String(Runs));}
              Runs--; // converts a run back into a load size of 30
              LoadSize = 30;
            }

            // Serial.println("Wire avalible: " + String(Wire.available()));
            // Serial.println("Load Size: " + String(LoadSize));
          }
        }
      }
    }
  }
}
