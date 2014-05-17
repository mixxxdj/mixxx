
#include "widget/wnumberdb.h"

#include "util/math.h"
#include <QVBoxLayout>

#include "widget/wskincolor.h"

WNumberDb::WNumberDb(QWidget* pParent)
        : WNumber(pParent) {
}

WNumberDb::~WNumberDb() {
}


void WNumberDb::setValue(double dValue) {
    QString strDb;
    if (dValue != 0.0) {
        double v = ratio2db(dValue);
        strDb = QString::number(v, 'f', m_iNoDigits);
    } else {
        strDb = "-" + QString(QChar(0x221E));
    }

    if (m_qsText.contains("%1")) {
        setText(m_qsText.arg(strDb));
    } else {
        setText(m_qsText + strDb + " dB");
    }
}
