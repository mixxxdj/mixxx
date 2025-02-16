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
          m_minHeight(-1),
          m_maxHeight(QWIDGETSIZE_MAX) {
    connect(this,
            &QGroupBox::toggled,
            this,
            &WCollapsibleGroupBox::slotToggled);
    // Set the custom style for the expand/collapse icons and other tweaks
    setStyle(new CollapsibleGroupBoxStyle(style()));
}

WCollapsibleGroupBox::WCollapsibleGroupBox(const QString& title, QWidget* pParent)
        : WCollapsibleGroupBox(pParent) {
    setTitle(title);
}

bool WCollapsibleGroupBox::event(QEvent* pEvent) {
    if (isCheckable()) {
        if (pEvent->type() == QEvent::Show) {
            if (m_minHeight == -1) {
                // Set the min height variable only on first show event.
                QStyleOptionGroupBox gbOpt;
                initStyleOption(&gbOpt);
                const QRect titleRect = style()->subControlRect(
                        QStyle::CC_GroupBox,
                        &gbOpt,
                        // In case we didn't set a title we use the rect of the original checkbox
                        title().isEmpty() ? QStyle::SC_GroupBoxCheckBox : QStyle::SC_GroupBoxLabel,
                        this);
                m_minHeight = titleRect.height();
                // The maximum height may change when the layout direction is changed,
                // so also set it on each resize/layoutchange event.
            }

            // If this has been toggled 'collapsed' before it has been shown we
            // collapsed it now that we have a valid min height.
            if (isChecked()) {
                m_maxHeight = maximumHeight();
            } else {
                setMaximumHeight(m_minHeight);
            }
        } else if (pEvent->type() == QEvent::LayoutRequest ||
                pEvent->type() == QEvent::ContentsRectChange ||
                pEvent->type() == QEvent::Resize) {
            if (!isChecked() && m_minHeight != -1) {
                // Content size has changed which triggers a paint event and
                // would draw the expanded box.
                // Though it is and should stay collapsed, so re-apply max height.
                setMaximumHeight(m_minHeight);
            } else if (isChecked() && m_maxHeight != maximumHeight()) {
                // Adpot the new max height
                m_maxHeight = maximumHeight();
            }
        }
    }
    return QGroupBox::event(pEvent);
}

void WCollapsibleGroupBox::slotToggled(bool checked) {
    // React to the 'toggled' signal. By now, QGroupBox would already have
    // enabled/disabled its children but that doesn't matter for us since we
    // show/hide them anyway.
    // Set the maximum height to show/hide the content.
    // This may be toggled before we acquired the min height, so only collapse
    // if we have a valid height.
    if (isCheckable() && m_minHeight != -1) {
        setMaximumHeight(checked ? m_maxHeight : m_minHeight);
    }
}
