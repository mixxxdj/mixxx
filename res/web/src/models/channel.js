/**
 * Channel model — represents a mixer channel state.
 */
export class Channel {
    constructor(index) {
        this.index = index; // 1 to 4
        this.gain = 50;
        this.high = 50;
        this.mid = 50;
        this.bass = 50;
        this.super_fx = 50;
        this.volume = 100; // default Mixxx value is 1.0
    }
}
