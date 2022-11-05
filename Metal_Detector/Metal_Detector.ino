#include <ADC.h>
#include <math.h>
int n_test = 1000;                        //the number of samples we are going to take at one go 
int n; 
const int n_max = 1620;
int driveCoil = 6;                       //defining which pin the drive coil circuitry will be attached to 
//setting up the sine and cosine waves
const float pi = 3.14159267;             //initiating a value for pi 
int rA0[n_max];
int rA1[n_max];
int buzzer = 9;                          //initialising the pin on which the buzzer is attached to
float f0 = 20000;                        //setting the frequency of 20kHz 
int t0 = (1/f0)*pow(10,6);               //finding how long one cycle will take at 20kHz
float w = 2*pi*f0;

//LED pin setup 
int RLED = 18;
int OLED = 17;
int YLED = 16;
int GLED = 20;
int WLED = 21;

int noBit = 12;
//variable to store the value of I and Q for both the received and transmitted signals
//received signal 
float I_rx = 0;
float Q_rx = 0;
//transmitted signal
float I_tx = 0;
float Q_tx = 0;  

float freq;                               //variable to hold a value of frequency which is used to change the buzzers pwm frequency
float f_sample;                           //used to store the sampling frequency
float t_sample;                           //used to store the period of the sampling frequency 
float phaseShift;
float samplingTime = 10;                  //time in milliseconds

float amp_ref;                            //placeholder for the value of the reference amplitude
float phase_ref;                          //placeholder for the value of the reference phase
int nRef = 10;                            //how many times the code should run during calibration
float Iref;                               //placeholder for the value of the reference I value
float Qref;                               //placeholder for the value of the reference Q value

void setup() {
  // put your setup code here, to run once:
  
  analogWriteFrequency(driveCoil, 20000);   //setting the frequency to 20kHz 
  analogReadResolution(noBit); // Can be 8, 10, 12, 16
  analogReadAveraging(1); // can be 1, 2, 4, 8, 16 or 32
  analogReference(0); // Voltage reference
  Serial.begin(9600);

  //setting up the pinModes used in the metal detector
  pinMode(driveCoil, OUTPUT);               //setting the pin to an output       
  pinMode(A0, INPUT);                       //setting A0 as an input pin to recieve signals 
  pinMode(A1, INPUT);                       //setting A0 as an input pin to recieve a signal
  analogWrite(driveCoil, 127);              //Setting a PWM signal with a 50% duty cycle
  
  //LED setup
  pinMode(RLED, OUTPUT);
  pinMode(OLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(YLED, OUTPUT);
  pinMode(WLED, OUTPUT);
  digitalWrite(WLED, HIGH);

  //getting the sample rate frequency to be used to generate the sine and cos waves 
  for (int i =0; i<10; i++){
    f_sample = RateSampling();
  }
  t_sample = 1/f_sample;

  n = round((samplingTime*pow(10,-3))/t_sample);

  phaseShift =(f0/f_sample)*(360/2);
  ref();
//  Serial.println(amp_ref);
//  Serial.println(phase_ref); 
}

float ref(){
   amp_ref = 0; 
   phase_ref = 0; 
   for(int j=0; j<nRef; j++){
     for(int i = 0; i<n; i++){
      rA0[i] = analogRead(A0);
      rA1[i] = analogRead(A1);  
     }
     for(int j = 0; j<n; j++){
         float x_v = 3.3*(rA0[j]/pow(2,noBit))-1.65;      //converting the bit value of the received signal to a voltage
         float x_tx = 3.3*(rA1[j]/pow(2,noBit))-1.65;    //converting the bit value of the transmitted signal to a voltage
         I_rx = I_rx + x_v*(2*cos(w*j*t_sample));
         Q_rx = Q_rx + x_v*(-2*sin(w*j*t_sample));
    
         I_tx = I_tx + x_tx*(2*cos(w*j*t_sample));
         Q_tx = Q_tx + x_tx*(-2*sin(w*j*t_sample));
      }
      //Simulating a low pass filter through weighted average
      float I_filt = I_rx/n;                     //getting the weighted average of the I for the recieved signal.  
      float Q_filt = Q_rx/n;                     //getting the weighted average of the Q for the recieved signal. 
    
      float I_filt_tx = I_tx/n;                  //getting the weighted average of the I for the transmitted signal. 
      float Q_filt_tx = Q_tx/n;                  //getting the weighted average of the Q for the transmitted signal.
    
      float amp_rx = sqrt((I_filt*I_filt)+(Q_filt*Q_filt));      //finding the amplitude of the received signal 
      
      float ang_rx = atan2(Q_filt, I_filt);          //finding the phase of the received signal 
      
      float amp_tx = sqrt((I_filt_tx*I_filt_tx)+(Q_filt_tx*Q_filt_tx)); //finding the amplitude of the transmitted signal 
      float ang_tx = atan2(Q_filt_tx, I_filt_tx);    //finding the phase of the transmitted signal
    
      float phaseDiff = ang_tx-ang_rx - phaseShift;
      float amp = amp_rx/amp_tx;

      //case if the phase is a negative number
      if(phaseDiff<0){
          phaseDiff = 360 + phaseDiff;  
      }
      else{
          phaseDiff = phaseDiff;  
      }
      amp_ref = amp_ref + amp; 
      phase_ref = phase_ref + phaseDiff;
   }
   amp_ref = amp_ref/nRef; 
   phase_ref = phase_ref/nRef;
   //getting the I and Q values 
   Iref = amp_ref*cos(phase_ref);
   Qref = amp_ref*sin(phase_ref);

}


//function to get the sample rate frequency of the analog reads 
float RateSampling(){
  float start = micros();
  int t1;
  t1 = micros();    //getting the time
  for(int i =0; i <n_test;i++){
    int x = analogRead(A0);                 //Reading in the received signal on the recieve coil                                         
    int x_tx = analogRead(A1);              //Reading in the current through the coil 
  }
  float t2 = micros(); 
  float t_total = t2-t1;                    //getting the time it takes
  float f_rate = 1/((t_total*pow(10,-6))/n_test); //frequency relating to the time taken
  //Serial.println(t_total);
//  Serial.println(f_rate);
  return f_rate;
}



void loop() {
  // put your main code here, to run repeatedly
  //reading the transmitted and received signal
  for(int i = 0; i<n; i++){
    rA0[i] = analogRead(A0);
    rA1[i] = analogRead(A1);  
  }
  for(int j = 0; j<n; j++){
     float x_v = 3.3*(rA0[j]/pow(2,noBit))-1.65;          //converting the bit value of the received signal to a voltage
     float x_tx = 3.3*(rA1[j]/pow(2,noBit))-1.65;    //converting the bit value of the transmitted signal to a voltage
     I_rx = I_rx + x_v*(2*cos(w*j*t_sample));
     Q_rx = Q_rx + x_v*(-2*sin(w*j*t_sample));

     I_tx = I_tx + x_tx*(2*cos(w*j*t_sample));
     Q_tx = Q_tx + x_tx*(-2*sin(w*j*t_sample));
  }
  //Simulating a low pass filter
  float I_filt = I_rx/n;                     //getting the weighted average of the I for the received signal.   
  float Q_filt = Q_rx/n;                     //getting the weighted average of the Q for the received signal. 
  float I_filt_tx = I_tx/n;                  //getting the weighted average of the I for the transmitted signal. 
  float Q_filt_tx = Q_tx/n;                  //getting the weighted average of the Q for the transmitted signal. 

  float amp_rx = sqrt((I_filt*I_filt)+(Q_filt*Q_filt));      //finding the amplitude of the recieved signal 
  float ang_rx = atan2(Q_filt, I_filt);          //finding the phase of the received signal 
  float amp_tx = sqrt((I_filt_tx*I_filt_tx)+(Q_filt_tx*Q_filt_tx));//finding the amplitude of the transmitted signal 
  float ang_tx = atan2(Q_filt_tx, I_filt_tx);    //finding the phase of the transmitted signal

  float phaseDiff = ang_tx-ang_rx;
  float amp = amp_rx/amp_tx;

  float Inew = amp*cos(phaseDiff);
  float Qnew = amp*sin(phaseDiff);
  float Idiff = Inew - Iref;
  float Qdiff = Qnew - Qref;

  float amp_obj = sqrt((Idiff*Idiff)+(Qdiff*Qdiff));
  float ang_obj = atan2(Qdiff, Idiff); 
  
//  Serial.print("The Amplitude is: ");
//    Serial.println(amp_obj);
//    Serial.println(ang_obj);
//  Serial.print(" The phase difference is: ");
  //case if the phase is a negative number
  ang_obj = ang_obj*180/pi;
  if(ang_obj<0){
      ang_obj = 360 + ang_obj;  
  }
  else{
      ang_obj = ang_obj;  
  }
//  Serial.print(ang_obj);
//  Serial.print(" ");
//  Serial.println(phaseDiff);
//    Serial.println(f_sample);
   //function to generate the value to be played through the speaker
   float amp_obj_sound = (pow(amp_obj,0.25)/pow(1.5,0.25))*1.5;
//   Serial.println(amp_obj);
    
   freq=map(amp_obj_sound,0,1.5,1,1000);      //mapping the calculated value to a frequency
   tone(buzzer,freq);                         //generating asound on the speaker
  //logic statements to determine what the LEDs need to turn on when 
  if(amp_obj>0.04 and amp_obj < 1.54){
    digitalWrite(YLED, HIGH);
    if(amp_obj>0.6 and amp_obj< 1.54){
     //FERROUS MATERIALS 
     if(ang_obj>5 and ang_obj<20){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>30 and ang_obj<40){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>50 and ang_obj<69){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>77 and ang_obj<86){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>110 and ang_obj<130){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>145 and ang_obj<160){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>280 and ang_obj<290){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     else if(ang_obj>310 and ang_obj<360){
        digitalWrite(OLED, HIGH);
        digitalWrite(GLED, LOW);     
     }
     
     //NON- FERROUS MATERIALS
     else if(ang_obj>20 and ang_obj<30){
        digitalWrite(GLED, HIGH);
        digitalWrite(OLED, LOW);  
     }
     else if(ang_obj>60 and ang_obj<75){
        digitalWrite(GLED, HIGH);
        digitalWrite(OLED, LOW);  
     }
     else if(ang_obj>90 and ang_obj<110){
        digitalWrite(GLED, HIGH);
        digitalWrite(OLED, LOW);  
     }
     else if(ang_obj>260 and ang_obj<270){
        digitalWrite(GLED, HIGH);
        digitalWrite(OLED, LOW);  
     }
  }}
  //saturation case
  else if(amp_obj>1.55){
    digitalWrite(RLED, HIGH);
    digitalWrite(GLED, LOW);
    digitalWrite(YLED, LOW);
    digitalWrite(OLED, LOW);  
  }
  //no changes
  else{
    digitalWrite(YLED, LOW);
    digitalWrite(GLED, LOW);
    digitalWrite(RLED, LOW);
    digitalWrite(OLED, LOW); 
  }

  //resetting the values 
  I_tx = 0;
  Q_tx = 0;
  I_rx = 0;
  Q_rx = 0;
}
