//
// Demp script for Sony SixxAxis controller. 
// Please note the example packet parser captures all data to dump() function
// and mixxx doesn't see any details itself.
//

SonySixxAxis = new SonySixxAxisController();

SonySixxAxis.init = function(id) {
    SonySixxAxis.id = id;
    SonySixxAxis.registerInputPackets();
    script.HIDDebug("Sony SixxAxis controller initialized: " + SonySixxAxis.id);
}

SonySixxAxis.shutdown = function() {
    script.HIDDebug("Sony SixxAxis controller shutdown: " + SonySixxAxis.id);
}

SonySixxAxis.incomingData = function(data,length) {
    SonySixxAxis.controller.parsePacket(data,length);
}

