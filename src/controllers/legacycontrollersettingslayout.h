#pragma once

#include <util/assert.h>

#include <QDebug>
#include <QDomElement>
#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <QtGlobal>
#include <memory>
#include <vector>

#include "controllers/legacycontrollersettings.h"
#include "defs_urls.h"
#include "preferences/usersettings.h"

#define MIN_SCREEN_SIZE_FOR_CONTROLLER_SETTING_ROW 1280

class LegacyControllerSettingsLayoutElement {
  public:
    LegacyControllerSettingsLayoutElement() {
    }
    virtual ~LegacyControllerSettingsLayoutElement() = default;

    virtual std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const = 0;

    virtual QWidget* build(QWidget* parent) = 0;
};

class LegacyControllerSettingsLayoutContainer : public LegacyControllerSettingsLayoutElement {
  public:
    enum Disposition {
        HORIZONTAL = 0,
        VERTICAL,
    };

    LegacyControllerSettingsLayoutContainer(Disposition disposition = HORIZONTAL)
            : LegacyControllerSettingsLayoutElement(), m_disposition(disposition) {
    }
    LegacyControllerSettingsLayoutContainer(const LegacyControllerSettingsLayoutContainer& other) {
        m_elements.reserve(other.m_elements.size());
        for (const auto& e : other.m_elements)
            m_elements.push_back(e->clone());
    }
    virtual ~LegacyControllerSettingsLayoutContainer() = default;

    virtual std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsLayoutContainer>(*this);
    }

    void addItem(std::shared_ptr<AbstractLegacyControllerSetting> setting);
    void addItem(std::unique_ptr<LegacyControllerSettingsLayoutContainer>&& container) {
        m_elements.push_back(std::move(container));
    }

    virtual QWidget* build(QWidget* parent) override;

  protected:
    QBoxLayout* buildLayout(QWidget* parent) const;

    Disposition m_disposition;
    std::vector<std::unique_ptr<LegacyControllerSettingsLayoutElement>> m_elements;
};

class LegacyControllerSettingsGroup : public LegacyControllerSettingsLayoutContainer {
  public:
    LegacyControllerSettingsGroup(const QString& label,
            LegacyControllerSettingsLayoutContainer::Disposition disposition =
                    VERTICAL)
            : LegacyControllerSettingsLayoutContainer(disposition),
              m_label(label) {
    }
    virtual ~LegacyControllerSettingsGroup() = default;

    std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsGroup>(*this);
    }

    QWidget* build(QWidget* parent) override;

  private:
    QString m_label;
};

class LegacyControllerSettingsLayoutItem : public LegacyControllerSettingsLayoutElement {
  public:
    LegacyControllerSettingsLayoutItem(std::shared_ptr<AbstractLegacyControllerSetting> setting)
            : LegacyControllerSettingsLayoutElement(), m_setting(setting) {
    }
    virtual ~LegacyControllerSettingsLayoutItem() = default;

    std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsLayoutItem>(*this);
    }

    QWidget* build(QWidget* parent) override {
        VERIFY_OR_DEBUG_ASSERT(m_setting.get() != nullptr) {
            return nullptr;
        }
        return m_setting->buildWidget(parent);
    }

  private:
    std::shared_ptr<AbstractLegacyControllerSetting> m_setting;
};
