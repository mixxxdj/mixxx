/*
Novation Dicer mapping scripts for loop rolls
by Pandemonium (djpandemonium@gmail.com)
April 2012

Concept adapted from shyft's stutterHack

Known issues:
Since loop rolls necessarily manipulate both the beatloop and loop states, triggering 
a loop roll will erase any ongoing loops or saved loop points for the channel.

As longer effects are performed, it starts to drift slightly off of the correct
place to resume playing.  I try to compensate for this a bit, but couldn't make it
perfect.  Also, my compensation is based on trial and error, and may be incorrect
on other people's machines.  YMMV.  It would be better if this capability were 
implemented in Mixxx itself so as to keep the correct time.

When an effect is released and pitch lock is on, there is a tiny silent pause
before it resumes playing.  Once again, this doesn't appear to be fixable here.
*/


function NovationDicer() {}

NovationDicer.effectsCH1 = 0; //Record number of effect buttons pressed. Protects against multi-button mashing
NovationDicer.effectsCH2 = 0;
NovationDicer.gainCH1 = 0;
NovationDicer.gainCH2 = 0;
NovationDicer.green = 0x7F;     //"On" state
NovationDicer.orange = 0x4F;    //"On" state
NovationDicer.softGreen = 0x71; //"Off" state
NovationDicer.softOrange = 0x41;//"Off" state

NovationDicer.init = function (id)
{
    midi.sendShortMsg(0xBA,0x00,0x00);  //Sets Dicers to known state
    
    midi.sendShortMsg(0x9B,0x3c,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x3c,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x3d,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x3d,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x3e,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x3e,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x3f,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x3f,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x40,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x40,NovationDicer.softGreen);
    
    midi.sendShortMsg(0x9B,0x41,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x41,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x42,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x42,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x43,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x43,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x44,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x44,NovationDicer.softGreen);
    midi.sendShortMsg(0x9B,0x45,NovationDicer.softGreen);
    midi.sendShortMsg(0x9E,0x45,NovationDicer.softGreen);
    
    midi.sendShortMsg(0x9c,0x3c,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x3d,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x3e,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x3f,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x40,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x3c,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x3d,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x3e,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x3f,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x40,NovationDicer.softOrange);
    
    midi.sendShortMsg(0x9c,0x41,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x42,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x43,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x44,NovationDicer.softOrange);
    midi.sendShortMsg(0x9c,0x45,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x41,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x42,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x43,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x44,NovationDicer.softOrange);
    midi.sendShortMsg(0x9f,0x45,NovationDicer.softOrange);
    
    engine.setValue("[Flanger]", "lfoDepth", 1);    //Crank up the depth so the flanger does something
}


NovationDicer.shutdown = function ()
{
    midi.sendShortMsg(0xBA,0x00,0x00);  //Resets Dicers
}


NovationDicer.effectThenContinue = function (channel, control, value, status, group)
{    
    if (value)  //Button pressed
    {
        if ((NovationDicer.effectsCH1 == 0) && (group == "[Channel1]")) //Record the starting point of the CH1 first effect
        {
            NovationDicer.startTimeCH1 = new Date();
            NovationDicer.startPlayPosCH1 = engine.getValue(group,"playposition");
        } 
        
        if ((NovationDicer.effectsCH2 == 0) && (group == "[Channel2]"))  //Record the starting point of the CH2 first effect
        {
            NovationDicer.startTimeCH2 = new Date();
            NovationDicer.startPlayPosCH2 = engine.getValue(group,"playposition");
        }
        
        NovationDicer.startEffect(channel, control, value, status, group);
        
        if (group == "[Channel1]")
            NovationDicer.effectsCH1++;
        else
            NovationDicer.effectsCH2++;
            
    }else      //Button released
    {
      
        if (((NovationDicer.effectsCH1 == 1) && (group == "[Channel1]")) || ((NovationDicer.effectsCH2 == 1) && (group == "[Channel2]")))  //End the effect session and resume playing if it's the last effect running
        {
            if (group == "[Channel1]")
            {
                var tDelta = new Date() - NovationDicer.startTimeCH1; // Duration of loop
                tDelta = tDelta + 28 + (tDelta * 0.00065);   //Compensate for wandering/delay
                tDelta = tDelta + (tDelta * (engine.getValue(group, "rate") * 0.1) * engine.getValue(group, "rate_dir"));   //Compensate for pitch
                var track1Ms = engine.getValue(group,"duration") * 1000;   // Convert seconds to MS
                var newPlayPos = (NovationDicer.startPlayPosCH1 + (tDelta/track1Ms));
            } else
            {
                var tDelta = new Date() - NovationDicer.startTimeCH2;
                tDelta = tDelta + 28 + (tDelta * 0.00065);   //Compensate for wandering/delay
                tDelta = tDelta + (tDelta * (engine.getValue(group, "rate") * 0.1) * engine.getValue(group, "rate_dir"));   //Compensate for pitch
                var track2Ms = engine.getValue(group,"duration") * 1000;   
                var newPlayPos = (NovationDicer.startPlayPosCH2 + (tDelta/track2Ms));
            }
            
            NovationDicer.endEffect(channel, control, value, status, group);
            
            engine.setValue(group,"playposition",newPlayPos); //Resume playing
        }
        
        if (group == "[Channel1]")
            NovationDicer.effectsCH1--;
        else
            NovationDicer.effectsCH2--;
        
        NovationDicer.turnOffLEDs(channel, control, value, status, group);
    }
}


NovationDicer.startEffect = function(channel, control, value, status, group)    //Start the effect and light the LED
{
    //----------Loop Rolls----------    
    if (channel == 0x0B && control == 0x3C)  //ch1
    {
        engine.setValue(group, "beatloop_0.0625", 1);
        midi.sendShortMsg(0x9B,0x3c,NovationDicer.green);
    }
    if (channel == 0x0E && control == 0x3C)   //ch2
    {
        engine.setValue(group, "beatloop_0.0625", 1);
        midi.sendShortMsg(0x9E,0x3c,NovationDicer.green);
    }
    if (channel == 0x0B && control == 0x3D)  //ch1
    {
        engine.setValue(group, "beatloop_0.125", 1);
        midi.sendShortMsg(0x9B,0x3d,NovationDicer.green);
    }
    if (channel == 0x0E && control == 0x3D)  //ch2
    {
        engine.setValue(group, "beatloop_0.125", 1);
        midi.sendShortMsg(0x9E,0x3d,NovationDicer.green);
    }
    if (channel == 0x0B && control == 0x3E)   //ch1
    {
        engine.setValue(group, "beatloop_0.25", 1);
        midi.sendShortMsg(0x9B,0x3e,NovationDicer.green);
    }
    if (channel == 0x0E && control == 0x3E)   //ch2
    {
        engine.setValue(group, "beatloop_0.25", 1);
        midi.sendShortMsg(0x9E,0x3e,NovationDicer.green);
    }
    if (channel == 0x0B && control == 0x3F)   //ch1
    {
        engine.setValue(group, "beatloop_0.5", 1);
        midi.sendShortMsg(0x9B,0x3f,NovationDicer.green);
    }
    if (channel == 0x0E && control == 0x3F)   //ch2
    {
        engine.setValue(group, "beatloop_0.5", 1);
        midi.sendShortMsg(0x9E,0x3f,NovationDicer.green);
    }
    if (channel == 0x0B && control == 0x40)  //ch1
    {
        engine.setValue(group, "beatloop_1", 1);
        midi.sendShortMsg(0x9B,0x40,NovationDicer.green);
    }
    if (channel == 0x0E && control == 0x40)   //ch2
    {
        engine.setValue(group, "beatloop_1", 1);
        midi.sendShortMsg(0x9E,0x40,NovationDicer.green);
    }
    
    //----------Rewind/stutter/cue----------
    if (channel == 0x0c && control == 0x3c)
    {
        engine.setValue(group, "fwd", 1);
        midi.sendShortMsg(0x9c,0x3c,NovationDicer.orange);
    }
    if (channel == 0x0f && control == 0x3c)
    {
        engine.setValue(group, "fwd", 1);
        midi.sendShortMsg(0x9f,0x3c,NovationDicer.orange);
    }
    if (channel == 0x0c && control == 0x40)
    {
        engine.setValue(group, "back", 1);
        midi.sendShortMsg(0x9c,0x40,NovationDicer.orange);
    }
    if (channel == 0x0f && control == 0x40)
    {
        engine.setValue(group, "back", 1);
        midi.sendShortMsg(0x9f,0x40,NovationDicer.orange);
    }
    
}


NovationDicer.endEffect = function(channel, control, value, status, group)
{
    //Make one test for each group of *all* effects that can possibly be triggered together
    //and disable them all at once
    
    //----------Loop Rolls----------    
    if (engine.getValue(group, "loop_enabled"))
    {
        engine.setValue(group,"loop_start_position", -1); //Exit any loops
        engine.setValue(group,"loop_end_position", -1);
    }
    
    //----------Rewind/stutter/cue----------
    if ((engine.getValue(group, "fwd")) || (engine.getValue(group, "back")))
    {
        engine.setValue(group, "fwd", 0);
        engine.setValue(group, "back", 0);
    }
    
}


NovationDicer.turnOffLEDs = function(channel, control, value, status, group)
{
    //----------Loop Rolls----------    
    if (channel == 0x0B && control == 0x3C)  //ch1
    {
        midi.sendShortMsg(0x9B,0x3c,NovationDicer.softGreen);
    }
    if (channel == 0x0E && control == 0x3C)   //ch2
    {
        midi.sendShortMsg(0x9E,0x3c,NovationDicer.softGreen);
    }
    if (channel == 0x0B && control == 0x3D)  //ch1
    {
        midi.sendShortMsg(0x9B,0x3d,NovationDicer.softGreen);
    }
    if (channel == 0x0E && control == 0x3D)  //ch2
    {
        midi.sendShortMsg(0x9E,0x3d,NovationDicer.softGreen);
    }
    if (channel == 0x0B && control == 0x3E)   //ch1
    {
        midi.sendShortMsg(0x9B,0x3e,NovationDicer.softGreen);
    }
    if (channel == 0x0E && control == 0x3E)   //ch2
    {
        midi.sendShortMsg(0x9E,0x3e,NovationDicer.softGreen);
    }
    if (channel == 0x0B && control == 0x3F)   //ch1
    {
        midi.sendShortMsg(0x9B,0x3f,NovationDicer.softGreen);
    }
    if (channel == 0x0E && control == 0x3F)   //ch2
    {
        midi.sendShortMsg(0x9E,0x3f,NovationDicer.softGreen);
    }
    if (channel == 0x0B && control == 0x40)  //ch1
    {
        midi.sendShortMsg(0x9B,0x40,NovationDicer.softGreen);
    }
    if (channel == 0x0E && control == 0x40)   //ch2
    {
        midi.sendShortMsg(0x9E,0x40,NovationDicer.softGreen);
    }
    
    //----------Rewind/stutter/cue----------
    if (channel == 0x0c && control == 0x3c) //FF ch1
    {
        midi.sendShortMsg(0x9c,0x3c,NovationDicer.softOrange);
    }
    if (channel == 0x0f && control == 0x3c) //FF ch2
    {
        midi.sendShortMsg(0x9f,0x3c,NovationDicer.softOrange);
    }
    if (channel == 0x0c && control == 0x40)
    {
        midi.sendShortMsg(0x9c,0x40,NovationDicer.softOrange);
    }
    if (channel == 0x0f && control == 0x40)
    {
        midi.sendShortMsg(0x9f,0x40,NovationDicer.softOrange);
    }
}


NovationDicer.transformer = function(channel, control, value, status, group)
{
    if(value)
    {
        if(group == "[Channel1]")
        {
            NovationDicer.gainCH1 = engine.getValue(group, "pregain");
            NovationDicer.timer1 = engine.beginTimer(((60000/engine.getValue(group,"bpm"))/8),"NovationDicer.stutter(\"[Channel1]\")");
            midi.sendShortMsg(0x9c,0x3f,NovationDicer.orange);
        } else
        {
            NovationDicer.gainCH2 = engine.getValue(group, "pregain");
            NovationDicer.timer2 = engine.beginTimer(((60000/engine.getValue(group,"bpm"))/8),"NovationDicer.stutter(\"[Channel2]\")");
            midi.sendShortMsg(0x9f,0x3f,NovationDicer.orange);
        }
    } else
    {
        if(group == "[Channel1]")
        {
            engine.stopTimer(NovationDicer.timer1);
            engine.setValue(group, "pregain", NovationDicer.gainCH1);
            midi.sendShortMsg(0x9c,0x3f,NovationDicer.softOrange);
        }
        if(group == "[Channel2]")
        {
            engine.stopTimer(NovationDicer.timer2);
            engine.setValue(group, "pregain", NovationDicer.gainCH2);
            midi.sendShortMsg(0x9f,0x3f,NovationDicer.softOrange);
        }
    }
}

NovationDicer.stutter = function(group)
{
    if(engine.getValue(group,"pregain") != 0)
        engine.setValue(group, "pregain", 0);
    else
    {
        if(group == "[Channel1]")
            engine.setValue(group, "pregain", NovationDicer.gainCH1);
        else
            engine.setValue(group, "pregain", NovationDicer.gainCH2);
     }       
}

NovationDicer.flangeEffect = function(channel, control, value, status, group)
{  
    if (value)  //Button pressed
    {
        engine.setValue(group, "flanger", !engine.getValue(group, "flanger"));  //Toggle the flanger
    }
}

NovationDicer.cueButton = function(channel, control, value, status, group)
{
    if(value)
    {
        if (group == "[Channel1]")
            midi.sendShortMsg(0x9c,0x3d,NovationDicer.orange);
        else
            midi.sendShortMsg(0x9f,0x3d,NovationDicer.orange);
            
        engine.setValue(group, "cue_default", 1);
    }else
    {
        if (group == "[Channel1]")
            midi.sendShortMsg(0x9c,0x3d,NovationDicer.softOrange);
        else
            midi.sendShortMsg(0x9f,0x3d,NovationDicer.softOrange);
        engine.setValue(group, "cue_default", 0);
    }
}