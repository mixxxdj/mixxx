/** ColorMapperJSProxy */

declare class ColorMapper {
    /**
     * Constructs a ColorMapper object which maps RGB colors to device specific color codes
     *
     * @param availableColors List of number pairs (e.g. {0xFF0000: 1, 0x00FF00: 2} )
     */
    constructor(availableColors: { [rgbColor: number]: number });

    /**
     * For a given RGB color code (e.g. 0xFF0000), this finds the nearest
     * available color and returns a JS object with properties "red", "green",
     * "blue" (each with value range 0-255).
     *
     * @param colorCode Device specific color code
     */
    getNearestColor(colorCode: number): { [rgb: number]: number };

    /**
     * For a given RGB color code (e.g. 0xFF0000), this finds the nearest
     * available color, then returns the value associated with that color
     * (which could be a MIDI byte value for example).
     *
     * @param rgbColor RGB color (e.g. 0xFF00CC)
     */
    getValueForNearestColor(rgbColor: number): number;
}
