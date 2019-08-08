#include "util/color/color.h"

#include "util/math.h"

namespace Color {

const PredefinedColorsSet kPredefinedColorsSet = PredefinedColorsSet();

// algorithm by http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
// NOTE(Swiftb0y): please suggest if I should you use other methods
// (like the W3C algorithm) or if this approach is to to performance hungry
// NOTE: the author did not take alpha transparency into account!
int brightness(int red, int green, int blue) {
    return static_cast<int>(sqrtf(
            red * red * .241 +
            green * green * .691 +
            blue * blue * .068));
};

// If the baseColor is darker than the global threshold,
// returns a lighter color, otherwise returns a darker color.
QColor chooseContrastColor(QColor baseColor) {
    // Will produce a color that is 60% brighter.
    static const int iLighterFactor = 160;
    // We consider a hsv color dark if its value is <= 20% of max value
    static const int iMinimumValue = 20 * 255 / 100;

    // Convert to Hsv to make sure the conversion only happens once.
    // QColor::darker() and QColor::lighter() internally convert from and to Hsv if the color is not already in Hsv.
    baseColor = baseColor.toHsv();

    QColor lightColor;
    QColor darkColor = baseColor.darker().toRgb();
    // QColor::lighter() multiplies the HSV Value by some factor. When baseColor is dark, Value is near 0,
    // thus after multiplication it's still near 0 and we get roughly the same color.
    // We manually set lightColor in this case.
    if (baseColor.value() <= iMinimumValue) {
        lightColor = baseColor;
        int newValue = iMinimumValue * iLighterFactor / 100;
        lightColor.setHsl(baseColor.hue(), baseColor.saturation(), newValue);
    } else {
        lightColor = baseColor.lighter(iLighterFactor);
    }

    // Even though we have the HSV representation for the color here, the "value" component alone is
    // not a good indicator of a color brightness (saturation comes into play too).
    // That's why we call chooseColorByBrightness so the proper brightness of the color is used
    // to choose between the light and the dark colors.
    QColor contrastColor = chooseColorByBrightness(baseColor, lightColor.toRgb(), darkColor.toRgb());
    return contrastColor;
}

} // namespace Color
