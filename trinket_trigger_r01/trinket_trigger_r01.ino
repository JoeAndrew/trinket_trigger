
/*
This sketch implements a "smart slave" for flash photography using studio strobes.
It lets manual strobes work with advanced TTL flashes by learning and then ignoring any number of preflash "pops."
Has been tested with Olympus RC system; should also work with other systems such as Nikon CLS and
Canon E-TTL II, but has not been tested against them.

The trigger starts up in "learn mode," in which we count how many pops the master flash emits when firing
and store the count in popCounter. Learn mode waits indefinitely for the first pop in the sequence, then
exits into "run mode" after an interval set by the variable giveUpTime.

In "run mode", we count how many pops the flash HAS emitted and store it in currentPop.
When currentPop equals popCounter, we know we've reached the last pop, and should fire the slave flash.

If the pop count is thrown off (e.g. by misfire of another flash) the trigger would fail to sync correctly
if it kept waiting for the previous cycle of pops to complete. To avoid this, we incorporate a "give-up" timer
that stops waiting for pops and resets the system if a pop sequence doesn't complete within the time pre-set
by the giveUpTime variable.

*/


//PINS
int triggerPin = 0;    // connects to transistor that switches inner jack
int readyPin = 1;      //connects to the built-in LED on an Arduino Micro
int interruptPin = 2;
int loopPad = 100;      // pause this amount during loop to ignore "ringing"
int postreset = 200;    // amount we wait after firing the flash so it doesn't re-trigger
int giveUpTime = 500;  // stop looking for pulses after this time so we don't hang


//VOLATILE INTS
//changed by interrupt to count pops
volatile int popCounter =0;
volatile int currentPop = 0;

//used for mode selection
int runMode = 1;  //0 for run; 1 for learn; 2 for fire

long timerStart;  //used for non-blocking timer in learn mode
long timerDuration = 10000;  //amount of time we stay in "learn mode" at startup; adjust if you want longer or shorter
long giveUpStartTime;        //used for non-blocking timer in run mode


void setup()
  {
    
    
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);
  
    pinMode(readyPin, OUTPUT);
    digitalWrite(readyPin, LOW);
    
    pinMode(interruptPin, INPUT);
    attachInterrupt(0, trigger, FALLING);  //on the Trinket, interrupt 0 is on pin 2
  
    digitalWrite(readyPin, HIGH);
    runMode = 1;  //start in learn mode
  }

void loop()
{
  
    //learn mode -- stay in this mode until we have received at least one pop AND give-up time has expired
    if (runMode == 1 && popCounter == 0)  {
            giveUpStartTime = millis();   //keep initializing the give-up timer 
     }
     
     if (runMode == 1 && popCounter > 0 && millis() > giveUpStartTime + giveUpTime)  {  //learn mode timed out
    
            runMode = 0;                  //switch to run mode
            
            // turn LED off:
            digitalWrite(readyPin, LOW);
            delay(1000);  //wait 1 second before we blink the LED
            
            //blink the LED to read out the number of pops captured
            for (int x = 0; x < popCounter; x++)
            {
            digitalWrite(readyPin, HIGH);
            delay(250);
            digitalWrite(readyPin,LOW);
            delay(250);
            }
     }
           
     
     // reset mode -- get ready for the next sequence
     if (runMode == 2)  {    //the interrupt has just triggered the flash
       
            digitalWrite (triggerPin, LOW);   //reset the flash trigger pin
            currentPop = 0;                   //reset the counter for the next sequence
            runMode = 0;                      //reset to normal running
            
            //if you want a confirmation LED, uncomment the following sequence;
            //otherwise, it will reset a little faster without it
    
            digitalWrite (readyPin, HIGH);    //turn on the LED so we know we've triggered
            delay(postreset);
            digitalWrite (readyPin, LOW);
        }
        
      //run mode -- waiting for flash to trigger
      if (runMode == 0) {
            
            delay(loopPad);  //delay a short time to ignore "ringing" in the circuit
            
            //the trigger will hang if one of the flashes misfires, because it's still waiting
            //for the right number of pulses. This section makes it give up and reset the counter
            // if subsequent pulses don't arrive within a set interval [giveUpTime] after the first
            
            if (currentPop == 0)  {  //still waiting for a flash
                  giveUpStartTime = millis();  //store the current millis for timing
            }
            else if(currentPop > 0 && millis() > giveUpStartTime + giveUpTime) {
                  runMode = 2;                //we've timed out; reset everything
            }
        }

    
}  //end of the loop
  

void trigger()
{
   if (runMode == 1) {              //counting pops in learn mode
        popCounter ++;
      }
   else if (runMode == 0)  {        //counting pops in run mode
      currentPop ++;
      
   }
  
  if (currentPop == popCounter)  {  //we've reached the last pop and should fire the slave
        digitalWrite(triggerPin, HIGH);  //fire the flash!
        runMode = 2;                //we've fired, so reset for next
      } 

}



