#include "imginvert.h"

QColor ImgInvert::doColorCorrection(const QColor& c) const {
    return QColor(0xff - c.red(),
                  0xff - c.green(),
                  0xff - c.blue(),
                  c.alpha());
}
