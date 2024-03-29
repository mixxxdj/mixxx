#include "widget/hexspinbox.h"

#include "moc_hexspinbox.cpp"

namespace {
const QRegularExpression kValidationRegex(QStringLiteral("^0(x|X)[0-9A-Fa-f]+"));
} // namespace

HexSpinBox::HexSpinBox(QWidget* pParent)
        : QSpinBox(pParent) {
    setRange(0, 255);
}

QString HexSpinBox::textFromValue(int value) const {
    // Construct a hex string formatted like 0xFF.
    return QString("0x") + QString("%1")
            .arg(value, 2, 16, QLatin1Char('0')).toUpper();
}

int HexSpinBox::valueFromText(const QString& text) const {
    bool ok;
    return text.toInt(&ok, 16);
}

QValidator::State HexSpinBox::validate(QString& input, int& pos) const {
    QRegularExpressionValidator validator(kValidationRegex, nullptr);
    return validator.validate(input, pos);
}
