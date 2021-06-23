#include "widget/wnumber.h"

#include "moc_wnumber.cpp"

WNumber::WNumber(QWidget* pParent)
        : WLabel(pParent),
          m_iNoDigits(2) {
}

void WNumber::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    // Number of digits after the decimal.
    context.hasNodeSelectInt(node, "NumberOfDigits", &m_iNoDigits);

    setValue(0.);
}

void WNumber::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dParameter);
    // We show the actual control value instead of its parameter.
    setValue(dValue);
}

void WNumber::setValue(double dValue) {
    if (m_skinText.contains("%1")) {
        setText(m_skinText.arg(QString::number(dValue, 'f', m_iNoDigits)));
    } else {
        setText(m_skinText + QString::number(dValue, 'f', m_iNoDigits));
    }
}
