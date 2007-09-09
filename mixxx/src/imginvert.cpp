#include "imginvert.h"

QColor ImgInvert::doColorCorrection(QColor c) {
    return QColor(0xff - c.red(),
                  0xff - c.green(),
                  0xff - c.blue());
}

