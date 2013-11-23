#ifndef HEXSPINBOX_H
#define HEXSPINBOX_H

#include <qspinbox.h>
#include <qvalidator.h>

class HexSpinBox : public QSpinBox
{
public:
    HexSpinBox(QWidget *parent);

protected:
    QString textFromValue(int value) const;
    int valueFromText(const QString& text) const;
    QValidator::State validate( QString & input, int & pos ) const;
};

#endif
