
#include "skin/colorschemeparser.h"

#include "widget/wpixmapstore.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "util/xml.h"
#include "skin/imgsource.h"
#include "skin/imgloader.h"
#include "skin/imgcolor.h"
#include "skin/imginvert.h"
#include "skin/legacyskinparser.h"

void ColorSchemeParser::setupLegacyColorSchemes(QDomElement docElem,
                                                UserSettingsPointer pConfig,
                                                QString* pStyle) {
    QDomNode colsch = docElem.namedItem("Schemes");

    bool found = false;

    if (!colsch.isNull() && colsch.isElement()) {
        QString schname = pConfig->getValueString(ConfigKey("[Config]","Scheme"));
        QDomNode sch = colsch.firstChild();

        if (schname.isEmpty()) {
            // If no scheme stored, accept the first one in the file
            found = true;
        }

        while (!sch.isNull() && !found) {
            QString thisname = XmlParse::selectNodeQString(sch, "Name");
            if (thisname == schname) {
                found = true;
            } else {
                sch = sch.nextSibling();
            }
        }

        if (found) {
            QSharedPointer<ImgSource> imsrc =
                    QSharedPointer<ImgSource>(parseFilters(sch.namedItem("Filters")));
            WPixmapStore::setLoader(imsrc);
            WImageStore::setLoader(imsrc);
            WSkinColor::setLoader(imsrc);

            if (pStyle) {
                *pStyle = LegacySkinParser::getStyleFromNode(sch);
            }
        }
    }
    if (!found) {
        QSharedPointer<ImgSource> imsrc =
                QSharedPointer<ImgSource>(new ImgLoader());
        WPixmapStore::setLoader(imsrc);
        WImageStore::setLoader(imsrc);
        WSkinColor::setLoader(imsrc);
    }
}

ImgSource* ColorSchemeParser::parseFilters(QDomNode filt) {
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
