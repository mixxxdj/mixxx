import QtQuick
import "../LateNightTheme"

// Draws a Font Awesome Free glyph. Used instead of an SVG asset wherever an
// icon is needed, which keeps the icon set consistent without shipping artwork.
Text {
    id: root

    property color iconColor: LateNightTheme.textColor
    /// Font Awesome icon name, for example "play" or "wand-magic-sparkles".
    property string iconName: ""
    property real iconSize: 14

    readonly property string glyph: LateNightTheme.iconGlyph(root.iconName)

    color: root.iconColor
    font.family: LateNightTheme.iconFontFamily
    font.pixelSize: root.iconSize
    horizontalAlignment: Text.AlignHCenter
    text: root.glyph
    verticalAlignment: Text.AlignVCenter
}
