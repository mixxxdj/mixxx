#ifndef COLOR_H
#define COLOR_H

#include "util/math.h"

#define BRIGHTNESS_TRESHOLD 130

// algorithm by http://www.nbdtech.com/Blog/archive/2008/04/27/Calculating-the-Perceived-Brightness-of-a-Color.aspx
// NOTE(Swiftb0y): please suggest if I should you use other methods
// (like the W3C algorithm) or if this approach is to to performance hungry
inline int brightness(int red, int green, int blue) {
   return static_cast<int>(sqrtf(
      red * red * .241 +
      green * green * .691 +
      blue * blue * .068));
};
inline bool isDimmColor(int red, int green, int blue) {
    return brightness(red,green,blue) < BRIGHTNESS_TRESHOLD;
}

#endif /* COLOR_H */
