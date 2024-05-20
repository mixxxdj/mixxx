#pragma once

#include <QPlainTextEdit>

/// An implementation of QPlainTextTedit with a more sensible minimum
/// size of 2 text lines.
class WMultiLineTextEdit : public QPlainTextEdit {
    Q_OBJECT

  public:
    WMultiLineTextEdit(QWidget* parent = nullptr);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

  protected:
    bool event(QEvent* e) override;
    void keyPressEvent(QKeyEvent* event) override;

  private:
    QSize sizeHintImpl(const int minLines) const;
};
