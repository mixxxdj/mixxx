#include "widget/wnumberbeatsize.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"

WNumberBeatSize::WNumberBeatSize(QWidget * parent)
        : WNumber(parent) {
}

QString WNumberBeatSize::fractionString(int numerator, int denominator) {
    return QString("<sup>%1</sup>&frasl;<sub>%2</sub>").arg(numerator).arg(denominator);
}

void WNumberBeatSize::setValue(double beats) {
    if (beats == 0.5) {
        setText(fractionString(1, 2));
    } else if (beats == 0.25) {
        setText(fractionString(1, 4));
    } else if (beats == 0.125) {
        setText(fractionString(1, 8));
    } else if (beats == 0.0625) {
        setText(fractionString(1, 16));
    } else if (beats == 0.03125) {
        setText(fractionString(1, 32));
    } else {
        setText(QString::number(beats, 'f', 0));
    }
}
