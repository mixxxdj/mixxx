var DJ202 = {};

DJ202.init = function () {

    DJ202.leftDeck = new DJ202.Deck([1,3], 0);
    DJ202.rightDeck = new DJ202.Deck([2,4], 1);
};

DJ202.shutdown = function () {
};

DJ202.Deck = function (deckNumbers, channel) {
    components.Deck.call(this, deckNumbers);



    // ============================= TRANSPORT ==================================
    this.play = new components.PlayButton([0x90 + channel, 0x00]);
    this.cue = new components.CueButton([0x90 + channel, 0x01]);
    this.sync = new components.SyncButton([0x90 + channel, 0x02]);

    this.reconnectComponents(function (component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};
DJ202.Deck.prototype = new components.Deck();
