#include <stdio.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define ANEMOMETER_PIN A0 
#define DHTPIN 2        
#define DHTTYPE DHT22  
 
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial BTserial(3, 4);

double calculatePMV(double tempAvg, double humAvg, double airFlowAvg);

void setup() {
  Serial.begin(9600);
  BTserial.begin(9600);
  dht.begin();
  
}

void loop() {

  double temp[5];
  double hum[5]; 
  double airFlow[5];	 
  double tempAvg = 0.0; 
  double humAvg = 0.0;
  double airFlowAvg = 0.0; 
 
  for(int i=0; i<5; i++) {
    delay(60000);
    temp[i] = dht.readTemperature(); 
    hum[i] = dht.readHumidity();
    int sensorValue = analogRead(ANEMOMETER_PIN);
    double voltage = sensorValue * (5.0 / 1023.0);
    double windSpeed = voltage * 0.4;
    airFlow[i] = windSpeed;     
    Serial.print("Temperature: ");
    Serial.print(temp[i]);
    Serial.print(" °C, Humidity: ");
    Serial.print(hum[i]);
    Serial.print(" %, AirSpeed: ");
    Serial.print(airFlow[i]);
    Serial.println(" m/s");
    
  }
 
  double tempMax = temp[0];
  double tempMin = temp[0];
  double humMax = hum[0];
  double humMin = hum[0];
  double airFlowMax = airFlow[0];
  double airFlowMin = airFlow[0];
  for(int i=1; i<5; i++) {
    if(temp[i] > tempMax) tempMax = temp[i];
    if(temp[i] < tempMin) tempMin = temp[i];
    if(hum[i] > humMax) humMax = hum[i];
    if(hum[i] < humMin) humMin = hum[i];
    if(airFlow[i] > airFlowMax) airFlowMax = airFlow[i];
    if(airFlow[i] < airFlowMin) airFlowMin = airFlow[i];
  }

  for(int i=0; i<5; i++) {
    tempAvg += temp[i];
    humAvg += hum[i];
    airFlowAvg += airFlow[i];
  }
  tempAvg /= 5.0;
  humAvg /= 5.0;
  airFlowAvg /= 5.0;

  double pmv = calculatePMV(tempAvg, humAvg, airFlowAvg);
  

  BTserial.println("ASHRAE 55 Comfortable Scale: ");
  BTserial.println("-3.0______-2.0______-1.0______0.0______1.0______2.0______3.0");
  BTserial.println("  |         |         |        |        |        |        |");
  BTserial.println(" Cold      Cool    Slightly  Neutral  Slightly  Warm    Hot");
  BTserial.println("                     Cool              Warm");
  BTserial.print("Your Comfort Range: ");
  BTserial.println(pmv,2);
  Serial.print("PMV: ");
  Serial.println(pmv,2);
}

double calculatePMV(double tempAvg, double humAvg, double airFlowAvg) {

    const double M = 58.15;
    const double W = 3.96 * pow(10, -8);
    const double ICL = 0.155;
    const double FCL = 1.0;
    const double HCN = 2.38 * pow(10, -3);
    const double TA = tempAvg + 273.15;  
   
    double airVelocity = 0.303 * airFlowAvg;

    double airTemperature = 0.33 * (TA - 273.0) - 184.0;


    double ICL_cl = 1;


    double metabolicRate = 1.0;


    double vaporPressure = 0.0;
    double saturationPressure = 6.11 * exp(5417.7530 * ((1 / 273.16) - (1 / TA)));
    double relativeHumidity = humAvg / 100.0;
    double vaporPressureSaturation = relativeHumidity * saturationPressure;
    double vaporPressureDifference = vaporPressureSaturation - vaporPressure;


    double pmv = (metabolicRate - W * (M - vaporPressureDifference) - HCN * (ICL_cl - ICL) - FCL * airVelocity * (TA - airTemperature)) / (metabolicRate * (0.42 * (M - vaporPressureDifference) + HCN * (ICL_cl - ICL)));

    return pmv;
}

