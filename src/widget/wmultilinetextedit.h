#pragma once

#include <QPlainTextEdit>

/// An implementation of QPlainTextTedit with a more sensible minimum
/// size of 2 text lines.
///
/// Used for DlgTrackInfo/~Multi and similar dialogs.
class WMultiLineTextEdit : public QPlainTextEdit {
    Q_OBJECT
    Q_PROPERTY(bool upDownChangesFocus READ upDownChangesFocus WRITE setUpDownChangesFocus)

  public:
    WMultiLineTextEdit(QWidget* parent = nullptr);

    /// Sets whether to change focus to the next/previous sibling control
    /// when the Up/Down arrow keys are pressed while the cursor is at
    /// the very end or very start of the control.
    void setUpDownChangesFocus(bool enable) {
        m_upDownChangesFocus = enable;
    }
    bool upDownChangesFocus() const {
        return m_upDownChangesFocus;
    }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

  protected:
    bool event(QEvent* e) override;
    void keyPressEvent(QKeyEvent* event) override;

  private:
    QSize sizeHintImpl(const int minLines) const;
    bool m_upDownChangesFocus;
};
