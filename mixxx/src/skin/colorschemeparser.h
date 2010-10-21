#ifndef COLORSCHEMEPARSER_H
#define COLORSCHEMEPARSER_H

#include "configobject.h"

class ImgSource;

class ColorSchemeParser {
  public:
    static void setupLegacyColorSchemes(QDomElement docElem, ConfigObject<ConfigValue>* pConfig);
  private:
    static ImgSource* parseFilters(QDomNode filter);
    ColorSchemeParser() { }
    ~ColorSchemeParser() { }
};

#endif /* COLORSCHEMEPARSER_H */
