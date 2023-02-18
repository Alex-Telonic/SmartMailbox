/*
Reads out proximity sensor and starts advertising when something is close.
*/

#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>


BLEService proximityService("1908A897-CABB-4D43-AFF0-503335941BAC"); // BLE Proximity Service

// BLE Proximity Read and Notify Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic readProximityCharacteristic("467A9870-9CCD-47E7-946F-83D1B086C459", BLERead | BLENotify | BLEBroadcast);

int advertise_state = 0;

//if proximity goes above or below threshold a fill event is triggert
int min_threshold_prox;
// Set the number of readings to use for the threshold average
int numReadings = 100;
int sample_delay_for_std = 100;


void setup() {
  
    // Initialize the serial communication
    Serial.begin(9600);
    delay(5000);
    Serial.println("Starting mailbox detection...");
  
    if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
  }
  
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");

    while (1);
  }
  
    // set advertised local name and service UUID:
  BLE.setLocalName("ASch-AdvertiseWhenFilled");
  BLE.setAdvertisedService(proximityService);

  // add the characteristic to the service
  proximityService.addCharacteristic(readProximityCharacteristic);

  // add service
  BLE.addService(proximityService);
  
  //broadcast value in advertising already ?
  readProximityCharacteristic.broadcast();

  // start advertising
  //BLE.advertise();
  // Set up variables for the readings and the total
  

  // Read the sensor values and add them to the total
  //Serial.println(APDS.readProximity());
  
}

    bool initiated = false;
    float std_dev;
    float variance;

  


void loop() {

    
  
  
  
  // check if a proximity reading is available
  if (APDS.proximityAvailable()) {
    
    int readings[numReadings];
    int total = 0;

    
    for (int i = 0; i < numReadings; i++) {
      int sensorValue = APDS.readProximity();
      readings[i] = sensorValue;
      //Serial.println(readings[i]);
      total += sensorValue;
      delay(sample_delay_for_std);
    }

    // Calculate the average
    int proximity_average = total / numReadings;
    Serial.println("Current proximity average is:");
    Serial.println(proximity_average);

    
    if (!initiated) {
      //delay(10000);
      Serial.println("Setting threshold and std_dev...");
      
      for (int i = 0; i < numReadings; i++) {
        variance += pow(readings[i] - proximity_average, 2);
      }
      variance /= (numReadings - 1); // calculate the variance
      
      std_dev = sqrt(variance); // calculate the standard deviation


        // get the minimum value
      int current_min = readings[0]; // assume the first value is the minimum
      Serial.println("Searching for minimal proximity threshold in following values:");
      for (int i = 1; i < numReadings; i++) {
        Serial.println(readings[i]);
        if (readings[i] < current_min) {
          current_min = readings[i]; // update the minimum value
        }
      }

      min_threshold_prox = current_min;
      Serial.print("Current min proximity at: ");
      Serial.println(min_threshold_prox);

      Serial.print("Current variance at: ");
      Serial.println(variance);

      //reset parameter for detection
      sample_delay_for_std = 100;
      numReadings = 20;
      initiated = true;
      Serial.println("Threshold and std_dev set.");
    } else {
    
    //Serial.println(proximity_average);
    
    
    // print value to the Serial Monitor
    //Serial.println(proximity);
    
      //Serial.println("Checking threshold...");
      Serial.print("Threshold prox is: ");
      Serial.println(min_threshold_prox);
      Serial.print("Current prox is: ");
      Serial.println(proximity_average);
      if (proximity_average < min_threshold_prox) {
        //if something is in close proximity (mailbox is filled) start advertising
        BLE.advertise();
        Serial.print("Current average at: ");
        Serial.println(proximity_average);
        Serial.print("versus the threshold is at: ");
        Serial.println(min_threshold_prox);
        Serial.println("mail delivered, advertising...");
      } else {
        //mailbox is empty 
        Serial.println("Mailbox empty. not advertising.");
        BLE.stopAdvertise();
      }
    }
    
  
  }
    
}
