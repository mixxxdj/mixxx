
/** ColorMapperJSProxy */

declare class ColorMapper {
// Passing a QMap<QRgb, QVariant> argument to the constructor as needed by
    // the ColorMapper constructor segfaults. QJSEngine converts a JS object to
    // a QVariantMap, so this constructor converts the QVariantMap to a
    // QMap<QRgb, QVariant>.
    constructor (availableColors: { [rgb: number]: number });

    /// For a given RGB color code (e.g. 0xFF0000), this finds the nearest
    /// available color and returns a JS object with properties "red", "green",
    /// "blue" (each with value range 0-255).
    getNearestColor(ColorCode: number): {[rgb: number]: number};

    /// For a given RGB color code (e.g. 0xFF0000), this finds the nearest
    /// available color, then returns the value associated with that color
    /// (which could be a MIDI byte value for example).
    getValueForNearestColor(ColorCode: number): number;
}
