// ADS1256 code for Teensy from https://github.com/mbilsky/TeensyADS1256
// requires "ads1256_constants.ino" and "ads_1256_stuff.ino" in same folder
// mods by J.Beale 27-Feb-2019

#include <SPI.h>

//      ADS_CLK    - pin 13
//      ADS_DIN    - pin 11 (MOSI)
//      ADS_DOUT   - pin 12 (MISO)
#define ADS_RST_PIN    8 // ADS1256 reset pin
#define ADS_RDY_PIN   22 // ADS1256 data ready
#define ADS_CS_PIN    21 // ADS1256 chip select

#define SAMPLES 25               // how many readings for stats calc
#define COUNTSPERVOLT (1658385)  // nominally (2^22 / 2.50 V) but that is not exact

double tDrift = 0.0;  // total drift since first reading

const int LED1 = 13;         // this pin both SPI clock and Arduino LED
float firstRead = 0.00;      // store first (averaged) ADC reading
bool startflag = true;       // are we doing the first reading?

// =========================================================================
void setup() {
  
  pinMode(LED1,OUTPUT);       // for visual startup signal
  
  // initialize the ADS chip
  pinMode(ADS_CS_PIN, OUTPUT);
  pinMode(ADS_RDY_PIN, INPUT);
  pinMode(ADS_RST_PIN, OUTPUT);
  digitalWrite(ADS_CS_PIN, 1);   // CS is active low
  digitalWrite(ADS_RST_PIN, 1);  // RST is active low

  for (int i=0;i<2;i++) {
    digitalWrite(LED1,HIGH);   
    delay(250);                // LED on for 1 second
    digitalWrite(LED1,LOW);  
    delay(250);                // LED on for 1 second
  }

  Serial.begin(115200);
  SPI.begin();
  Serial.println("V, stdev_uV, pk_uV, drift_uV");

  initADS();  // sets input channel, sample rate, PGA gain, buffer, etc.
}

// ------------------------------------------------------------------------------
void loop() {

  double sMax,sMin;  // max and min values for peak-to-peak amplitude
  long n;            // count of how many readings so far
  double x,datSum,mean,delta,m2,variance,stdev;  // to calculate standard deviation

  sMax = -8388608 / (double) COUNTSPERVOLT;  // ADC range is +/- 2E23 = 8388608
  sMin = 8388608 / (double) COUNTSPERVOLT;

  datSum = 0;   // sum of readings
  n = 0;     // have not made any ADC readings yet
  mean = 0;  // start off with running mean at zero
  m2 = 0;    // incremental variance accumulator  

  for (int i=0;i<SAMPLES; i++) {
        x = ((double) read_Value()) / COUNTSPERVOLT;  // returns signed 32-bit int from 24 bit ADC
        datSum += x;  // scaled to volts;
        if (x > sMax) sMax = x;
        if (x < sMin) sMin = x;
        
              // from http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        n++;
        delta = x - mean;
        mean += delta/n;
        m2 += (delta * (x - mean));
  } 
  variance = m2/(n-1);     // (n-1):Sample Variance  (n): Population Variance
  stdev = sqrt(variance);  // Calculate standard deviation

  double datAvg = (1.0*datSum)/n;
  double pk = sMax-sMin;

  if ( startflag == true) {
    firstRead = datAvg;
    startflag = false;
  } else {
    tDrift = datAvg - firstRead;
  }
  Serial.print(datAvg,7);      Serial.print(", ");
  Serial.print(stdev*1E6,1);   Serial.print(", ");
  Serial.print(pk*1E6,1);      Serial.print(", ");
  Serial.print(tDrift*1E6,1);  // units of uV
  Serial.println();
  
} // end loop()
