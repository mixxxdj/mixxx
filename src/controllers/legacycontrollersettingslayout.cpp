
#include "controllers/legacycontrollersettingslayout.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

#include "controllers/legacycontrollersettings.h"

void LegacyControllerSettingsLayoutContainer::addItem(
        std::shared_ptr<AbstractLegacyControllerSetting> setting) {
    m_elements.push_back(std::make_unique<LegacyControllerSettingsLayoutItem>(
            setting, m_widgetOrientation));
}

QBoxLayout* LegacyControllerSettingsLayoutContainer::buildLayout(QWidget* pParent) const {
    QBoxLayout* pLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    pParent->setLayout(pLayout);

    return pLayout;
}

QWidget* LegacyControllerSettingsLayoutContainer::build(QWidget* pParent) {
    QWidget* pContainer = new WLegacyControllerSettingsContainer(m_disposition, pParent);
    QBoxLayout* pLayout = buildLayout(pContainer);

    pLayout->setContentsMargins(0, 0, 0, 0);

    for (auto& element : m_elements) {
        pLayout->addWidget(element->build(pContainer));
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

QWidget* LegacyControllerSettingsLayoutItem::build(QWidget* parent) {
    VERIFY_OR_DEBUG_ASSERT(m_setting.get() != nullptr) {
        return nullptr;
    }
    return m_setting->buildWidget(parent, m_prefferedOrientation);
}

void WLegacyControllerSettingsContainer::resizeEvent(QResizeEvent* event) {
    if (m_prefferedOrientation == LegacyControllerSettingsLayoutContainer::VERTICAL) {
        return;
    }

    auto* pLayout = dynamic_cast<QBoxLayout*>(layout());
    if (pLayout == nullptr) {
        return;
    }

    if (event->size().width() < MIN_SCREEN_SIZE_FOR_CONTROLLER_SETTING_ROW &&
            pLayout->direction() == QBoxLayout::LeftToRight) {
        pLayout->setDirection(QBoxLayout::TopToBottom);
        pLayout->setSpacing(6);
        emit orientationChanged(LegacyControllerSettingsLayoutContainer::VERTICAL);
    } else if (event->size().width() >=
                    MIN_SCREEN_SIZE_FOR_CONTROLLER_SETTING_ROW &&
            pLayout->direction() == QBoxLayout::TopToBottom) {
        pLayout->setDirection(QBoxLayout::LeftToRight);
        pLayout->setSpacing(16);
        emit orientationChanged(LegacyControllerSettingsLayoutContainer::HORIZONTAL);
    }
}
