#include "skin/legacy/colorschemeparser.h"

#include "widget/wpixmapstore.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "util/xml.h"
#include "skin/legacy/imgsource.h"
#include "skin/legacy/imgloader.h"
#include "skin/legacy/imgcolor.h"
#include "skin/legacy/imginvert.h"
#include "skin/legacy/legacyskinparser.h"
#include "skin/legacy/skincontext.h"

void ColorSchemeParser::setupLegacyColorSchemes(const QDomElement& docElem,
        UserSettingsPointer pConfig,
        QString* pStyle,
        SkinContext* pContext) {
    QDomNode schemesNode = docElem.namedItem("Schemes");

    bool bSelectedColorSchemeFound = false;

    if (!schemesNode.isNull() && schemesNode.isElement()) {
        QString selectedSchemeName = pConfig->getValueString(ConfigKey("[Config]","Scheme"));
        QDomNode schemeNode = schemesNode.firstChild();

        if (selectedSchemeName.isEmpty()) {
            // If no scheme selected, accept the first one in the file
            bSelectedColorSchemeFound = true;
        }

        while (!schemeNode.isNull() && !bSelectedColorSchemeFound) {
            QString schemeName = XmlParse::selectNodeQString(schemeNode, "Name");
            if (schemeName == selectedSchemeName) {
                bSelectedColorSchemeFound = true;
            } else {
                schemeNode = schemeNode.nextSibling();
            }
        }

        if (!bSelectedColorSchemeFound) {
            // If we didn't find a matching color scheme, pick the first one
            schemeNode = schemesNode.firstChild();
            bSelectedColorSchemeFound = !schemeNode.isNull();
        }

        if (bSelectedColorSchemeFound) {
            QSharedPointer<ImgSource> imsrc =
                    QSharedPointer<ImgSource>(parseFilters(schemeNode.namedItem("Filters")));
            WPixmapStore::setLoader(imsrc);
            WImageStore::setLoader(imsrc);
            WSkinColor::setLoader(imsrc);

            // This calls SkinContext::updateVariables in skincontext.cpp which
            // iterates over all <SetVariable> nodes in the selected color scheme node
            pContext->updateVariables(schemeNode);

            if (pStyle) {
                *pStyle = LegacySkinParser::getStyleFromNode(schemeNode);
            }
        }
    }

    if (!bSelectedColorSchemeFound) {
        QSharedPointer<ImgSource> imsrc =
                QSharedPointer<ImgSource>(new ImgLoader());
        WPixmapStore::setLoader(imsrc);
        WImageStore::setLoader(imsrc);
        WSkinColor::setLoader(imsrc);
    }
}

ImgSource* ColorSchemeParser::parseFilters(const QDomNode& filt) {
    ImgSource* ret = new ImgLoader();

    if (!filt.hasChildNodes()) {
        return ret;
    }

    QDomNode f = filt.firstChild();

    while (!f.isNull()) {
        QString name = f.nodeName().toLower();
        if (name == "invert") {
            ret = new ImgInvert(ret);
        } else if (name == "hueinv") {
            ret = new ImgHueInv(ret);
        } else if (name == "add") {
            ret = new ImgAdd(ret, XmlParse::selectNodeInt(f, "Amount"));
        } else if (name == "scalewhite") {
            ret = new ImgScaleWhite(ret, XmlParse::selectNodeFloat(f, "Amount"));
        } else if (name == "hsvtweak") {
            int hmin = 0;
            int hmax = 359;
            int smin = 0;
            int smax = 255;
            int vmin = 0;
            int vmax = 255;
            float hfact = 1.0f;
            float sfact = 1.0f;
            float vfact = 1.0f;
            int hconst = 0;
            int sconst = 0;
            int vconst = 0;

            if (!f.namedItem("HMin").isNull()) { hmin = XmlParse::selectNodeInt(f, "HMin"); }
            if (!f.namedItem("HMax").isNull()) { hmax = XmlParse::selectNodeInt(f, "HMax"); }
            if (!f.namedItem("SMin").isNull()) { smin = XmlParse::selectNodeInt(f, "SMin"); }
            if (!f.namedItem("SMax").isNull()) { smax = XmlParse::selectNodeInt(f, "SMax"); }
            if (!f.namedItem("VMin").isNull()) { vmin = XmlParse::selectNodeInt(f, "VMin"); }
            if (!f.namedItem("VMax").isNull()) { vmax = XmlParse::selectNodeInt(f, "VMax"); }

            if (!f.namedItem("HConst").isNull()) { hconst = XmlParse::selectNodeInt(f, "HConst"); }
            if (!f.namedItem("SConst").isNull()) { sconst = XmlParse::selectNodeInt(f, "SConst"); }
            if (!f.namedItem("VConst").isNull()) { vconst = XmlParse::selectNodeInt(f, "VConst"); }

            if (!f.namedItem("HFact").isNull()) { hfact = XmlParse::selectNodeFloat(f, "HFact"); }
            if (!f.namedItem("SFact").isNull()) { sfact = XmlParse::selectNodeFloat(f, "SFact"); }
            if (!f.namedItem("VFact").isNull()) { vfact = XmlParse::selectNodeFloat(f, "VFact"); }

            ret = new ImgHSVTweak(ret, hmin, hmax, smin, smax, vmin, vmax, hfact, hconst,
                                  sfact, sconst, vfact, vconst);
        } else {
            qDebug() << "Unknown image filter:" << name;
        }
        f = f.nextSibling();
    }

    return ret;
}
