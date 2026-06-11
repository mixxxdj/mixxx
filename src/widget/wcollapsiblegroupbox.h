#pragma once

#include <QGroupBox>
#include <QProxyStyle>

/// Custom style for QGroupBox that renders the 'enabled' checkbox as ArrowDown/ArrowRight
/// icon and skips drawing the groupbox frame when the box is collapsed.
class CollapsibleGroupBoxStyle : public QProxyStyle {
    Q_OBJECT
  public:
    explicit CollapsibleGroupBoxStyle(QStyle* pStyle = nullptr)
            : QProxyStyle(pStyle) {
    }

    void drawPrimitive(QStyle::PrimitiveElement element,
            const QStyleOption* pOption,
            QPainter* pPainter,
            const QWidget* pWidget) const override;
};

/// This is a custom QGroupBox that can be collapsed/expanded and has
/// ArrowDown / ArrowRight icons for the 'enabled' checkbox.
class WCollapsibleGroupBox : public QGroupBox {
    Q_OBJECT
  public:
    explicit WCollapsibleGroupBox(QWidget* pParent = nullptr);
    explicit WCollapsibleGroupBox(const QString& title, QWidget* pParent = nullptr);

  protected:
    bool event(QEvent* pEvent) override;

  private slots:
    void slotToggled(bool checked);

  private:
    bool hasValidMinHeight() {
        // This covers invalid dimensions of QRect (-1) returned by style()->subControlRect()
        // as well as our initial value.
        return m_minHeight >= 0;
    }

    int m_minHeight;
    int m_maxHeight;
};
