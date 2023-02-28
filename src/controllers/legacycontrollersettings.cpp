#include "controllers/legacycontrollersettings.h"

#include "moc_legacycontrollersettings.cpp"
#include <util/assert.h>

#include <QCheckBox>
#include <QComboBox>

#include "util/parented_ptr.h"

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::__self = nullptr;

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::instance() {
    if (__self == nullptr) {
        __self = new LegacyControllerSettingBuilder();
    }

    return __self;
}

AbstractLegacyControllerSetting::AbstractLegacyControllerSetting(const QDomElement& element) {
    m_variableName = element.attribute("variable").trimmed();
    m_label = element.attribute("label", m_variableName).trimmed();

    QDomElement description = element.firstChildElement("description");
    if (!description.isNull()) {
        m_description = description.text().trimmed();
    }
}

QWidget* AbstractLegacyControllerSetting::buildWidget(QWidget* pParent,
        LegacyControllerSettingsLayoutContainer::Disposition orientation) {
    auto pRoot = make_parented<QWidget>(pParent);
    QBoxLayout* pLayout = new QBoxLayout(QBoxLayout::LeftToRight);

    pLayout->setContentsMargins(0, 0, 0, 0);

    if (orientation == LegacyControllerSettingsLayoutContainer::VERTICAL) {
        auto* pSettingsContainer = dynamic_cast<WLegacyControllerSettingsContainer*>(pParent);
        if (pSettingsContainer) {
            connect(pSettingsContainer,
                    &WLegacyControllerSettingsContainer::orientationChanged,
                    this,
                    [pLayout, pParent](
                            LegacyControllerSettingsLayoutContainer::Disposition
                                    disposition) {
                        pLayout->setDirection(disposition ==
                                                LegacyControllerSettingsLayoutContainer::
                                                        HORIZONTAL
                                        ? QBoxLayout::TopToBottom
                                        : QBoxLayout::LeftToRight);
                        pParent->layout()->invalidate();
                    });
        }
    }

    auto pLabelWidget = make_parented<QLabel>(pRoot);
    pLabelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    pLabelWidget->setText(label());

    if (!description().isEmpty()) {
        pRoot->setToolTip(QString("<p>%1</p>").arg(description()));
    }

    pLayout->addWidget(pLabelWidget);
    pLayout->addWidget(buildInputWidget(pRoot));

    pLayout->setStretch(0, 3);
    pLayout->setStretch(1, 1);

    pRoot->setLayout(pLayout);

    return pRoot;
}

LegacyControllerBooleanSetting::LegacyControllerBooleanSetting(
        const QDomElement& element)
        : AbstractLegacyControllerSetting(element) {
    m_defaultValue = parseValue(element.attribute("default"));
    m_currentValue = m_defaultValue;
    m_dirtyValue = m_defaultValue;
}

QWidget* LegacyControllerBooleanSetting::buildWidget(
        QWidget* pParent, LegacyControllerSettingsLayoutContainer::Disposition) {
    return buildInputWidget(pParent);
}

QWidget* LegacyControllerBooleanSetting::buildInputWidget(QWidget* pParent) {
    auto pCheckBox = make_parented<QCheckBox>(label(), pParent);
    pCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    if (m_currentValue) {
        pCheckBox->setCheckState(Qt::Checked);
    }

    if (!description().isEmpty()) {
        pCheckBox->setToolTip(QString("<p>%1</p>").arg(description()));
    }

    connect(pCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        m_dirtyValue = state == Qt::Checked;
        emit changed();
    });

    return pCheckBox;
}

bool LegacyControllerBooleanSetting::match(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "boolean",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerBooleanSetting);

template<class SettingType,
        Serializer<SettingType> ValueSerializer,
        Deserializer<SettingType> ValueDeserializer,
        class InputWidget>
LegacyControllerNumberSetting<SettingType,
        ValueSerializer,
        ValueDeserializer,
        InputWidget>::LegacyControllerNumberSetting(const QDomElement& element)
        : AbstractLegacyControllerSetting(element) {
    bool isOk = false;
    m_minValue = ValueDeserializer(element.attribute("min"), &isOk);
    if (!isOk) {
        m_minValue = std::numeric_limits<int>::min();
    }
    m_maxValue = ValueDeserializer(element.attribute("max"), &isOk);
    if (!isOk) {
        m_maxValue = std::numeric_limits<int>::max();
    }
    m_stepValue = ValueDeserializer(element.attribute("step"), &isOk);
    if (!isOk) {
        m_stepValue = 1;
    }
    m_defaultValue = ValueDeserializer(element.attribute("default"), &isOk);
    if (!isOk) {
        m_defaultValue = 0;
    }
    reset();
}

template<class SettingType,
        Serializer<SettingType> ValueSerializer,
        Deserializer<SettingType> ValueDeserializer,
        class InputWidget>
QWidget* LegacyControllerNumberSetting<SettingType,
        ValueSerializer,
        ValueDeserializer,
        InputWidget>::buildInputWidget(QWidget* pParent) {
    auto spinBox = make_parented<InputWidget>(pParent);
    spinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    spinBox->setRange(this->m_minValue, this->m_maxValue);
    spinBox->setSingleStep(this->m_stepValue);
    spinBox->setValue(this->m_currentValue);

    connect(spinBox,
            QOverload<SettingType>::of(&InputWidget::valueChanged),
            this,
            [this](SettingType value) {
                m_dirtyValue = value;
                emit changed();
            });

    return spinBox;
}

template<class SettingType,
        Serializer<SettingType> ValueSerializer,
        Deserializer<SettingType> ValueDeserializer,
        class InputWidget>
bool LegacyControllerNumberSetting<SettingType,
        ValueSerializer,
        ValueDeserializer,
        InputWidget>::match(const QDomElement& element) {
    return matchSetting<SettingType>(element);
}

template<>
bool matchSetting<int>(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "integer",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerIntegerSetting);

LegacyControllerRealSetting::LegacyControllerRealSetting(const QDomElement& element)
        : LegacyControllerNumberSetting(element) {
    bool isOk = false;
    m_precisionValue = element.attribute("precision").toInt(&isOk);
    if (!isOk) {
        m_precisionValue = 2;
    }
}

QWidget* LegacyControllerRealSetting::buildInputWidget(QWidget* pParent) {
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(
            LegacyControllerNumberSetting::buildInputWidget(pParent));
    VERIFY_OR_DEBUG_ASSERT(spinBox != nullptr) {
        qWarning() << "Unable to set precision on the input widget "
                      "input: it does not appear to be a valid QDoubleSpinBox";
        return spinBox;
    }
    spinBox->setDecimals(m_precisionValue);

    return spinBox;
}

template<>
bool matchSetting<double>(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "real",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerRealSetting);

LegacyControllerEnumSetting::LegacyControllerEnumSetting(
        const QDomElement& element)
        : AbstractLegacyControllerSetting(element), m_options(), m_defaultValue(0) {
    size_t pos = 0;
    for (QDomElement value = element.firstChildElement("value");
            !value.isNull();
            value = value.nextSiblingElement("value")) {
        QString val = value.text();
        m_options.append(std::tuple<QString, QString>(val, value.attribute("label", val)));
        if (value.hasAttribute("default")) {
            m_defaultValue = pos;
        }
        pos++;
    }
    reset();
}

void LegacyControllerEnumSetting::parse(const QString& in, bool* ok) {
    if (ok != nullptr) {
        *ok = false;
    }
    reset();

    size_t pos = 0;
    for (const auto& value : qAsConst(m_options)) {
        if (std::get<0>(value) == in) {
            if (ok != nullptr) {
                *ok = true;
            }
            m_currentValue = pos;
            m_dirtyValue = m_currentValue;
            return;
        }
        pos++;
    }
}

QWidget* LegacyControllerEnumSetting::buildInputWidget(QWidget* pParent) {
    auto comboBox = make_parented<QComboBox>(pParent);

    for (const auto& value : qAsConst(m_options)) {
        comboBox->addItem(std::get<1>(value));
    }
    comboBox->setCurrentIndex(m_currentValue);

    connect(comboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int selected) {
                m_dirtyValue = selected;
                emit changed();
            });

    return comboBox;
}

bool LegacyControllerEnumSetting::match(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "enum",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerEnumSetting);
