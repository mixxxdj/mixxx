#pragma once

#include <QGroupBox>
#include <QProxyStyle>

/// Custom style to draw the 'enabled' checkbox with ArrowDown / ArrowRight icons
/// and not draw the groupbox frame when the box is collapsed.
class CollapsibleGroupBoxStyle : public QProxyStyle {
    Q_OBJECT
  public:
    explicit CollapsibleGroupBoxStyle(QStyle* pStyle = 0)
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
    int m_minH;
    int m_maxH;
};
