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
    if (!isCheckable()) {
        // Only enable the special expand/collapse behavior
        // when the users has requested it via setCheckable(true).
        // Otherwise, this should just behave like a normal QGroupBox.
        return QGroupBox::event(pEvent);
    }

    switch (pEvent->type()) {
    case QEvent::Show: {
        if (!hasValidMinHeight()) {
            // Set the min height variable only on first show event.
            QStyleOptionGroupBox gbOpt;
            initStyleOption(&gbOpt);
            QRect titleRect;
            if (title().isEmpty()) {
                // In case we didn't set a title we use the rect of the original checkbox
                titleRect = style()->subControlRect(
                        QStyle::CC_GroupBox,
                        &gbOpt,
                        QStyle::SC_GroupBoxCheckBox,
                        this);
            } else {
                titleRect = style()->subControlRect(
                        QStyle::CC_GroupBox,
                        &gbOpt,
                        QStyle::SC_GroupBoxLabel,
                        this);
            }

            // Get margin of the focus frame (is usually drawn outside the widget's
            // regular frame)
            int focusVMargin = style()->pixelMetric(QStyle::PM_FocusFrameHMargin, &gbOpt, this);
            m_minHeight = titleRect.height() + focusVMargin;
        }

        // If this has been toggled 'collapsed' before it has been shown we
        // collapsed it now that we have a valid min height.
        if (isChecked()) {
            m_maxHeight = maximumHeight();
        } else {
            DEBUG_ASSERT(hasValidMinHeight());
            setMaximumHeight(m_minHeight);
        }
        break;
    }
    case QEvent::LayoutRequest:
    case QEvent::ContentsRectChange:
    case QEvent::Resize: {
        if (!isChecked() && hasValidMinHeight()) {
            // Content size has changed which triggers a paint event and
            // would draw the expanded box.
            // Though it is and should stay collapsed, so re-apply max height.
            setMaximumHeight(m_minHeight);
        } else if (isChecked() && m_maxHeight != maximumHeight()) {
            // The maximum height may have changed when the layout direction
            // changed, so also adopt it on each resize/layoutchange event.
            m_maxHeight = maximumHeight();
        }
        break;
    }
    default:
        break;
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
    if (isCheckable() && hasValidMinHeight()) {
        setMaximumHeight(checked ? m_maxHeight : m_minHeight);
    }
}
