#include "widget/wcollapsiblegroupbox.h"

#include <QEvent>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wcollapsiblegroupbox.cpp"
#include "util/assert.h"

void CollapsibleGroupBoxStyle::drawPrimitive(QStyle::PrimitiveElement element,
        const QStyleOption* pOption,
        QPainter* pPainter,
        const QWidget* pWidget) const {
    const WCollapsibleGroupBox* pCGB = qobject_cast<const WCollapsibleGroupBox*>(pWidget);
    if (pCGB && pCGB->isCheckable()) {
        if (element == QStyle::PE_IndicatorCheckBox) {
            // Draw Down/Right arrow instead of original checkbox
            QProxyStyle::drawPrimitive(pCGB->isChecked()
                            ? QStyle::PE_IndicatorArrowDown
                            : QStyle::PE_IndicatorArrowRight,
                    pOption,
                    pPainter,
                    pWidget);
            return;
        } else if (!pCGB->isChecked() && element == QStyle::PE_FrameGroupBox) {
            // When the box is collapsed, the default style might still draw the
            // frame, e.g. underneath the title, even though the box is reduced
            // to its title row, so skip drawing the frame.
            return;
        }
    }

    QProxyStyle::drawPrimitive(element, pOption, pPainter, pWidget);
}

WCollapsibleGroupBox::WCollapsibleGroupBox(QWidget* pParent)
        : QGroupBox(pParent),
          m_minH(-1),
          m_maxH(10000) {
    connect(this,
            &QGroupBox::toggled,
            this,
            &WCollapsibleGroupBox::slotToggled);
    // Set the custom style for the expand/collapse icons and other tweaks
    setStyle(new CollapsibleGroupBoxStyle(style()));
    setFocusPolicy(Qt::NoFocus);
}

WCollapsibleGroupBox::WCollapsibleGroupBox(const QString& title, QWidget* pParent)
        : WCollapsibleGroupBox(pParent) {
    setTitle(title);
}

bool WCollapsibleGroupBox::event(QEvent* pEvent) {
    if (isCheckable()) {
        if (pEvent->type() == QEvent::Show) {
            if (m_minH == -1) {
                // Set the min height variable only on first show event.
                QStyleOptionGroupBox gbOpt;
                initStyleOption(&gbOpt);
                const QRect titleRect = style()->subControlRect(
                        QStyle::CC_GroupBox,
                        &gbOpt,
                        // In case we didn't set a title we use the rect of the original checkbox
                        title().isEmpty() ? QStyle::SC_GroupBoxCheckBox : QStyle::SC_GroupBoxLabel,
                        this);
                m_minH = titleRect.height();
                // The maximum height may change when the layout direction is changed,
                // so also set it on each resize/layoutchange event.
            }

            // If this has been toggled 'collapsed' before it has been shown we
            // collapsed it now that we have a valid min height.
            if (isChecked()) {
                m_maxH = maximumHeight();
            } else {
                setMaximumHeight(m_minH);
            }
        } else if (pEvent->type() == QEvent::LayoutRequest ||
                pEvent->type() == QEvent::ContentsRectChange ||
                pEvent->type() == QEvent::Resize) {
            if (!isChecked() && m_minH != -1) {
                // Content size has changed which triggers a paint event and
                // would draw the expanded box.
                // Though it is and should stay collapsed, so re-apply max height.
                setMaximumHeight(m_minH);
            } else if (isChecked() && m_maxH != maximumHeight()) {
                // Adpot the new max height
                m_maxH = maximumHeight();
            }
        }
    }
    return QGroupBox::event(pEvent);
}

void WCollapsibleGroupBox::slotToggled(bool checked) {
    // React to the 'toggled' signal. By now, QGroupBox would already have
    // enable/disable its children but that doesn't matter for us since we
    // show/hide them anyway.
    // Set the maximum height to show/hide the content.
    // This may be toggled before we acquired the min height, so only collapse
    // if we have a valid height.
    if (isCheckable() && m_minH != -1) {
        setMaximumHeight(checked ? m_maxH : m_minH);
        updateGeometry();
    }
}
