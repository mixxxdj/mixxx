#include "widget/wdoublespinbox.h"

#include <QLineEdit>
#include <QLocale>

#include "moc_wdoublespinbox.cpp"

WDoubleSpinBox::WDoubleSpinBox(QWidget* pParent)
        : QDoubleSpinBox(pParent),
          m_decSep(QLocale().decimalPoint()) {
    lineEdit()->setValidator(new QRegExpValidator(
            QRegExp("[0-9]{1,5}[." +
                    // add locale decimal point if it's not a dot
                    (m_decSep != '.' ? m_decSep : QChar()) +
                    "]?[0-9]{,8}"),
            this));
}

// This gets the text from lineEdit()
double WDoubleSpinBox::valueFromText(const QString& text) const {
    QString temp = text;
    // replace , with dot before toDouble()
    temp.replace(",", ".");
    return temp.toDouble();
}
