/* eslint no-redeclare: "off", no-unused-vars: "off" */

class Component {
    constructor(group, inKey) {
        this.group = group;
        this.inKey = inKey;
    }
}

class Deck {
    constructor(decks) {
        if (typeof decks === "number") {
            this.group = Deck.groupForNumber(decks);
        } else if (Array.isArray(decks)) {
            this.decks = decks;
            this.currentDeckNumber = decks[0];
            this.group = Deck.groupForNumber(decks[0]);
        }
    }
    toggleDeck() {
        if (this.decks === undefined) {
            throw Error("toggleDeck can only be used with Decks constructed with an Array of deck numbers, for example [1, 3]");
        }

        const currentDeckIndex = this.decks.indexOf(this.currentDeckNumber);
        let newDeckIndex = currentDeckIndex + 1;
        if (currentDeckIndex >= this.decks.length) {
            newDeckIndex = 0;
        }

        this.switchDeck(Deck.groupForNumber(this.decks[newDeckIndex]));
    }
    switchDeck(newGroup) {
        for (let property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                if (this[property] instanceof Component) {
                    if (this[property].disconnect !== undefined && typeof this[property].disconnect === "function") {
                        this[property].disconnect();
                    }
                    this[property].group = newGroup;
                    if (this[property].connect !== undefined && typeof this[property].connect === "function") {
                        this[property].connect();
                    }
                }
            }
        }
    }
    static groupForNumber(deckNumber) {
        return "[Channel" + deckNumber + "]";
    }
}

class Button extends Component {
    constructor(group, inKey) {
        super(group, inKey);
    }
}

class PushButton extends Button {
    constructor(group, inKey) {
        super(group, inKey);
    }
    input(pressed) {
        engine.setValue(this.group, this.inKey, pressed);
    }
}

class ToggleButton extends Button {
    constructor(group, inKey) {
        super(group, inKey);
    }
    input(pressed) {
        if (pressed) {
            script.toggleControl(this.group, this.inKey);
        }
    }
}

class Pot extends Component {
    constructor(group, inKey, max) {
        super(group, inKey);
        this.max = max;
        this.firstValueReceived = false;
    }
    input(value) {
        engine.setParameter(this.group, this.inKey, value/this.max);
        if (!this.firstValueReceived) {
            this.firstValueReceived = true;
            this.connect();
        }
    }
    connect() {
        engine.softTakeover(this.group, this.inKey, true);
    }
    disconnect() {
        engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
    }
}

const getBit = (byte, index) => {
    return (byte >> 8 - index) & 1;
};

class S4MK3 {
    constructor() {
        this.potMax = 2**12;
        this.leftDeck = new Deck([1, 3]);
        this.leftDeck.playButton = new ToggleButton(this.leftDeck.group, "play");
        this.leftDeck.cueButton = new PushButton(this.leftDeck.group, "cue_default");
        this.leftDeck.tempoFader = new Pot(this.leftDeck.group, "rate", this.potMax);
    }
    incomingData(data) {
        const reportId = data[0];
        // slice off the reportId
        if (reportId === 1) {
            let string;
            for (let byte of data) {
                string = string + byte.toString(2) + ",";
            }
            print(string);

            this.leftDeck.playButton.input(getBit(data[5], 8));
            this.leftDeck.cueButton.input(getBit(data[5], 7));

            if (getBit(data[6], 6)) {
                this.leftDeck.switchDeck(Deck.groupForNumber(1));
            } else if (getBit(data[6], 5)) {
                this.leftDeck.switchDeck(Deck.groupForNumber(3));
            }
        } else if (reportId === 2) {
            const buffer = data.buffer.slice(1);
            const view = new Uint16Array(buffer, 0, buffer.byteLength/2);

            engine.setParameter("[Master]", "crossfader", view[0]/this.potMax);

            engine.setParameter("[Channel1]", "volume", view[1]/this.potMax);
            engine.setParameter("[Channel2]", "volume", view[2]/this.potMax);
            engine.setParameter("[Channel3]", "volume", view[3]/this.potMax);
            engine.setParameter("[Channel4]", "volume", view[4]/this.potMax);

            engine.setParameter("[Channel2]", "rate", view[5]/this.potMax);
            this.leftDeck.tempoFader.input(view[6]);

            engine.setParameter("[Channel3]", "pregain", view[7]/this.potMax);
            engine.setParameter("[Channel1]", "pregain", view[8]/this.potMax);
            engine.setParameter("[Channel2]", "pregain", view[9]/this.potMax);
            engine.setParameter("[Channel4]", "pregain", view[10]/this.potMax);

            // These control the controller's audio interface in hardware, so do not map them.
            // engine.setParameter('[Master]', 'gain', view[11]/this.potMax);
            // engine.setParameter('[Master]', 'booth_gain', view[12]/this.potMax);
            // engine.setParameter('[Master]', 'headMix', view[13]/this.potMax);
            // engine.setParameter('[Master]', 'headVolume', view[14]/this.potMax);

            engine.setParameter("[EffectRack1_EffectUnit1]", "mix", view[15]/this.potMax);
            engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", view[16]/this.potMax);
            engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", view[17]/this.potMax);
            engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", view[18]/this.potMax);

            engine.setParameter("[EqualizerRack1_[Channel3]_Effect1]", "parameter3", view[19]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel3]_Effect1]", "parameter2", view[20]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel3]_Effect1]", "parameter1", view[21]/this.potMax);

            engine.setParameter("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", view[22]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", view[23]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", view[24]/this.potMax);

            engine.setParameter("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", view[25]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", view[26]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", view[27]/this.potMax);

            engine.setParameter("[EqualizerRack1_[Channel4]_Effect1]", "parameter3", view[28]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel4]_Effect1]", "parameter2", view[29]/this.potMax);
            engine.setParameter("[EqualizerRack1_[Channel4]_Effect1]", "parameter1", view[30]/this.potMax);

            engine.setParameter("[QuickEffectRack1_[Channel3]]", "super1", view[31]/this.potMax);
            engine.setParameter("[QuickEffectRack1_[Channel1]]", "super1", view[32]/this.potMax);
            engine.setParameter("[QuickEffectRack1_[Channel2]]", "super1", view[33]/this.potMax);
            engine.setParameter("[QuickEffectRack1_[Channel4]]", "super1", view[34]/this.potMax);

            engine.setParameter("[EffectRack1_EffectUnit2]", "mix", view[35]/this.potMax);
            engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", view[36]/this.potMax);
            engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", view[37]/this.potMax);
            engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", view[38]/this.potMax);
        }
    }
    init() {
    }
    shutdown() {
    }
}

var TraktorS4MK3 = new S4MK3();
