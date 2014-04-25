
#include "widget/wnumberdb.h"

#include "util/math.h"
#include <QVBoxLayout>

#include "widget/wskincolor.h"

WNumberDb::WNumberDb(QWidget* pParent)
        : WLabel(pParent),
          m_iNoDigits(2) {
}

WNumberDb::~WNumberDb() {
}

void WNumberDb::setup(QDomNode node, const SkinContext& context) {
    WLabel::setup(node, context);

    // Number of digits after the decimal.
    if (context.hasNode(node, "NumberOfDigits")) {
        m_iNoDigits = context.selectInt(node, "NumberOfDigits");
    }

    setValue(0.);
}

void WNumberDb::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // We show the actual control value instead of its parameter.
    setValue(dValue);
}

void WNumberDb::setValue(double dValue) {
    QString strDb;
    if (dValue != 0.0) {
        double v = log10(dValue) * 20;
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
