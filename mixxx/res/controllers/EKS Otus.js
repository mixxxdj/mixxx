/****************************************************************/
/*      EKS Otus HID controller script v0.01                    */
/*          Copyright (C) 2012, Sean M. Pappalardo              */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.11.x                                */
/****************************************************************/

EksOtus = new Controller();

// ----------   Customization variables ----------
//      See   for details

// ----------   Other global variables    ----------
EksOtus.commandID = {       wheelLeds:20, buttonLeds:22, sliderLeds:23  };
EksOtus.packetLength = {    wheelLeds:32, buttonLeds:24, sliderLeds:21  };
EksOtus.ledState = { off:0, on:15 };

// ----------   Functions   ----------

EksOtus.init = function (id) {    // called when the MIDI device is opened & set up
    EksOtus.id = id;   // Store the ID of this device for later use

    // Send an LED packet
    var packet=new Packet(EksOtus.packetLength["wheelLeds"],0);

    packet.data[0]=EksOtus.commandID["wheelLeds"];
    packet.data[1]=EksOtus.packetLength["wheelLeds"];
    
    packet.data[3]=EksOtus.ledState["on"];
    packet.data[4]=EksOtus.ledState["on"];
    packet.data[7]=EksOtus.ledState["on"];
    packet.data[9]=EksOtus.ledState["on"];

    // Report ID is always 0
    controller.send(packet.data, packet.length, 0);

    print("EKS Otus "+id+" initialized");
}
