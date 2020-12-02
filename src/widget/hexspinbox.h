#ifndef HEXSPINBOX_H
#define HEXSPINBOX_H

#include <QByteArrayData>
#include <QSpinBox>
#include <QString>
#include <QValidator>

class QObject;
class QWidget;

class HexSpinBox : public QSpinBox {
    Q_OBJECT
  public:
    explicit HexSpinBox(QWidget* pParent);

  protected:
    QString textFromValue(int value) const override;
    int valueFromText(const QString& text) const override;
    QValidator::State validate(QString& input, int& pos) const override;
};

#endif /* HEXSPINBOX_H */
