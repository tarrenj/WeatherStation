#include <Wire.h> //I2C needed for sensors
#include "SparkFunMPL3115A2.h" //Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFun_Si7021_Breakout_Library.h" //Humidity sensor - Search "SparkFun Si7021" and install from Library Manager

MPL3115A2 PressureLib; //Create an instance of the pressure sensor
Weather HumidityLib;//Create an instance of the humidity sensor

//Hardware pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;

const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;

// Structure for payload to Pi
struct payload {
  float humidity;
  float humidity_temp;
  float pressure;
  float pressure_temp;
  float light;
  float battery;
};    
void setup() {
  Serial.begin(9600); // For debugging the things
  
  pinMode(STAT_BLUE, OUTPUT); //Status LED Blue
  pinMode(STAT_GREEN, OUTPUT); //Status LED Green

  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);
  
  // Init and config the sensors
  PressureLib.begin();
  PressureLib.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  PressureLib.setOversampleRate(7); // Set Oversample to the recommended 128
  PressureLib.enableEventFlags(); // Enable all three pressure and temp event flags
  
  HumidityLib.begin();
  
  // Test humidity sensor
  while (true){
    float humidity = HumidityLib.getRH();
    int count = 0;
    int failure_limit = 10 // Tries until program dies
    // Check if getRH returned error code, or we've tried failure_limit times
    if (humidity == 998 && count < failure_limit) {
      // Report
      Serial.println("Humidity Sensor fucked"); // Will need to change for Pi.
      // Reinit
      PressureLib.begin(); // Get sensor online
      PressureLib.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
      PressureLib.setOversampleRate(7); // Set Oversample to the recommended 128
      PressureLib.enableEventFlags(); // Enable all three pressure and temp event flags 
    }
    else {
      if (count == failure_limit) {
        Serial.println("Gave up, it's all fucked.");
      }
      break;
    }
  }
}

float get_voltage(String type) {
  float reference_v = analogRead(REFERENCE_3V3); // Calibrate 
  float raw_v = 0.0; // Failure value
  float calculated_v = 0.0; // Failure value
  
  if (type == "battery") {
    raw_v = analogRead(BATT);
  }
  else if (type == "light") {
    raw_v = analogRead(LIGHT);
  }
  
  raw_v = (3.30 / reference_v) * raw_v; // Calibrate
  
  if (type == "battery") {
    calculated_v *= 4.90; // (3.9k+1k)/1k - multiple BATT voltage by the voltage divider to get actual system voltage
  }
  else if (type == "light") {
    calculated_v = reference_v * raw_v;
  }
  
  return calculated_v;
}

void loop() {
  // Init and populate struct full of data
  payload c_payload;
  c_payload.humidity = HumidityLib.getRH();
  c_payload.humidity_temp = HumidityLib.getTemp();
  c_payload.pressure = PressureLib.readPressure();
  c_payload.pressure_temp = PressureLib.readTemp();
  String battery = "battery"; // String for voltage type
  String light = "light"; // String for voltage type
  c_payload.battery = get_voltage(battery);
  c_payload.light = get_voltage(battery);
  
  // Send it to the Pi over UART
  
  delay(100); // Time (milliseconds) between messages to Pi
}
