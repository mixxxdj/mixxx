// eslint-disable-next-line no-var
var PioneerUnlock = {};

PioneerUnlock.init = function(id, _debugging) {
    console.info(`Pioneer Controller mapping started ${id}`);
};

PioneerUnlock.shutdown = function() {
    // nothing to do
};

// sysex handler
PioneerUnlock.incomingData = function(data, _length) {
    if (pcon.handleAuth(data, pcon.protocol.HID)) {
        // done, packet handled.
    }
};
