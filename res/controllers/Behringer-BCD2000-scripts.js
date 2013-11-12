function BehringerBCD2000 () {}
BehringerBCD2000.debug = false;
BehringerBCD2000.escratch = [false, false];

//sensitivity setting
BehringerBCD2000.UseAcceleration = true;
BehringerBCD2000.JogSensivity = 0.2;

BehringerBCD2000.init = function (id) { // called when the device is opened & set up

   BehringerBCD2000.reset();

   // Ask BCD to send the current values of all rotary knobs and sliders
   midi.sendShortMsg(0xB0,0x64,0x7F);

   // Set jog acceleration
   if (BehringerBCD2000.UseAcceleration)
      midi.sendShortMsg(0xB0, 0x63, 0x7F);
   else
      midi.sendShortMsg(0xB0, 0x63, 0x0);
};

BehringerBCD2000.shutdown = function () {

   BehringerBCD2000.reset();

   // Reenable jog acceleration
   if (!BehringerBCD2000.UseAcceleration)
      midi.sendShortMsg(0xB0, 0x63, 0x7F);
};

BehringerBCD2000.reset = function () {

   // Turn off all the lights
   for (i = 0; i <= 25; i++) {
      midi.sendShortMsg(0xB0, i, 0);
   }

};

BehringerBCD2000.getDeck = function (group) {
   if (group == "[Channel1]")
      return 0;
   else if (group == "[Channel2]")
      return 1;

   print("Invalid group : " + group);
   return -1; // error
}


//Scratch, cue search and pitch bend function
BehringerBCD2000.jogWheel = function (channel, control, value, status, group) {


   deck = BehringerBCD2000.getDeck(group);

   if (BehringerBCD2000.escratch[deck]) {

if (value >= 65)

{scratchValue = (value - 0x40);}

else

{scratchValue = (value - 0x41);}
      engine.scratchTick(deck + 1, scratchValue);

      if (BehringerBCD2000.debug)
         print(group + " scratch tick : " + scratchValue);

   } else {

      if (value >= 65)

{jogValue = (value - 0x40) * BehringerBCD2000.JogSensivity;}

else

{jogValue = (value - 0x41) * BehringerBCD2000.JogSensivity;}
      engine.setValue(group, "jog", jogValue);

      if (BehringerBCD2000.debug)
         print(group + " pitching jog adjust : " + jogValue);

   }
};

//Scratch button function
BehringerBCD2000.scratchButton = function (channel, control, value, status, group) {

   if (value != 0x7F)
      return;

   deck = BehringerBCD2000.getDeck(group);

   BehringerBCD2000.escratch[deck] = !BehringerBCD2000.escratch[deck];

   if (BehringerBCD2000.debug)
      print(group + " scratch enabled :" + BehringerBCD2000.escratch[deck]);

   if (BehringerBCD2000.escratch[deck]) {
      // Turn on the scratch light
      if (!deck)
         midi.sendShortMsg(0xB0, 0x13, 0x7F);
      else
         midi.sendShortMsg(0xB0, 0x0B, 0x7F);

      // Enable scratching
      engine.scratchEnable(deck + 1, 100, 33+1/3, 1.0/8, (1.0/8)/32);

   } else {
      // Turn off the scratch light
      if (!deck)
         midi.sendShortMsg(0xB0, 0x13, 0x00);
      else
         midi.sendShortMsg(0xB0, 0x0B, 0x00);

      // Disable scratching
      engine.scratchDisable(deck + 1);
   }
};

//Set loop function
BehringerBCD2000.loop = function (channel, control, value, status, group) {
   if (value)
      action = "loop_in";
   else
      action = "loop_out";

   if (BehringerBCD2000.debug)
      print(group + " " + action);

    engine.setValue(group, action, 1);
};
