
#include "skin/colorschemeparser.h"

#include "widget/wpixmapstore.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

#include "skin/imgsource.h"
#include "skin/imgloader.h"
#include "skin/imgcolor.h"
#include "skin/imginvert.h"

void ColorSchemeParser::setupLegacyColorSchemes(QDomElement docElem,
                                                ConfigObject<ConfigValue>* pConfig) {
    QDomNode colsch = docElem.namedItem("Schemes");

    if (!colsch.isNull() && colsch.isElement()) {
        QString schname = pConfig->getValueString(ConfigKey("[Config]","Scheme"));
        QDomNode sch = colsch.firstChild();

        bool found = false;

        if (schname.isEmpty()) {
            // If no scheme stored, accept the first one in the file
            found = true;
        }

        while (!sch.isNull() && !found) {
            QString thisname = WWidget::selectNodeQString(sch, "Name");
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
            WSkinColor::setLoader(imsrc);
        } else {
            WPixmapStore::setLoader(QSharedPointer<ImgSource>());
            WSkinColor::setLoader(QSharedPointer<ImgSource>());
        }
    } else {
        WPixmapStore::setLoader(QSharedPointer<ImgSource>());
        WSkinColor::setLoader(QSharedPointer<ImgSource>());
    }
}

ImgSource* ColorSchemeParser::parseFilters(QDomNode filt) {

    // TODO: Move this code into ImgSource
    if (!filt.hasChildNodes()) {
        return 0;
    }

    ImgSource * ret = new ImgLoader();

    QDomNode f = filt.firstChild();

    while (!f.isNull()) {
        QString name = f.nodeName().toLower();
        if (name == "invert") {
            ret = new ImgInvert(ret);
        } else if (name == "hueinv") {
            ret = new ImgHueInv(ret);
        } else if (name == "add") {
            ret = new ImgAdd(ret, WWidget::selectNodeInt(f, "Amount"));
        } else if (name == "scalewhite") {
            ret = new ImgScaleWhite(ret, WWidget::selectNodeFloat(f, "Amount"));
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

            if (!f.namedItem("HMin").isNull()) { hmin = WWidget::selectNodeInt(f, "HMin"); }
            if (!f.namedItem("HMax").isNull()) { hmax = WWidget::selectNodeInt(f, "HMax"); }
            if (!f.namedItem("SMin").isNull()) { smin = WWidget::selectNodeInt(f, "SMin"); }
            if (!f.namedItem("SMax").isNull()) { smax = WWidget::selectNodeInt(f, "SMax"); }
            if (!f.namedItem("VMin").isNull()) { vmin = WWidget::selectNodeInt(f, "VMin"); }
            if (!f.namedItem("VMax").isNull()) { vmax = WWidget::selectNodeInt(f, "VMax"); }

            if (!f.namedItem("HConst").isNull()) { hconst = WWidget::selectNodeInt(f, "HConst"); }
            if (!f.namedItem("SConst").isNull()) { sconst = WWidget::selectNodeInt(f, "SConst"); }
            if (!f.namedItem("VConst").isNull()) { vconst = WWidget::selectNodeInt(f, "VConst"); }

            if (!f.namedItem("HFact").isNull()) { hfact = WWidget::selectNodeFloat(f, "HFact"); }
            if (!f.namedItem("SFact").isNull()) { sfact = WWidget::selectNodeFloat(f, "SFact"); }
            if (!f.namedItem("VFact").isNull()) { vfact = WWidget::selectNodeFloat(f, "VFact"); }

            ret = new ImgHSVTweak(ret, hmin, hmax, smin, smax, vmin, vmax, hfact, hconst,
                                  sfact, sconst, vfact, vconst);
        } else {
            qDebug() << "Unkown image filter:" << name;
        }
        f = f.nextSibling();
    }

    return ret;
}
