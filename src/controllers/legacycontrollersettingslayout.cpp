
#include "controllers/legacycontrollersettingslayout.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

void LegacyControllerSettingsLayoutContainer::addItem(
        std::shared_ptr<AbstractLegacyControllerSetting> setting) {
    m_elements.push_back(std::make_unique<LegacyControllerSettingsLayoutItem>(setting));
}

QBoxLayout* LegacyControllerSettingsLayoutContainer::buildLayout(QWidget* pParent) const {
    QBoxLayout* pLayout = nullptr;

    /// Find the active screen. If its size is too small, we force vertical orientation
    QScreen* pActive = nullptr;
    QWidget* pWidget = pParent;

    while (pWidget) {
        auto* pW = pWidget->windowHandle();
        if (pW != nullptr) {
            pActive = pW->screen();
            break;
        } else {
            pWidget = pWidget->parentWidget();
        }
    }

    if (m_disposition == VERTICAL ||
            (pActive != nullptr &&
                    pActive->availableSize().width() <
                            MIN_SCREEN_SIZE_FOR_CONTROLLER_SETTING_ROW)) {
        pLayout = new QVBoxLayout();
    } else {
        pLayout = new QHBoxLayout();
        pLayout->setSpacing(16);
    }

    pParent->setLayout(pLayout);

    return pLayout;
}

QWidget* LegacyControllerSettingsLayoutContainer::build(QWidget* pParent) {
    QWidget* pContainer = new QWidget(pParent);
    QBoxLayout* pLayout = buildLayout(pContainer);

    pLayout->setContentsMargins(0, 0, 0, 0);

    for (auto& element : m_elements) {
        auto* pWidget = element->build(pContainer);
        if (pLayout->direction() == QBoxLayout::LeftToRight) {
            auto* pLayout = dynamic_cast<QBoxLayout*>(pWidget->layout());
            if (pLayout != nullptr) {
                pLayout->setDirection(QBoxLayout::TopToBottom);
            }
        }
        pLayout->addWidget(pWidget);
    }

    return pContainer;
}

QWidget* LegacyControllerSettingsGroup::build(QWidget* pParent) {
    QWidget* pContainer = new QGroupBox(m_label, pParent);
    QBoxLayout* pLayout = buildLayout(pContainer);

    for (auto& element : m_elements) {
        pLayout->addWidget(element->build(pContainer));
    }

    return pContainer;
}
