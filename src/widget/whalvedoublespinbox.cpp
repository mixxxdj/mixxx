#include "widget/whalvedoublespinbox.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"

WHalveDoubleSpinBox::WHalveDoubleSpinBox(QWidget * parent,
                                         ControlObject* pValueControl,
                                         int decimals,
                                         double minimum, double maximum)
        : WBaseWidget(parent),
          m_valueControl(
            pValueControl ?
            pValueControl->getKey() : ConfigKey(), this
          ) {
    setDecimals(decimals);
    setMinimum(minimum);
    setMaximum(maximum);

    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotSpinboxValueChanged(double)));

    setValue(m_valueControl.get());
    m_valueControl.connectValueChanged(SLOT(slotControlValueChanged(double)));
    connect(this, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateControlValue(double)));
}

void WHalveDoubleSpinBox::stepBy(int steps) {
    double newValue = value() * pow(2, steps);
    if (newValue < minimum()) {
        newValue = minimum();
    } else if (newValue > maximum()) {
        newValue = maximum();
    }
    setValue(newValue);
}

void WHalveDoubleSpinBox::slotSpinboxValueChanged(double newValue) {
    m_valueControl.set(newValue);
}

void WHalveDoubleSpinBox::slotControlValueChanged(double newValue) {
    if (value() != newValue) {
        setValue(newValue);
    }
}

QString WHalveDoubleSpinBox::fractionString(int numerator, int denominator) const {
    return QString("%1/%2").arg(numerator).arg(denominator);
}

QString WHalveDoubleSpinBox::textFromValue(double value) const {
    double dWholePart, dFracPart;
    dFracPart = modf(value, &dWholePart);

    QString sFracPart;
    if (dFracPart == 0.5) {
        sFracPart = fractionString(1, 2);
    } else if (dFracPart == 0.25) {
        sFracPart = fractionString(1, 4);
    } else if (dFracPart == 0.75) {
        sFracPart = fractionString(3, 4);
    } else if (dFracPart == 0.33333) {
        sFracPart = fractionString(1, 3);
    } else if (dFracPart == 0.66667) {
        sFracPart = fractionString(2, 3);
    } else if (dFracPart == 0.125) {
        sFracPart = fractionString(1, 8);
    } else if (dFracPart == 0.375) {
        sFracPart = fractionString(3, 8);
    } else if (dFracPart == 0.625) {
        sFracPart = fractionString(5, 8);
    } else if (dFracPart == 0.875) {
        sFracPart = fractionString(7, 8);
    } else if (dFracPart == 0.0625) {
        sFracPart = fractionString(1, 16);
    } else if (dFracPart == 0.1875) {
        sFracPart = fractionString(3, 16);
    } else if (dFracPart == 0.3125) {
        sFracPart = fractionString(5, 16);
    } else if (dFracPart == 0.4375) {
        sFracPart = fractionString(7, 16);
    } else if (dFracPart == 0.5625) {
        sFracPart = fractionString(9, 16);
    } else if (dFracPart == 0.6875) {
        sFracPart = fractionString(11, 16);
    } else if (dFracPart == 0.8125) {
        sFracPart = fractionString(13, 16);
    } else if (dFracPart == 0.9375) {
        sFracPart = fractionString(15, 16);
    } else if (dFracPart == 0.03125) {
        sFracPart = fractionString(1, 32);
    } else if (dFracPart == 0.09375) {
        sFracPart = fractionString(3, 32);
    } else if (dFracPart == 0.15625) {
        sFracPart = fractionString(5, 32);
    } else if (dFracPart == 0.21875) {
        sFracPart = fractionString(7, 32);
    } else if (dFracPart == 0.28125) {
        sFracPart = fractionString(9, 32);
    } else if (dFracPart == 0.34375) {
        sFracPart = fractionString(11, 32);
    } else if (dFracPart == 0.40625) {
        sFracPart = fractionString(13, 32);
    } else if (dFracPart == 0.46875) {
        sFracPart = fractionString(15, 32);
    } else if (dFracPart == 0.53125) {
        sFracPart = fractionString(17, 32);
    } else if (dFracPart == 0.59375) {
        sFracPart = fractionString(19, 32);
    } else if (dFracPart == 0.65625) {
        sFracPart = fractionString(21, 32);
    } else if (dFracPart == 0.71875) {
        sFracPart = fractionString(23, 32);
    } else if (dFracPart == 0.78125) {
        sFracPart = fractionString(25, 32);
    } else if (dFracPart == 0.84375) {
        sFracPart = fractionString(27, 32);
    } else if (dFracPart == 0.90625) {
        sFracPart = fractionString(29, 32);
    } else if (dFracPart == 0.96875) {
        sFracPart = fractionString(31, 32);
    }

    if (dWholePart > 0) {
        if (sFracPart.isEmpty()) {
            if (dFracPart == 0.00000) {
                return QString::number(dWholePart, 'f', 0);
            } else {
                return QString::number(value, 'f', 5);
            }
        }
        return QString::number(dWholePart, 'f', 0) + " " + sFracPart;
    } else {
        if (sFracPart.isEmpty() ) {
            return QString::number(value, 'f', 5);
        }
        return sFracPart;
    }
}

double WHalveDoubleSpinBox::valueFromText(const QString& text) const {
    if (text.count(" ") > 1 || text.count("/") > 1) {
        return value();
    }

    bool conversionWorked = false;
    double dValue;
    dValue = text.toDouble(&conversionWorked);
    if (conversionWorked) {
        return dValue;
    }

    QString sIntPart, sFracPart, sNumerator, sDenominator;

    if (text.contains(" ")) {
        QStringList numberParts = text.split(" ");
        sIntPart = numberParts.at(0);
        sFracPart = numberParts.at(1);
    } else if (text.contains("/")) {
        sFracPart = text;
    }

    QStringList splitFraction = sFracPart.split("/");
    sNumerator = splitFraction.at(0);
    sDenominator = splitFraction.at(1);

    return sIntPart.toDouble() + sNumerator.toDouble() / sDenominator.toDouble();
}

QValidator::State WHalveDoubleSpinBox::validate(QString& input, int& pos) const {
    if (input.isEmpty()) {
        return QValidator::Intermediate;
    }

    if (input.count(" ") > 1 || input.count("/") > 1) {
        return QValidator::Invalid;
    }

    bool conversionWorked = false;
    input.toDouble(&conversionWorked);
    if (conversionWorked) {
        return QValidator::Acceptable;
    }

    QString sIntPart, sFracPart, sNumerator, sDenominator;

    if (input.contains(" ")) {
        // Whole number + fraction, for example "1 1/2"
        QStringList numberParts = input.split(" ");
        sIntPart = numberParts.at(0);
        sFracPart = numberParts.at(1);
        // Whole number + trailing space
        if (sFracPart.isEmpty()) {
            return QValidator::Intermediate;
        }
    } else if (input.contains("/")) {
        // Fraction without whole number, for example "1/2"
        sFracPart = input;
    }

    if (!sIntPart.isEmpty()) {
        conversionWorked = false;
        sIntPart.toInt(&conversionWorked);
        if (!conversionWorked) {
            return QValidator::Invalid;
        }
    }

    if (!sFracPart.contains("/")) {
        return QValidator::Intermediate;
    }
    QStringList fracParts = sFracPart.split("/");
    sNumerator = fracParts.at(0);
    if (sNumerator.isEmpty()) {
        // when deleting the numerator, for example "/32"
        return QValidator::Intermediate;
    }
    sDenominator = fracParts.at(1);

    conversionWorked = false;
    sNumerator.toInt(&conversionWorked);
    if (!conversionWorked) {
        return QValidator::Invalid;
    }

    if (sDenominator.isEmpty()) {
        return QValidator::Intermediate;
    }
    conversionWorked = false;
    sDenominator.toInt(&conversionWorked);
    if (!conversionWorked) {
        return QValidator::Invalid;
    }

    return QValidator::Acceptable;
}
