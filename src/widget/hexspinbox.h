#pragma once

#include <QSpinBox>
#include <QValidator>

class HexSpinBox : public QSpinBox {
    Q_OBJECT
  public:
    explicit HexSpinBox(QWidget* pParent);

  protected:
    QString textFromValue(int value) const override;
    int valueFromText(const QString& text) const override;
    QValidator::State validate(QString& input, int& pos) const override;
};
