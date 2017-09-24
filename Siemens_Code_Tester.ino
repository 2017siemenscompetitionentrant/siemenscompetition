/*

  digitalWrite(inPin1, LOW);   //make longer
  digitalWrite(inPin2, HIGH);
  
  digitalWrite(inPin1, HIGH);  //make shorter
  digitalWrite(inPin2, LOW);

*/

//Declare L298N Dual H-Bridge Motor Controller
int inPin1 = 32;
int inPin2 = 31;
int enPinA = 30;
int feedback = 2;
int val = 0;

int resistor = 0;
int gaugeresistance = 1;

    
// Define variables globally
double gaugelength = 6.5; //add .5 to gauge length to account for extra length of actuator pulling point
double gaugeheight = 0; // calculates height of gauge needed
double pi = 3.14159; 
double newlengthpull = 0; //length to which actuator needs to pull (in voltage units)
double angle = 0;  // current angle at which the gauge is at
double newangle = 0;  //angle at which the gauge needs to be pulled to
double voltageunits = 0;  //voltage units of length at which the gauge needs to be pulled
double currentposition = 0; // position it is at currently (in voltage units)
double actuatorbaseheight = 0; // baseheight of actuator
double basevoltageunits = 0; //voltage unit conversion of baseheight of actuator
double resistorvalue = 0; //resistorvalue that is measured
double gaugevalue = 0; //gauge value that is measured
double actuatorlength = 0; //actuatorlength to be printed to array
double resistancechange = 0; //change in resistance that is printed to array
double averageresistancechange = 0; // average resistance change
double data[100][2]; //make starting array 
int x = 0;
//73.345x + 28.464
//74.098x + 31.106
//0.0135x - 0.4129

//--------------------------------------------------------------------------------------------

void reset(){
  actuatorbaseheight = gaugeheight - 19.8;
  basevoltageunits = round(74.098 * actuatorbaseheight + 31.106);
  Serial.println((String)basevoltageunits + " basevoltageunits");
  currentposition = analogRead(feedback);
  if (currentposition == basevoltageunits){
    digitalWrite(inPin1, LOW); 
    digitalWrite(inPin2, LOW);
    Serial.println("perfect");
  }
  else if (currentposition > basevoltageunits){
    do{
      digitalWrite(inPin1, HIGH); //make shorter
      digitalWrite(inPin2, LOW);
      currentposition = analogRead(feedback);
    } while (currentposition > basevoltageunits);
    digitalWrite(inPin1, LOW);
    Serial.println("made shorter");
  }
  else if (currentposition < basevoltageunits){
    do{
      digitalWrite(inPin1, LOW); //make longer
      digitalWrite(inPin2, HIGH);
      currentposition = analogRead(feedback);
    } while (currentposition < basevoltageunits);
    digitalWrite(inPin2, LOW);
    Serial.println("made longer");
  }
  currentposition = analogRead(feedback);
  Serial.println((String)currentposition + " resetcurrentposition");
  savevalues();
}




void angletolength(){
  newangle = angle * pi / 180;    //radian converter
  newlengthpull = gaugelength * tan(newangle);
}

void lengthtovoltageunits(){
  voltageunits = 74.098 * newlengthpull + 31.106;
  
}

void resistancecalc(){
  resistorvalue = analogRead(resistor);
  gaugevalue = analogRead(gaugeresistance);
  resistancechange = 10000 * gaugevalue / ((resistorvalue * 2) - gaugevalue);
}
void savevalues(){
  delay(1000);
  Serial.println((String)x + "x");
  double total = 0;
  for (int i = 0; i < 100; i++){
    resistancecalc();
    total += resistancechange;
  }
  averageresistancechange = total/100;
  Serial.println((String)averageresistancechange + " averageresistancechange");
  data[x][1] = averageresistancechange;//writing values into array
  //actuatorlength = 0.0135 * currentposition - 0.4129;
  data[x][0] = angle; //writing values into array
  x += 1;
}
void goingup(){ 
  Serial.println((String)x + "x");
  data[x][1] = -5;
  data[x][0] = -5;
  x += 1;
}




void downanglecalculator(){
  angle += 2;
  angletolength();
  lengthtovoltageunits();
  voltageunits = basevoltageunits - round(voltageunits);
  retract();

}
void retract(){
  currentposition = analogRead(feedback);
  do{
      digitalWrite(inPin1, HIGH); //make shorter
      digitalWrite(inPin2, LOW);
      currentposition = analogRead(feedback);
    } while (currentposition > voltageunits);
  digitalWrite(inPin1, LOW);
  savevalues();
  delay(1000);
}





void upanglecalculator(){
  angle -= 2;
  angletolength();
  lengthtovoltageunits();
  voltageunits = basevoltageunits - round(voltageunits);
  extend();
}
void extend(){
  currentposition = analogRead(feedback);
  do{
     digitalWrite(inPin1, LOW); //make longer
     digitalWrite(inPin2, HIGH);
     currentposition = analogRead(feedback);
    } while (currentposition < voltageunits);
  digitalWrite(inPin2, LOW);
  savevalues();
  delay(1000);
}


//--------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);  
  Serial.println("start");
  
  //Define L298N Dual H-Bridge Motor Controller Pins
  pinMode(inPin1, OUTPUT); //if high, shorter
  pinMode(inPin2, OUTPUT); //if high, longer
  pinMode(enPinA, OUTPUT);

  digitalWrite(enPinA, HIGH); // enabling all pins
  digitalWrite(inPin1, LOW);  // turning off all pins for now
  digitalWrite(inPin2, LOW); 

  analogReference(EXTERNAL); //defining analog pins for resistance change

  gaugeheight = ceil(gaugelength * tan(pi/3) + 18); //calibrated gaugeheight with actuator
  //the plus value doesnt matter as long as it works and goes all the way down
  Serial.println((String)gaugeheight + " gaugeheight");
  


  for (int i = 0; i < 100; i++){
    for (int j = 0; j < 2; j++){
      data[i][j] = -1 ;
    }
  }


  currentposition = analogRead(feedback);  // read the value from the sensor:
  reset();  // go to angle 0
  currentposition = analogRead(feedback);
  Serial.println(currentposition);
  delay(1000);
  

  do {
    Serial.println("going down");
    downanglecalculator();
  } while (angle != 20);
  delay(1000);
  
  goingup(); //to separate going down values to going up values
  savevalues(); //second reading of bottom value for comparison
  do {
    Serial.println("going up");
    upanglecalculator();
  } while (angle != 0);
  delay(1000);

  
  currentposition = analogRead(feedback);
  Serial.println((String)currentposition + " finalposition");




  Serial.println("Printing values-----------------------------");
  


  Serial.println("angle");
  for(int i = 0; i < sizeof(data); i++){ //printing values of array, actuatorlength
    if(data[i][0] == -1) break;
    Serial.println(data[i][0]);
  }

  Serial.println("ohm value");
  for(int i = 0; i < sizeof(data); i++){  //printing values of array, resistancechange
    if(data[i][1] == -1) break;
    Serial.println(data[i][1]);
  }

  Serial.println("done");
  digitalWrite(enPinA, LOW);

}




                  






void loop(){

}

