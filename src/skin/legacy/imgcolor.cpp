#include "imgcolor.h"

QColor ImgHueInv::doColorCorrection(const QColor& c) const {
    int r, g, b, a;
    c.getRgb(&r, &g, &b, &a);
    return QColor(0xff - r, 0xff - g, 0xff - b, a);
}

QColor ImgHueRot::doColorCorrection(const QColor& c) const {
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);
    h = (h + m_amt) % 256;
    if (h < 0) { h += 256; }
    QColor ret;
    ret.setHsv(h, s, v, a);
    return ret;
}

QColor ImgScaleWhite::doColorCorrection(const QColor& c) const {
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);
    if (s < 50) {
        v = static_cast<int>(v * m_amt);
    }
    if (v > 255) { v = 255; }
    QColor ret;
    ret.setHsv(h, s, v, a);
    return ret;
}

ImgAdd::ImgAdd(ImgSource * parent, int amt)
    : ImgColorProcessor(parent), m_amt(amt) {
    // Nothing left to do
}

QColor ImgAdd::doColorCorrection(const QColor& c) const {
    int r = c.red() + m_amt;
    int g = c.green() + m_amt;
    int b = c.blue() + m_amt;
    if (r < 0) { r = 0; }
    if (g < 0) { g = 0; }
    if (b < 0) { b = 0; }
    if (r > 255) { r = 255; }
    if (g > 255) { g = 255; }
    if (b > 255) { b = 255; }
    return QColor(r, g, b, c.alpha());
}

ImgMax::ImgMax(ImgSource * parent, int amt)
    : ImgColorProcessor(parent), m_amt(amt) {
}

QColor ImgMax::doColorCorrection(const QColor& c) const {
    int r = c.red();
    int g = c.green();
    int b = c.blue();
    if (r > m_amt) { r = m_amt; }
    if (g > m_amt) { g = m_amt; }
    if (b > m_amt) { b = m_amt; }
    return QColor(r, g, b, c.alpha());
}


QColor ImgHSVTweak::doColorCorrection(const QColor& c) const {
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);

    if (h >= m_hmin && h <= m_hmax && s >= m_smin && s <= m_smax &&
        v >= m_vmin && v <= m_vmax) {
        h = static_cast<int>(h * m_hfact);
        s = static_cast<int>(s * m_sfact);
        v = static_cast<int>(v * m_vfact);
        h += m_hconst;
        s += m_sconst;
        v += m_vconst;

        h = h % 360;
        if (h < 0) { h += 360; }
        if (s < 0) { s = 0; }
        if (s > 255) { s = 255; }
        if (v < 0) { v = 0; }
        if (v > 255) { v = 255; }

        QColor ret;
        ret.setHsv(h, s, v, a);
        return ret;
    }
    return c;
}
