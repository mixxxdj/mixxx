#include "skin/legacy/colorschemeparser.h"

#include "widget/wpixmapstore.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"
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
    QDomNode schemeNode = findConfiguredColorSchemeNode(docElem, pConfig);

    if (!schemeNode.isNull()) {
        std::shared_ptr<ImgSource> pImgSrc =
                std::shared_ptr<ImgSource>(parseFilters(schemeNode.namedItem("Filters")));
        WPixmapStore::setLoader(pImgSrc);
        WImageStore::setLoader(pImgSrc);
        WSkinColor::setLoader(pImgSrc);

        // This calls SkinContext::updateVariables which iterates over all
        // <SetVariable> nodes in the selected color scheme node.
        pContext->updateVariables(schemeNode);

        if (pStyle) {
            // read scheme's stylesheet (node text or stylesheet file)
            *pStyle = LegacySkinParser::getStyleFromNode(schemeNode);
        }
    } else {
        std::shared_ptr<ImgSource> pImgSrc = std::make_shared<ImgLoader>();
        WPixmapStore::setLoader(pImgSrc);
        WImageStore::setLoader(pImgSrc);
        WSkinColor::setLoader(pImgSrc);
    }
}

QDomNode ColorSchemeParser::findConfiguredColorSchemeNode(
        const QDomElement& docElem,
        UserSettingsPointer pConfig) {
    QDomNode schemesNode = docElem.namedItem("Schemes");
    if (schemesNode.isNull() || !schemesNode.isElement()) {
        return {};
    }

    QDomNode schemeNode = schemesNode.firstChild();
    const QString selectedSchemeName = pConfig->getValueString(ConfigKey("[Config]", "Scheme"));
    if (selectedSchemeName.isEmpty()) {
        // If no scheme selected, accept the first one in the file
        return schemeNode;
    }

    while (!schemeNode.isNull()) {
        QString schemeName = XmlParse::selectNodeQString(schemeNode, "Name");
        if (schemeName == selectedSchemeName) {
            return schemeNode;
        }
        schemeNode = schemeNode.nextSibling();
    }

    // If we didn't find a matching color scheme, pick the first one
    return schemesNode.firstChild();
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
