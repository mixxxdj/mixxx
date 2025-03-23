
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
#include "moc_legacycontrollersettingslayout.cpp"
#include "util/parented_ptr.h"
#include "widget/wcollapsiblegroupbox.h"

namespace {
constexpr int kMinScreenSizeForControllerSettingRow = 960;
} // anonymous namespace

void LegacyControllerSettingsLayoutContainer::addItem(
        std::shared_ptr<AbstractLegacyControllerSetting> setting) {
    m_elements.push_back(std::make_unique<LegacyControllerSettingsLayoutItem>(
            setting, m_widgetOrientation));
}

QBoxLayout* LegacyControllerSettingsLayoutContainer::buildLayout(QWidget* pParent) const {
    auto pLayout = make_parented<QBoxLayout>(QBoxLayout::TopToBottom, pParent);

    pParent->setLayout(pLayout);

    return pLayout;
}

QWidget* LegacyControllerSettingsLayoutContainer::build(QWidget* pParent) {
    auto pContainer = make_parented<WLegacyControllerSettingsContainer>(m_disposition, pParent);
    QBoxLayout* pLayout = buildLayout(pContainer);

    pLayout->setContentsMargins(0, 0, 0, 0);

    auto& lastElement = m_elements.back();
    for (auto& element : m_elements) {
        auto* pWidget = element->build(pContainer);
        pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        pLayout->addWidget(pWidget);
        if (element != lastElement) {
            pLayout->addItem(new QSpacerItem(
                    10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
        }
    }

    return pContainer;
}

QWidget* LegacyControllerSettingsGroup::build(QWidget* pParent) {
    auto pContainer = make_parented<WCollapsibleGroupBox>(m_label, pParent);
    QBoxLayout* pLayout = buildLayout(pContainer);

    // Note: Don't set checkable here, yet! We do this for top-level groups
    // only in DlgPrefController.

    for (auto& element : m_elements) {
        pLayout->addWidget(element->build(pContainer));
    }

    return pContainer;
}

QWidget* LegacyControllerSettingsLayoutItem::build(QWidget* parent) {
    VERIFY_OR_DEBUG_ASSERT(m_setting.get() != nullptr) {
        return nullptr;
    }
    return m_setting->buildWidget(parent, m_preferredOrientation);
}

void WLegacyControllerSettingsContainer::resizeEvent(QResizeEvent* event) {
    if (m_preferredOrientation == LegacyControllerSettingsLayoutContainer::VERTICAL) {
        return;
    }

    auto* pLayout = dynamic_cast<QBoxLayout*>(layout());
    if (pLayout == nullptr) {
        return;
    }

    if (event->size().width() < kMinScreenSizeForControllerSettingRow &&
            pLayout->direction() == QBoxLayout::LeftToRight) {
        pLayout->setDirection(QBoxLayout::TopToBottom);
        pLayout->setSpacing(6);
        emit orientationChanged(LegacyControllerSettingsLayoutContainer::VERTICAL);
    } else if (event->size().width() >=
                    kMinScreenSizeForControllerSettingRow &&
            pLayout->direction() == QBoxLayout::TopToBottom) {
        pLayout->setDirection(QBoxLayout::LeftToRight);
        pLayout->setSpacing(16);
        emit orientationChanged(LegacyControllerSettingsLayoutContainer::HORIZONTAL);
    }
}
