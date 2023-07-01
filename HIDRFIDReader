//
// RFID READER
//

RFIDReader = new HIDController();

RFIDReader.init = function(id) {
    RFIDReader.id = id;
    HIDDebug("RFID Reader Initialized: " + RFIDReader.id);
}

RFIDReader.shutdown = function() {
    HIDDebug("RFID Reader Shutdown: " + RFIDReader.id);
}

RFIDReader.incomingData = function(data, length) {
    var controller = RFIDReader.controller;
    if (controller == undefined) {
        HIDDebug("Error in script initialization: controller not found");
        return;
    }
    controller.parsePacket(data, length);
}
