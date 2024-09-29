// eslint-disable-next-line no-var
var PioneerUnlock = {};

PioneerUnlock.init = function(id, _debugging) {
    console.info(`Pioneer Controller mapping started ${id}`);
    pcon.sysexGreet();
};

PioneerUnlock.shutdown = function() {
    // nothing to do
};

// sysex handler
PioneerUnlock.incomingData = function(data, _length) {
    if (pcon.handleAuth(data, pcon.protocol.SYSEX)) {
        // done, packet handled.

    }
};
