
#include "widget/wnumberdb.h"

#include "util/math.h"
#include <QVBoxLayout>

#include "widget/wskincolor.h"

WNumberDb::WNumberDb(QWidget* pParent)
        : WNumber(pParent) {
}

void WNumberDb::setValue(double dValue) {
    QString strDb;
    if (dValue != 0.0) {
        double v = ratio2db(dValue);
        strDb = QString::number(v, 'f', m_iNoDigits);
    } else {
        strDb = "-" + QString(QChar(0x221E));
    }

    if (m_skinText.contains("%1")) {
        setText(m_skinText.arg(strDb));
    } else {
        setText(m_skinText + strDb + " dB");
    }
}
