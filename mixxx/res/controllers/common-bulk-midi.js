midi = new Object();

midi.sendShortMsg = function(status, a, b) {
    controller.send([status, a, b], 3);
}
