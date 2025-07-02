#pragma once

#include <Qt>
#include <memory>

#include "defs_urls.h"
#include "preferences/usersettings.h"

class AbstractLegacyControllerSetting;
class QBoxLayout;

/// @brief Layout information used for controller setting when rendered in the Preference Dialog
class LegacyControllerSettingsLayoutElement {
  public:
    LegacyControllerSettingsLayoutElement() {
    }
    virtual ~LegacyControllerSettingsLayoutElement() = default;

    virtual std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const = 0;

    virtual QWidget* build(QWidget* parent) = 0;

    virtual QList<LegacyControllerSettingsLayoutElement*> children() const = 0;
};

/// @brief This layout element can hold others element. It is also the one used
/// to represent a `row` in the settings
class LegacyControllerSettingsLayoutContainer : public LegacyControllerSettingsLayoutElement {
#ifdef MIXXX_USE_QML
    Q_GADGET
    Q_PROPERTY(Disposition disposition MEMBER m_disposition CONSTANT)
    Q_PROPERTY(Disposition widgetOrientation MEMBER m_widgetOrientation CONSTANT)
#endif
  public:
    /// @brief This is a simplified representation of disposition orientation. This used to
    /// define how a container orients its children. This is also used by layout
    /// items to decide how the label should be rendered alongside the input
    /// widget
    enum class Disposition {
        HORIZONTAL = 0,
        VERTICAL,
    };
#ifdef MIXXX_USE_QML
    Q_ENUM(Disposition);
#endif

    LegacyControllerSettingsLayoutContainer(
            Disposition disposition = Disposition::HORIZONTAL,
            Disposition widgetOrientation = Disposition::HORIZONTAL)
            : LegacyControllerSettingsLayoutElement(),
              m_disposition(disposition),
              m_widgetOrientation(widgetOrientation) {
    }
    LegacyControllerSettingsLayoutContainer(const LegacyControllerSettingsLayoutContainer& other) {
        m_elements.reserve(other.m_elements.size());
        for (const auto& e : other.m_elements) {
            m_elements.push_back(e->clone());
        }
    }
    virtual ~LegacyControllerSettingsLayoutContainer() = default;

    virtual std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsLayoutContainer>(*this);
    }

    /// @brief This helper method allows to add a LegacyControllerSetting
    /// directly, without to have it to wrap within an item object. This is
    /// helpful as the item that will be create to wrap will be initialised with
    /// the right parameters
    /// @param setting The controller setting to add to the layout container
    void addItem(std::shared_ptr<AbstractLegacyControllerSetting> setting);
    void addItem(std::unique_ptr<LegacyControllerSettingsLayoutContainer> container) {
        m_elements.push_back(std::move(container));
    }

    QWidget* build(QWidget* parent) override;

    LegacyControllerSettingsLayoutContainer::Disposition disposition() const {
        return m_disposition;
    }
    LegacyControllerSettingsLayoutContainer::Disposition widgetOrientation() const {
        return m_widgetOrientation;
    }
    QList<LegacyControllerSettingsLayoutElement*> children() const override {
        QList<LegacyControllerSettingsLayoutElement*> elements;
        for (const auto& element : m_elements) {
            elements.append(element.get());
        }
        return elements;
    }

  protected:
    QBoxLayout* buildLayout(QWidget* parent) const;

    Disposition m_disposition;
    Disposition m_widgetOrientation;
    std::vector<std::unique_ptr<LegacyControllerSettingsLayoutElement>> m_elements;
};

class LegacyControllerSettingsGroup : public LegacyControllerSettingsLayoutContainer {
  public:
    LegacyControllerSettingsGroup(const QString& label,
            LegacyControllerSettingsLayoutContainer::Disposition disposition =
                    Disposition::VERTICAL)
            : LegacyControllerSettingsLayoutContainer(disposition),
              m_label(label) {
    }
    virtual ~LegacyControllerSettingsGroup() = default;

    std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsGroup>(*this);
    }

    QWidget* build(QWidget* parent) override;
    LegacyControllerSettingsLayoutContainer::Disposition disposition() const {
        return m_disposition;
    }
    const QString& label() const {
        return m_label;
    }

  private:
    QString m_label;
};

class LegacyControllerSettingsLayoutItem : public LegacyControllerSettingsLayoutElement {
  public:
    LegacyControllerSettingsLayoutItem(
            std::shared_ptr<AbstractLegacyControllerSetting> setting,
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    LegacyControllerSettingsGroup::Disposition::HORIZONTAL)
            : LegacyControllerSettingsLayoutElement(),
              m_setting(setting),
              m_preferredOrientation(orientation) {
    }
    virtual ~LegacyControllerSettingsLayoutItem() = default;

    std::unique_ptr<LegacyControllerSettingsLayoutElement> clone() const override {
        return std::make_unique<LegacyControllerSettingsLayoutItem>(*this);
    }

    QWidget* build(QWidget* parent) override;

    QList<LegacyControllerSettingsLayoutElement*> children() const override {
        return {};
    }
    AbstractLegacyControllerSetting* setting() const {
        return m_setting.get();
    }
    LegacyControllerSettingsLayoutContainer::Disposition preferredOrientation() const {
        return m_preferredOrientation;
    }

  private:
    std::shared_ptr<AbstractLegacyControllerSetting> m_setting;
    LegacyControllerSettingsLayoutContainer::Disposition m_preferredOrientation;
};

class WLegacyControllerSettingsContainer : public QWidget {
    Q_OBJECT
  public:
    WLegacyControllerSettingsContainer(
            LegacyControllerSettingsLayoutContainer::Disposition
                    preferredOrientation,
            QWidget* parent)
            : QWidget(parent), m_preferredOrientation(preferredOrientation) {
    }

  protected:
    void resizeEvent(QResizeEvent* event);

  signals:
    void orientationChanged(LegacyControllerSettingsLayoutContainer::Disposition);

  private:
    LegacyControllerSettingsLayoutContainer::Disposition m_preferredOrientation;
};
