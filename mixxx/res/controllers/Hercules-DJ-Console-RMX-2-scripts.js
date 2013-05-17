/*╔══:::Made Lovingly By Circuitfry:::═════════════════════════════════╗
  ║     Hercules DJConsole RMX 2 Mapping Scripts v. 0.1.3              ║
  ╚════════════════════════════════════════════════════════════════════╝
  * Version 0.1.0: Basic Midi Wizard Mapping
  * Version 0.1.1: Partially-Functional platters (version 1).
  * Version 0.1.2: Functional platters (version 1)
  * Version 0.1.3: Functional EQ Kill/Pitch Bending buttons
                   Functional Looping/Sample/Effect pads
                   Bugfix: Source 2 Gain knob doesn't load tracks.
                   Overhaul: MIDI Scripting file.
    Worklog: Need to implement Microphone/Source1/Source2 input.
  * Note 1: [DEP] Means the command is meant for Mixxx v1.10.x + below.
  * Note 2: [FUT] Means the command is meant for Mixxx v1.11.x + above.
*/
function DJCRMX2(){}
DJCRMX2.scratching = [];

/*  [ Function init ] - Version 0.1.3
  * Initiates some global variables and assigns an ID. Required.
*/
DJCRMX2.init = function(id){	
 DJCRMX2.id = id;
 DJCRMX2.scratching[1]=false;
 DJCRMX2.scratching[2]=false;
 engine.setValue("[Microphone]","enabled",0);
 engine.setValue("[Microphone]","talkover",0);
}

/*  [ Function wheelPress ] - Version 0.1.2
  * Detects whether a jog wheel is pressed or not and sets a specific
  * variable on and off accordingly.
*/
DJCRMX2.wheelPress = function (channel, control, value, status, group){
 if (status == 0x90) // If status #144 is active (2 possibilities)
 {
  if (value == 0x7F) // And the jog wheel is pressed down:
  {  /*  engine.scratchEnable(int,int,float,float,float,bool);
       * [ int deck ]              Which track/platter is playing?
       * [ int intervalsPerRev ]   # of MIDI signals sent in 1 spin.
       * [ float rpm ]             Imaginary vinyl rotation speed.
       * [ float alpha ]           Just a fine-tuning variable.
       * [ float beta ]            Just a fine-tuning variable.
       * [ bool ramp ]             As far as I know, nothing...
     */
      var alpha = 1.0/8;
      var beta = alpha/32;
      if(group=="[Channel1]")
      {
       engine.scratchEnable(1, 250, 50, alpha, beta);
       DJCRMX2.scratching[1] = true; //[DEP]
	  }
	  if(group=="[Channel2]")
	  {
	   engine.scratchEnable(2, 250, 50, alpha, beta);
	   DJCRMX2.scratching[2] = true; //[DEP]
	  }
      
  }
  if (value == 0x00 ) // If the jog wheel is released:
  {
   if(group=="[Channel1]")
   {
    DJCRMX2.scratching[1] = false; // <- v1.10.x and below
    engine.scratchDisable(1);
   }
   if(group=="[Channel2]")
   {
    DJCRMX2.scratching[2] = false; // <- v1.10.x and below
   engine.scratchDisable(2);
   }
  }
 }
 else //Default setting where button is not held down.
 {
  DJCRMX2.scratching[1] = false;  // Only for v1.10.x and below
  DJCRMX2.scratching[2] = false;  // Only for v1.10.x and below
  engine.scratchDisable(1);
  engine.scratchDisable(2);
 }
 return;
}
 
/*  [ Function wheelTurn ] - Version 0.1.2
  * Pays attention to the current deck, checks scratching, affects the
  * song accordingly.
*/
DJCRMX2.wheelTurn = function (channel, control, value, status, group){
 var newValue=0;
 // Spinning backwards = 127 or less (less meaning faster)
 // Spinning forwards  = 1 or more (more meaning faster)
 if (value-64 > 0) newValue = (value-128);
 else newValue=value;
 //if (!engine.isScratching(DJCRMX2.currentDeck)) // [FUT]
 if(group=="[Channel1]")
 {
  if(DJCRMX2.scratching[1]==true) {engine.scratchTick(1,newValue);return;}
 }
 else if(group=="[Channel2]")
 {
  if(DJCRMX2.scratching[2]==true) {engine.scratchTick(2,newValue);return;}
 }
 engine.setValue(group, "jog", newValue);
 return;
}

DJCRMX2.micSwitch = function (channel, control, value, status) //???
{
 if(status == 0x90 && control == 0x48 && value == 0x7F)
 {
  engine.setValue("[Microphone]","enabled",1);
  engine.setValue("[Microphone]","talkover",1);
 }
 if(status == 0x90 && control == 0x48 && value == 0x00)
 {
  engine.setValue("[Microphone]","enabled",0);
  engine.setValue("[Microphone]","talkover",0);
 }
}

/*  [ Function shutdown ] - Version 0.1.3
  * Sets variables down for shutoff.
*/
DJCRMX2.shutdown = function(id){
 DJCRMX2.scratching[1]=false;
 DJCRMX2.scratching[2]=false;
 engine.setValue("[Microphone]","enabled",0);
 engine.setValue("[Microphone]","talkover",0);
}
