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
EksOtus.packetSize = {  output:32, input:64 };
EksOtus.ledState = { off:0, on:15 };
EksOtus.state= {    "flash":false   };  // temporary state variables
EksOtus.initialized=false;

// ----------   Functions   ----------

EksOtus.init = function (id) {    // called when the MIDI device is opened & set up
    EksOtus.id = id;   // Store the ID of this device for later use

    var packet=new Packet(32,0);

    // Check the firmware version
    packet.data[0]=10;
    packet.data[1]=2;

    // Report ID is always 0
    controller.send(packet.data, packet.length, 0);

    engine.beginTimer(60,"EksOtus.flash()");
}

EksOtus.shutdown = function() {
    print("EKS "+EksOtus.id+" shut down");
}

EksOtus.flash = function() {
    var packet=new Packet(32,0);

    // Send a wheel LED packet
    packet.data[0]=20;
    packet.data[1]=32;
    if (EksOtus.state["flash"]) {
        EksOtus.state["flash"] = false;
        
        packet.data[2]=15;
        packet.data[4]=15;
        packet.data[6]=15;
        packet.data[8]=15;
        packet.data[10]=15;
        packet.data[12]=15;
    }
    else {
        EksOtus.state["flash"] = true;
    }
    
    controller.send(packet.data, packet.length, 0);
}

EksOtus.incomingData = function (data, length) {
//     print("command="+data[0]+" length="+data[1]);
    // See which type of packet this is. The first byte is the command ID
    switch(data[0]) {
        case 1: // Set Timestamp response
        case 5: // Mouse on/off response
            if (data[2]==1) {
                // Success
            }
            else {
                // Failure
            }
            break;
        case 10: // Firmware version response
            var major=data[2];
            var minor=data[3];
            EksOtus.version=major+"."+minor;
            
            // Do firmware version-specific initialization here
            
            EksOtus.initialized=true;
            print("EKS "+EksOtus.id+" v"+EksOtus.version+" initialized");
            break;
        case 0: // Control state
            if (!EksOtus.initialized) break;
            // Populate variables
            
            // Call process
            break;
    }
}
