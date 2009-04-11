#include <qvalidator.h>

#include "hexspinbox.h"

HexSpinBox::HexSpinBox(QWidget *parent)
    : QSpinBox(parent)
{

    setRange(0, 255);
}

QString HexSpinBox::textFromValue(int value) const 
{
    //Construct a hex string formatted like 0x0f. 
    return QString("0x") + QString("%1").arg(value, 
                                2,   //Field width (makes "F" become "0F")
                                16, 
                                QLatin1Char('0')).toUpper();
}

int HexSpinBox::valueFromText(const QString& text) const
{
    bool ok;
    return text.toInt(&ok, 16);
}

QValidator::State HexSpinBox::validate ( QString & input, int & pos ) const
{
    const QRegExp regExp("^0(x|X)[0-9A-Fa-f]+");
    QRegExpValidator validator(regExp, NULL); 
    return validator.validate(input, pos);
}
