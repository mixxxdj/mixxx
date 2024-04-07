#pragma once

#include <QDoubleSpinBox>

// This spinbox accepts , and . as decimal separator which comes in handy when
// the used (typed) decimal separator doesn't match  the locale's separator for
// some reason, e.g. when typing with numpads that have a fixed separator char.
class WDoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT
  public:
    explicit WDoubleSpinBox(QWidget* pParent);

    double valueFromText(const QString& text) const override;

  private:
    const QChar m_decSep;
};
