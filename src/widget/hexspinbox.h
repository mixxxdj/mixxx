#ifndef HEXSPINBOX_H
#define HEXSPINBOX_H

#include <QSpinBox>
#include <QValidator>

class HexSpinBox : public QSpinBox {
  public:
    explicit HexSpinBox(QWidget *pParent);

  protected:
    QString textFromValue(int value) const override;
    int valueFromText(const QString& text) const override;
    QValidator::State validate(QString& input, int& pos) const override;
};

#endif /* HEXSPINBOX_H */
