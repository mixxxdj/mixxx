#ifndef PREFERENCES_CONSTANTS_H
#define PREFERENCES_CONSTANTS_H

namespace mixxx {

// Don't change these constants since they are stored in user configuration
// files.
enum class TooltipsPreference {
    TOOLTIPS_OFF = 0,
    TOOLTIPS_ON = 1,
    TOOLTIPS_ONLY_IN_LIBRARY = 2,
};

}  // namespace mixxx

#endif /* PREFERENCES_CONSTANTS_H */
