/*
Reads out proximity sensor and starts advertising when something is close.
*/

#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>
#include <Arduino_LSM9DS1.h>

//ble
BLEService proximityService("1908A897-CABB-4D43-AFF0-503335941BAC"); // BLE Proximity Service
// BLE Proximity Read and Notify Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic readProximityCharacteristic("467A9870-9CCD-47E7-946F-83D1B086C459", BLERead | BLENotify | BLEBroadcast);

//detection
//initialise detection trigger threshold
int proximity_treshold = 255;
boolean proximity_threshold_initiated = false;
//Set the number of readings to use for setting the proximity triggert threshold 
int const numReadings_threshold = 100; 
int const sample_delay_for_threshold = 10; //Sampling 500/10, so 50 seconds
//Set sample settings for proximity detection trigger
int const numReadings_detection = 10; 
int const sample_delay_detection = 0; //Sample 10 values for throw detection
int const detection_noise = 10; //filtering noise

//universal
int const wait_time_before_setup = 0;


void setup() {
  
  // Initialize the serial communication
  Serial.begin(9600);
  Serial.print("Waiting ");
  Serial.print(wait_time_before_setup);
  Serial.println(" milliseconds");
  delay(wait_time_before_setup);
  Serial.println("Starting mailbox detection...");
  
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
    while (1);
  }
  
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");
    while (1);
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
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

//initiate acceleration values
float x_prev = 0;
float y_prev = 0; 
float z_prev = 0;
float x = 0;
float y = 0;
float z = 0;

float const x_thresh = 0.2; //x opening threshold
float const y_tresh = 0.2; //y opening threshold
float const z_tresh = 0.3; //z opening threshold

//

void loop() {


  //read accelerometer data. As long as it is not zero wait and stop advertise
  if (IMU.accelerationAvailable()) {

    IMU.readAcceleration(x, y, z);
    

    //check for opening, else go in detection mode
    if ( fabs(x - x_prev) > x_thresh || fabs(y - y_prev) > y_tresh || fabs(z - z_prev) > z_tresh ) {
      //Serial.print(fabs(y - y_prev));
      //Serial.print('\t');
      //Serial.print(y);
      //Serial.print('\t');
      //Serial.println(z);

      //mailbox opened
      BLE.stopAdvertise(); //stop advertising
      proximity_threshold_initiated = false; //reset detection by triggering new proximity threshold
      //reset acceleromater values
      x_prev = x;
      y_prev = y;
      z_prev = z;
      Serial.println("Mailbox opened, stopped advertising...");
      delay(2000); //wait for 2 seconds to prevent some weird behaviour of triggering the proximity after the box has be closed. (prevent false positives)
    } else {
      //detection mode
      if (APDS.proximityAvailable()) {

        //if treshold not yet set
        if (!proximity_threshold_initiated) {
          //setting threshold
          Serial.println("Setting proximity threshold...");
          proximity_treshold = get_proximity(numReadings_threshold, sample_delay_for_threshold);
          proximity_threshold_initiated = true;
          Serial.print("Proximity threshold set at: ");
          Serial.println(proximity_treshold);
          Serial.println("starting detection");
        }
        //Serial.print("Current Proximity: ");
        //Serial.println(get_proximity(numReadings_detection, sample_delay_detection));
        if (get_proximity(numReadings_detection, sample_delay_detection) + detection_noise < proximity_treshold) {
          //mail detected
          Serial.println("mail delivered, starting to advertise...");
          BLE.advertise();
          //Serial.println("mail delivered, advertising...");
          delay(5000); //wait 5 seconds to prevent acceleration triggered by mail man to reset advertising. (prevent false negatives)
        }

      }

    }






  }



  /*
  Serial.print(x);
  Serial.print('\t');
  Serial.print(y);
  Serial.print('\t');
  Serial.println(z);
  */




    
  //when accelerometer is zero then wait for proximity triggers


  //if proximity is triggert, advertise and read accelerometer for opening event again
    
}


/*
Takes in the number of readings and the delay 
to deduce an average proximity value
*/
int get_proximity(int numReadings, int delay_value){
  int proximity_average;

  int readings[numReadings];
  int total = 0;

  for (int i = 0; i < numReadings; i++) {
    int sensorValue = APDS.readProximity();
    readings[i] = sensorValue;
    //Serial.println(readings[i]);
    total += sensorValue;
    delay(delay_value);
  }

    // Calculate the average
    return proximity_average = total / numReadings;



}