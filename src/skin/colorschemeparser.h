#ifndef COLORSCHEMEPARSER_H
#define COLORSCHEMEPARSER_H

#include "preferences/usersettings.h"
#include "skin/legacyskinparser.h"
#include "skin/skincontext.h"

class ImgSource;

class ColorSchemeParser {
  public:
    static void setupLegacyColorSchemes(
            QDomElement docElem,
            UserSettingsPointer pConfig,
            QString* pStyle,
            const SkinContext& context);
  private:
    static ImgSource* parseFilters(QDomNode filter);
    ColorSchemeParser() { }
    ~ColorSchemeParser() { }
	SkinContext* *m_pContext;
};

#endif /* COLORSCHEMEPARSER_H */
