/*
  Aquarium Control

  Controls a few aquarium parameters

  Starting with light control.
  Will control the brightness of the LED simulating the
  sun light over the day.
  With 3 LED row, starts illuminating 1st row then the
  2nd one and them the last one.

*/

// <  >

// pins attached to LED rows
int sun1 = 9;
int sun2 = 10;
int sun3 = 11;
int night = 6;                                                                                                                                 

// milis to refresh the time, for set time is 10 times faster and
// started being fastermail
int refreshTime = 1000;                                                                                                                                                                                                                                            
int refreshSetTime = 50;

int refreshHour;
unsigned long refreshHourMilis;

// to set the time
int setTimePinPlus = 2;
int setTimePinMinus = 4;
int setPin7 = 7; // on when setting time
unsigned long setTimeSpeed = 0; // after a few seconds of setting time on, speed up
int setTimeGap = 60; // value to add (or subtract) to currentTime

int bright = 100;             // maximun brightness %
int nightBright = 20;         //maximum brightness for night light %
long day = 86399 ;            // seconds a day will last
//long day = 10000 ;            // seconds a day will last, for test purposes
long sunrise = 11L * (day/24L);// counting from day variable
long sunHours =  (day/24L)*10L ;  // starting on sunrise;
long sunDelay = day/24L ;        // delay to simulating sun movement, how long after 1st set of lights turn on, will the 2nd turn on, and then the 3rd...
unsigned long sunTime ;       //CurrentTime in milis                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
long currentTime = 0;         //CurrentTime in seconds
int ledLight = 255;           // maximum value for output pin
int x = 0;                    // aux variable

char strHour[] = "00:00:00"; // receive the seconds converted to time format


void setup() {
  Serial.begin(9600);

  pinMode(sun1, OUTPUT);
  pinMode(sun2, OUTPUT);
  pinMode(sun3, OUTPUT);
  pinMode(night, OUTPUT);
  
  pinMode(setTimePinPlus, INPUT_PULLUP);
  pinMode(setTimePinMinus, INPUT);
 
  analogWrite(sun1, 0);
  analogWrite(sun2, 0);
  analogWrite(sun3, 0);
  analogWrite(night, 0);

  pinMode(7, OUTPUT);
  digitalWrite(7,LOW);
    
  sunTime = millis();
}

void loop () {
  String a;

//Serial.print (millis());

  // set time
  if ((digitalRead(setTimePinPlus) == HIGH) or (digitalRead(setTimePinMinus) == HIGH)) {
    digitalWrite(setPin7, HIGH);
    refreshHour = refreshTime / refreshSetTime;
    setTime ();

  } else {
    digitalWrite(setPin7, LOW);
    refreshHour = refreshTime;
    setTimeSpeed = 0;
    setTimeGap = 60;

  }
  // make the h:m:s, refresh time every refreshTime
  // to simulate 24h and configure the led brightness according
  if (millis() - sunTime >=  refreshTime) {
      // midnight to sunrise; night LED should increase
    if (currentTime < sunrise) {
      x = ledBright (currentTime , nightBright,1);
      analogWrite(night, x);
Serial.write("Sunrise:" );
Serial.print(x);
Serial.write(" -- ");
Serial.write("CurrentTime:" );
Serial.print(currentTime);
    } else if(currentTime> (sunrise + sunHours + 2*sunDelay)){
        // sunset to midnight, night LED shoul decrease
      x = ledBright (currentTime , nightBright,2);
      analogWrite(night, x);
Serial.write("Sunset:" );
Serial.print(x);
Serial.write(" -- ");
Serial.write("CurrentTime:" );
Serial.print(currentTime);
      
    } else {
//      analogWrite(night, 0);
        // hours of sun
      x = ledBright (currentTime, bright,3);
      analogWrite(sun1, x);
Serial.write("Sun1:" );
Serial.print(x);
Serial.write(" -- ");
      x = ledBright (currentTime - sunDelay, bright,3);
       // blue light will be equal to sun1 until sun1 is higher than nightBright, after that, blue light remains on nightBright until night light starts.
      if (x >= (nightBright*ledLight/100))
        analogWrite(night, x);
      
      analogWrite(sun2, x);
Serial.write("Sun2:" );
Serial.print(x);
Serial.write(" -- ");
      x = ledBright (currentTime - 2 * sunDelay, bright,3);
      analogWrite(sun3, x);
Serial.write("Sun3:" );
Serial.print(x);
Serial.write(" -- ");


     }

    if (currentTime++ >= day)
      currentTime = 0;
    else if (currentTime < 0)
      currentTime = day + currentTime; // avoid negative time

    sunTime = millis();
Serial.print('\n');    
  }

  // refresh to show hours in monitor, if setting the hour
  // will refresh faster
  if ( (millis() - refreshHourMilis) >= refreshHour) {
    secToHours();
    refreshHourMilis = millis();
//Serial.write(strHour);
//Serial.print('\n');
  }



}
/*
  Calculate the brightness to send to LED
  timeToCalculate => depends on the sunDelay
  whichBright => if day use bright variable, if night use nightBright variable
  nightDay => if 1 => midnight to sunrise
              if 2 => sunset to midnight
              if 3 => sun hours
*/
int ledBright (long timeToCalculate, long whichBright, int nightDay) {
  long int y = 0;
  long a = 0;
  long result = 0;

    // calculate % of brightness
    // midnight to sunrise    
  if (nightDay == 1)
    y = timeToCalculate * whichBright / sunrise  ;
    //sunset to midnight
  else if (nightDay == 2)
    y = (day-timeToCalculate) * whichBright / (day - (sunrise + sunHours))  ;
  else
      // sun hours
    y = (timeToCalculate - sunrise) *  whichBright / sunHours ;

  if (y < 50)
    a = 2L * y;
  else
      // sun hours
    if (nightDay == 3)
      a = 100L - (2L * y - 100L);
      // night hours
    else
      a = 20L - (2L * y - 20L);

  result = (a * (ledLight / 100));
    // avoid the unsigned problem with the output
  result = (result <= 0) ? 0 : result;

  return result;
}

/*
  Gets the time in seconds and format it to hh:mm:ss

*/
char* secToHours () {
  char hms[9];
  long a = 0;
  // hour, minute, second
  long h = 0;
  long m = 0;
  long s = 0;

  // convert the time we use to 24h (86400s) format
  // on final version 24h will be used
  a = currentTime * (86400 / day);
  //  a= currentTime;

  h = a / 3600; // gets the hour
  m = (a - (h * 3600)) / 60;
  s = (a - (h * 3600) - (m * 60));

  sprintf(hms, "%02ld:%02ld:%02ld", h, m, s);
  strcpy(strHour, hms);
//      Serial.write (hms);
//      Serial.write('\n');
  //    return hms;


}

int setTime() {
  int addMinus = 1; // if 1 add, if -1 subtract

  addMinus = digitalRead(setTimePinPlus) ? 1 : -1;
if (setTimeGap != 60)
  Serial.println ("STOP");
  // every second steps increments the Gap to set the time by 1m
  if ( (millis() - setTimeSpeed) >= (refreshTime / refreshSetTime)) {
    currentTime += (setTimeGap * addMinus);
    setTimeSpeed = millis();

 Serial.print ("addMinus: ");
Serial.print (addMinus);
Serial.print (" -- currentTime: ");
Serial.print (currentTime);
Serial.print("  -- setTimeGap: ");
Serial.println (setTimeGap);
   
   // when decrementing time, when time reachs zero, for some reason (bug?) this variable, becomes zero too.
  if (setTimeGap == 0)
    setTimeGap = 60;
 
  }
}
