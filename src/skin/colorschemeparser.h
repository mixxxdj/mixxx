#ifndef COLORSCHEMEPARSER_H
#define COLORSCHEMEPARSER_H

#include "preferences/usersettings.h"

class ImgSource;

class ColorSchemeParser {
  public:
    static void setupLegacyColorSchemes(QDomElement docElem, UserSettingsPointer pConfig);
  private:
    static ImgSource* parseFilters(QDomNode filter);
    ColorSchemeParser() { }
    ~ColorSchemeParser() { }
};

#endif /* COLORSCHEMEPARSER_H */
