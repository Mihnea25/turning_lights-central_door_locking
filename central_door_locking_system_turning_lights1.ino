#include <LiquidCrystal.h>

#define loopDelay 20
#define tOnPeriod 300
#define tOffPeriod 700
#define holdPeriod 800
#define minHoldPeriod 50

#define unlocked 0 
#define locked 1 
#define doubleLocked 2

#define off 0
#define lc_left 1
#define lc_right 2
#define n_left 3
#define n_right 4
#define hazard 5

#define left 0
#define right 1
#define nr_of_leds 2

#define left_IL 0
#define right_IL 1 
#define nr_of_leds_IL 2

#define tOnCount (tOnPeriod/loopDelay)
#define tOffCount tOffPeriod/loopDelay
#define holdCount holdPeriod/loopDelay
#define minHoldCount minHoldPeriod/loopDelay

/*enum cdl_state_t{
	unlocked,
    locked,
    doubleLocked};

enum leduri{
	left,
    right,
   	nr_of_leds};*/

typedef struct switch_type_t{
	int switchState;
    int switchPin;
    int cntApasat;
};

typedef struct led_type_t{
	int ledPin;
    int ledCnt;
    int tOn;
    int tOff;
    int initState;
    int repeats;

};

struct switch_type_t_IL
{
int switch_state;
int switchPin;
int cnt_apasat;
int cnt_neapasat;
};

struct led_type_t_IL
{
int LedPin;
int led_cnt;
int t_on;
int t_off;
int init_state;
int lc_repeat;
};




LiquidCrystal lcd(12,11,5,6,7,8);

//Declarations for readSW
switch_type_t doorSwitch;
//Declarations for CDL_main
int CDLstate = unlocked;
int eventSwitchLockedToDoubleLocked = 0;
int eventSwitchLockedToUnlocked = 0;
int eventSwitchUnlockedToLocked = 0;
int eventSwitchUnlockedToLockedinhibibit = 0;
int eventSwitchDoubleLockedToUnlocked = 0;
int printCheck = 0;

//Declarations for ledBlink
led_type_t leftLed;
led_type_t rightLed;
led_type_t* Leds[nr_of_leds];

switch_type_t_IL leftSwitch;
switch_type_t_IL rightSwitch;
switch_type_t_IL hazardSwitch;


int ILstate = off;
int eventSwitchLeftToNormal = 0;
int eventSwitchLeftOutOfNormal  = 0;
int eventSwitchRightToNormal = 0;
int eventSwitchRightOutOfNormal = 0;
int eventSwitchLeftToLane = 0;
int eventSwitchRightToLane;
int eventSwitchLeftOutOfLane = 0;
int eventSwitchRightOutOfLane = 0;
int eventSwitchHazard = 0;
int leftSwitchState = 0;
int hazardSwitchState = 0;
int rightSwitchState = 0;

led_type_t_IL ilLeftLed;
led_type_t_IL ilRightLed;
led_type_t_IL* ilLeds[nr_of_leds_IL];
/*----------------------------
------------------------------
-----------------------------*/

const int leftLedPin = 10;
const int rightLedPin = 9;
const int switchPin = 2;
int leftRepeats = 5;
int rightRepeats = 12;
int leftCounter = 0;
int rightCounter = 0;
int leftZeroCheck = 0;
int rightZeroCheck = 0;
int preValue = 65535 - 20000;
int flag = 0;


void setup()
{
  lcd.begin(16,2);
  InitReadSW();
  initCDL();
  InitBlinkLED();
  lcd.print("Unlocked");
  Serial.begin(9600);
  
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = preValue;
  TCCR1B |=(1<<CS11);
  TIMSK1 |=(1<<TOIE1);
  EICRA |= (1 << ISC00);
  EIMSK |= (1 << INT0);
  interrupts();
}

void loop()
{  
  if(flag == 1){
   flag = 0;
   ReadSwitch();
   LedControl();
   CDL_main();
   //delay(loopDelay);
  }
  
}

void readSwitch(){

 static int leftcnt_apasat = 0;
 static int rightcnt_apasat = 0;
 static int hazardcnt_apasat = 0;
 static int leftcnt_neapasat = 0;
 static int rightcnt_neapasat = 0;


 leftSwitchState = digitalRead(leftSwitch.switchPin);
 hazardSwitchState = digitalRead(hazardSwitch.switchPin);
 rightSwitchState = digitalRead(rightSwitch.switchPin);

  
  //LEFT SWITCH
  if (leftSwitch.switch_state == HIGH )
  {
    //NORMAL MODE LEFT AND LANE CHANGE MODE LEFT
    if (leftSwitch.cnt_apasat< 50)
    {  
      leftSwitch.cnt_apasat++;
    }
    if (leftSwitch.cnt_apasat == 50)
    {    
      eventSwitchLeftToNormal = 1;
    }
   leftSwitch.cnt_neapasat = 0;
  }

  else
  {
    if ((leftSwitch.cnt_apasat >= 5) && (leftSwitch.cnt_apasat < 50))
    {
      eventSwitchLeftToLane = 1;
    }
    if (leftSwitch.cnt_neapasat < 10)
    {  
      leftcnt_neapasat++;
    }

    if (leftSwitch.cnt_neapasat == 10)
    {    
      eventSwitchLeftOutOfNormal = 1;
    }

    leftSwitch.cnt_apasat = 0;
  }

  //RIGHT SWITCH
  if (rightSwitch.switch_state == HIGH )
  {
     //NORMAL MODE RIGHT AND LANE CHANGE MODE RIGH
    if (rightSwitch.cnt_apasat < 50)
    {  
      rightSwitch.cnt_apasat++;
    }

    if (rightSwitch.cnt_apasat == 50)
    {
     eventSwitchRightToNormal  = 1;
    }
    rightSwitch.cnt_neapasat = 0;

  }
  else
  {
    if ((rightSwitch.cnt_apasat >= 5) && (rightSwitch.cnt_apasat < 50))
    {
      eventSwitchRightToLane = 1;
    }
    //OFF TO NORMAL MODE AND CHANGE LANE
    if (rightSwitch.cnt_neapasat < 10 )
    {  
     rightSwitch.cnt_apasat++;
    }
    if (rightSwitch.cnt_neapasat == 10)
    {
     eventSwitchRightOutOfNormal  = 1;
    }
    rightSwitch.cnt_apasat = 0;
  }

  //HAZARD SWITCH
  if (hazardSwitch.switch_state == HIGH )
  {
    if (hazardSwitch.cnt_apasat < 20)
    {  
      hazardSwitch.cnt_apasat++;
    }
  }
  else
  {
    if (hazardSwitch.cnt_apasat == 20)
    {
      eventSwitchHazard  = 1;
    }

    hazardSwitch.cnt_apasat = 0;
  }
}




void InitReadSW(void){
	doorSwitch.switchState = 0;
    doorSwitch.switchPin = 4;
  	doorSwitch.cntApasat = 0;

    pinMode(doorSwitch.switchPin,INPUT);
}

//!!!!!!!!!!!!!!!!
void InitReadSW_IL(void)
{
  leftSwitch.switch_state = 0;
  leftSwitch.switchPin = 1;  
  leftSwitch.cnt_apasat = 0;
  leftSwitch.cnt_neapasat = 0;
  
  rightSwitch.switch_state = 0;
  rightSwitch.switchPin = 0;  
  rightSwitch.cnt_apasat = 0;
  rightSwitch.cnt_neapasat = 0;
  
  hazardSwitch.switch_state = 0;
  hazardSwitch.switchPin = 13;  
  hazardSwitch.cnt_apasat = 0;
  hazardSwitch.cnt_neapasat = 0;
  
  pinMode(leftSwitch.switchPin, INPUT);
  pinMode(rightSwitch.switchPin, INPUT);
  pinMode(hazardSwitch.switchPin, INPUT);
}


void InitBlinkLED(void){
  
    Leds[left] = &leftLed;
	leftLed.ledPin = 10;
    leftLed.ledCnt = 0;
    leftLed.tOn = 0;
    leftLed.tOff = 0;
    leftLed.initState = LOW;
    leftLed.repeats = 0;
  
    Leds[right] = &rightLed;
    rightLed.ledPin = 9;
    rightLed.ledCnt = 0;
    rightLed.tOn = 0;
    rightLed.tOff = 0;
    rightLed.initState = LOW;
    rightLed.repeats = 0;
  
   pinMode(leftLed.ledPin,OUTPUT);
   pinMode(rightLed.ledPin,OUTPUT);
   digitalWrite(leftLed.ledPin,LOW);
   digitalWrite(rightLed.ledPin,LOW);
    
}

//!!!!!!!
void InitBlinkLED_IL(void)
{
  ilLeds[left] = &ilLeftLed;
  ilLeftLed.LedPin = 4;
  ilLeftLed.led_cnt = 0;
  ilLeftLed.t_on = 0;
  ilLeftLed.t_off = 0;
  ilLeftLed.init_state = LOW;
  ilLeftLed.lc_repeat = 0;
  
  ilLeds[right] = &ilRightLed;
  ilRightLed.LedPin = 3;
  ilRightLed.led_cnt = 0;
  ilRightLed.t_on = 0;
  ilRightLed.t_off = 0;
  ilRightLed.init_state = LOW;
  ilRightLed.lc_repeat = 0;
  
  pinMode(ilLeftLed.LedPin, OUTPUT);
  pinMode(ilRightLed.LedPin, OUTPUT);
  digitalWrite(ilLeftLed.LedPin, LOW);
  digitalWrite(ilRightLed.LedPin, LOW);
}


void ReadSwitch(void){
    //doorSwitch.switchState = digitalRead(doorSwitch.switchPin);
    if(doorSwitch.switchState == 1)
    {   
    	if(doorSwitch.cntApasat < holdCount)
        {
      	   doorSwitch.cntApasat++;	
        }	 	  
      	if(doorSwitch.cntApasat == holdCount)
        {
          if(CDLstate == locked)
          {
          eventSwitchLockedToUnlocked = 1;
          }
          if(CDLstate == doubleLocked)
          {
          eventSwitchDoubleLockedToUnlocked = 1;
      	  }
        }
      } 
  else{
    
    	if(doorSwitch.cntApasat>minHoldCount && doorSwitch.cntApasat<holdCount)
    	{
      	if(CDLstate == locked ){	
        	eventSwitchLockedToDoubleLocked = 1; }
    	}
    	
    	if(doorSwitch.cntApasat==holdCount)
    	{
     	if(eventSwitchDoubleLockedToUnlocked == 0 && eventSwitchLockedToUnlocked == 0 ){
          if (eventSwitchUnlockedToLockedinhibibit == 0)
          {
              if(CDLstate == unlocked  ){
              eventSwitchUnlockedToLocked = 1;
              }
          }
          eventSwitchUnlockedToLockedinhibibit = 0;
		}          
    	} 
    	doorSwitch.cntApasat = 0;  
  		}
     
} 

void BlinkControl(const int ledID,const int tOn,const int tOff,int *ledCnt,int *repeats,const int initState){
  
  if((*repeats) != 0){
  if((*ledCnt) < tOn){
  	digitalWrite(ledID,(initState));
  }
  else if((*ledCnt) < tOn + tOff){ 
  	digitalWrite(ledID,!(initState));
  }
  else if((*ledCnt) == tOn + tOff){
  	(*ledCnt) = 0;
    if((*repeats) >0){
    	(*repeats)--;
    }
  }
  if ((*ledCnt) <= (tOn+tOff))
  {
  	(*ledCnt)++;
  }  
 }   
}          

void LedControl(){
	BlinkControl(leftLed.ledPin,leftLed.tOn,leftLed.tOff,&leftLed.ledCnt,&leftLed.repeats,leftLed.initState);
	BlinkControl(rightLed.ledPin,rightLed.tOn,rightLed.tOff,&rightLed.ledCnt,&rightLed.repeats,rightLed.initState);
    BlinkControl(ilLeftLed.LedPin,rightLed.tOn,rightLed.tOff,&rightLed.ledCnt,&rightLed.repeats,rightLed.initState);
    BlinkControl(ilRightLed.LedPin,rightLed.tOn,rightLed.tOff,&rightLed.ledCnt,&rightLed.repeats,rightLed.initState);
    
}
void Blinkcmd(const int led,const int tOn,const int tOff,const int initState,const int nrCycles){
	Leds[led]->tOn = tOn;
  	Leds[led]->tOff = tOff;
  	Leds[led]->initState= initState;
  	Leds[led]->ledCnt = 0;
    if(nrCycles == 0){
    	Leds[led] -> repeats = -1;
    }
  else
    {
  		Leds[led] -> repeats = nrCycles;
  
    }

}


void initCDL(void){
  CDLstate = unlocked;
  eventSwitchLockedToDoubleLocked = 0;
  eventSwitchLockedToUnlocked = 0;
  eventSwitchUnlockedToLocked = 0;
  eventSwitchDoubleLockedToUnlocked = 0;
}

void initIL(void){
   ILstate = off;
   eventSwitchLeftToNormal = 0;
   eventSwitchLeftOutOfNormal  = 0;
   eventSwitchRightToNormal = 0;
   eventSwitchRightOutOfNormal = 0;
   eventSwitchLeftToLane = 0;
   eventSwitchRightToLane;
   eventSwitchLeftOutOfLane = 0;
   eventSwitchRightOutOfLane = 0;
   eventSwitchHazard = 0;	
/*------------------------------------------
  ------------------------------------------
  ------------------------------------------*/
}

void CDL_main(void){
	//Serial.print(CDLstate);
    switch(CDLstate){
    
    case unlocked:
    
      if(eventSwitchUnlockedToLocked==1){
      	CDLstate = locked;
        Blinkcmd(left,tOnCount,tOffCount,HIGH,1);
        Blinkcmd(right,tOnCount,tOffCount,HIGH,1);
        eventSwitchUnlockedToLocked = 0;
        lcd.clear();
        lcd.print("Locked");
        //printCheck = 0;
      }
    break;
      
   case locked:
   
      if(eventSwitchLockedToUnlocked == 1){
      	CDLstate = unlocked;
        eventSwitchUnlockedToLockedinhibibit = 1;
        Blinkcmd(left,tOnCount,tOffCount,HIGH,2);
        Blinkcmd(right,tOnCount,tOffCount,HIGH,2);
        eventSwitchLockedToUnlocked = 0;
        lcd.clear();
        lcd.print("Unlocked");
        //printCheck  = 0;
      }
      if(eventSwitchLockedToDoubleLocked == 1){
      	CDLstate = doubleLocked;
        Blinkcmd(left,tOnCount,tOffCount,HIGH,3);
        Blinkcmd(right,tOnCount,tOffCount,HIGH,3);
        eventSwitchLockedToDoubleLocked = 0;
        lcd.clear();
        lcd.print("Double Locked");
        //printCheck = 0;
      }
   break;
      
   case doubleLocked:
      if(eventSwitchDoubleLockedToUnlocked == 1){
      	CDLstate = unlocked;
        eventSwitchUnlockedToLockedinhibibit = 1;
        Blinkcmd(left,tOnCount,tOffCount,HIGH,2);
        Blinkcmd(right,tOnCount,tOffCount,HIGH,2);
        eventSwitchDoubleLockedToUnlocked = 0;
        lcd.clear();
        lcd.print("Unlocked");
        //printCheck = 0;
      }

	break;
    }
}

void IL_main(void){
  
  //Serial.println(IL_state);
  switch(ILstate){
    
    case off:
    
    if(eventSwitchLeftToLane == 1){
      
      ILstate = lc_left;
      Blinkcmd(ilLeftLed.LedPin,30,70,HIGH,3);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      eventSwitchLeftToLane = 0;
    }
    if(eventSwitchRightToLane == 1){
      
      ILstate = lc_right;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,30,70,HIGH,3);
      eventSwitchRightToLane = 0;
    }
    
     if( eventSwitchLeftToNormal == 1){
      
      ILstate = n_left;
      Blinkcmd(ilLeftLed.LedPin,30,70,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      eventSwitchLeftToNormal = 0;
    }
    if( eventSwitchRightToNormal == 1){
      
      ILstate = n_right;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,30,70,HIGH,0);
      eventSwitchRightToNormal = 0;
    }
    if(eventSwitchHazard == 1){
      
      ILstate = hazard;
      Blinkcmd(ilLeftLed.LedPin,50,50,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,50,50,HIGH,0);
      
      eventSwitchHazard = 0;
    }
    break;
    
    case lc_left:
      
    if( eventSwitchLeftOutOfLane == 1){
       ILstate = off;
       Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
       Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
       initIL();
    }
    if( eventSwitchLeftToNormal == 1){
      //!!!
      ILstate = n_left;
      Blinkcmd(ilLeftLed.LedPin,30,70,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      eventSwitchLeftToNormal = 0;
    }
    if( eventSwitchRightToNormal == 1){
      
      ILstate = n_right;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,30,70,HIGH,0);
      eventSwitchRightToNormal = 0;
    }
    if(eventSwitchHazard == 1){
      
      ILstate = hazard;
      Blinkcmd(ilLeftLed.LedPin,50,50,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,50,50,HIGH,0);
      eventSwitchHazard = 0;
    }
     
    break;
    
    case lc_right:
   
   
    if(eventSwitchRightOutOfLane == 1){
      ILstate = off;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      initIL();
    }
    if( eventSwitchLeftToNormal == 1){
      
      ILstate = n_left;
      Blinkcmd(ilLeftLed.LedPin,30,70,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      eventSwitchLeftToNormal = 0;
    }
    if( eventSwitchRightToNormal == 1){
      
      ILstate = n_right;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,30,70,HIGH,0);
      eventSwitchRightToNormal = 0;
    }
    if(eventSwitchHazard == 1){
      
      ILstate = hazard;
      Blinkcmd(ilLeftLed.LedPin,50,50,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,50,50,HIGH,0);
      eventSwitchHazard = 0;
    }
    break;
    case n_left:
       
    if( eventSwitchLeftOutOfNormal == 1){
      
      ILstate = off;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      initIL();
    }
    if(eventSwitchHazard == 1){
      
      ILstate = hazard;
      Blinkcmd(ilLeftLed.LedPin,50,50,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,50,50,HIGH,0);
      eventSwitchHazard = 0;
    }
    
    break;
    
    case n_right:
    if( eventSwitchRightOutOfNormal == 1){
      
      ILstate = off;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      initIL();
    }
    if(eventSwitchHazard == 1){
      
      ILstate = hazard;
      Blinkcmd(ilLeftLed.LedPin,50,50,HIGH,0);
      Blinkcmd(ilRightLed.LedPin,50,50,HIGH,0);
      eventSwitchHazard = 0;
    }
    break;
    
    case hazard:
    
   
    
    if(eventSwitchHazard == 1){
      
      ILstate = off;
      Blinkcmd(ilLeftLed.LedPin,0,0,LOW,0);
      Blinkcmd(ilRightLed.LedPin,0,0,LOW,0);
      initIL();
    }    
    break;    
  }
  
}

  
ISR(TIMER1_OVF_vect){
	TCNT1 = preValue;
    flag = 1;
}

ISR(INT0_vect){
   
  doorSwitch.switchState = !(doorSwitch.switchState);
    
}          
