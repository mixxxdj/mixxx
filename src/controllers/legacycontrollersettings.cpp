#include "controllers/legacycontrollersettings.h"

#include "moc_legacycontrollersettings.cpp"
#include <util/assert.h>

#include <QCheckBox>
#include <QComboBox>

// TODO (acolombier): remove "new" where possible and use parented_ptr

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

QWidget* AbstractLegacyControllerSetting::buildWidget(QWidget* parent) {
    QWidget* pRoot = new QWidget(parent);
    QBoxLayout* pLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    pLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* pLabelWidget = new QLabel(pRoot);
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

QWidget* LegacyControllerBooleanSetting::buildWidget(QWidget* parent) {
    QWidget* pRoot = new QWidget(parent);
    QBoxLayout* pLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    pLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* pLabelWidget = new QLabel(pRoot);
    pLabelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    pLabelWidget->setText(label());

    if (!description().isEmpty()) {
        pRoot->setToolTip(QString("<p>%1</p>").arg(description()));
    }

    pLayout->addWidget(buildInputWidget(pRoot));
    pLayout->addWidget(pLabelWidget);

    pRoot->setLayout(pLayout);

    return pRoot;
}

QWidget* LegacyControllerBooleanSetting::buildInputWidget(QWidget* parent) {
    QCheckBox* checkBox = new QCheckBox("", parent);
    checkBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    if (m_currentValue) {
        checkBox->setCheckState(Qt::Checked);
    }

    connect(checkBox, &QCheckBox::stateChanged, this, [this](int state) {
        m_dirtyValue = state == Qt::Checked;
        emit changed();
    });

    return checkBox;
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
        InputWidget>::buildInputWidget(QWidget* parent) {
    InputWidget* spinBox = new InputWidget(parent);
    spinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

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

QWidget* LegacyControllerRealSetting::buildInputWidget(QWidget* parent) {
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(
            LegacyControllerNumberSetting::buildInputWidget(parent));
    VERIFY_OR_DEBUG_ASSERT(spinBox != nullptr) {
        qWarning() << "Unable to set precision on the controller setting "
                      "input: tt does not appear to be a valid QDoubleSpinBox";
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

QWidget* LegacyControllerEnumSetting::buildInputWidget(QWidget* parent) {
    QComboBox* comboBox = new QComboBox(parent);

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
