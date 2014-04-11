#ifndef HEXSPINBOX_H
#define HEXSPINBOX_H

#include <QSpinBox>
#include <QValidator>

class HexSpinBox : public QSpinBox {
  public:
    HexSpinBox(QWidget *parent);
    virtual ~HexSpinBox();

  protected:
    QString textFromValue(int value) const;
    int valueFromText(const QString& text) const;
    QValidator::State validate(QString& input, int& pos) const;
};

#endif /* HEXSPINBOX_H */
