#pragma once

#include "preferences/usersettings.h"
#include "skin/legacyskinparser.h"
#include "skin/skincontext.h"

class ImgSource;

class ColorSchemeParser {
  public:
    static void setupLegacyColorSchemes(
            const QDomElement& docElem,
            UserSettingsPointer pConfig,
            QString* pStyle,
            SkinContext* pContext);

  private:
    static ImgSource* parseFilters(const QDomNode& filter);
    ColorSchemeParser() { }
    ~ColorSchemeParser() { }
};
