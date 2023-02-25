#include "controllers/legacycontrollersettings.h"

#include "moc_legacycontrollersettings.cpp"
#include <util/assert.h>

#include <QCheckBox>
#include <QComboBox>

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::__self = nullptr;

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::instance() {
    if (__self == nullptr)
        __self = new LegacyControllerSettingBuilder();

    return __self;
}

QWidget* AbstractLegacyControllerSetting::buildWidget(QWidget* parent) {
    QWidget* root = new QWidget(parent);
    root->setLayout(new QHBoxLayout());
    root->layout()->setContentsMargins(0, 0, 0, 0);

    QLabel* labelWidget = new QLabel(root);
    labelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    labelWidget->setText(label());

    if (!description().isEmpty()) {
        auto layout = new QVBoxLayout(root);

        root->layout()->setContentsMargins(0, 0, 0, 0);

        auto descriptionWidget = new QLabel(root);
        descriptionWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        descriptionWidget->setText(description());
        QFont descriptionFont;
        descriptionFont.setPointSize(8);
        descriptionFont.setItalic(true);
        descriptionWidget->setFont(descriptionFont);

        layout->addWidget(labelWidget);
        layout->addWidget(descriptionWidget);
        root->layout()->addItem(layout);
    } else {
        root->layout()->addWidget(labelWidget);
    }

    root->layout()->addWidget(buildInputWidget(root));

    static_cast<QHBoxLayout*>(root->layout())->setStretch(0, 3);
    static_cast<QHBoxLayout*>(root->layout())->setStretch(1, 1);
    return root;
}

LegacyControllerBooleanSetting::LegacyControllerBooleanSetting(
        const QDomElement& element)
        : AbstractLegacyControllerSetting(element) {
    m_defaultValue = parseValue(element.attribute("default"));
    m_currentValue = m_defaultValue;
    m_dirtyValue = m_defaultValue;
}

QWidget* LegacyControllerBooleanSetting::buildWidget(QWidget* parent) {
    QWidget* root = new QWidget(parent);
    root->setLayout(new QHBoxLayout());
    root->layout()->setContentsMargins(0, 0, 0, 0);

    QLabel* labelWidget = new QLabel(root);
    labelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    labelWidget->setText(label());

    root->layout()->addWidget(buildInputWidget(root));

    if (!description().isEmpty()) {
        auto layout = new QVBoxLayout();

        layout->setContentsMargins(0, 0, 0, 0);

        auto descriptionWidget = new QLabel(root);
        descriptionWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        descriptionWidget->setText(description());
        QFont descriptionFont;
        descriptionFont.setPointSize(8);
        descriptionFont.setItalic(true);
        descriptionWidget->setFont(descriptionFont);

        layout->addWidget(labelWidget);
        layout->addWidget(descriptionWidget);

        root->layout()->addItem(layout);
    } else {
        root->layout()->addWidget(labelWidget);
    }

    return root;
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
    // TODO(acolombier) improve the function so it can detect the type from the
    // spec if there is no type attribute
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
    // TODO(acolombier) improve the function so it can detect the type from the
    // spec if there is no type attribute
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "integer",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerNumberSetting<int,
        packSettingIntegerValue,
        extractSettingIntegerValue,
        QSpinBox>);

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
    // TODO(acolombier) improve the function so it can detect the type from the
    // spec if there is no type attribute
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "real",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerRealSetting);

LegacyControllerEnumSetting::LegacyControllerEnumSetting(
        const QDomElement& element)
        : AbstractLegacyControllerSetting(element), m_options() {
    for (QDomElement value = element.firstChildElement("value");
            !value.isNull();
            value = value.nextSiblingElement("value")) {
        QString val = value.text();
        m_options.append(std::tuple<QString, QString>(val, value.attribute("label", val)));
    }
    m_defaultValue = extractSettingIntegerValue(element.attribute("default"));
    reset();
}

void LegacyControllerEnumSetting::parse(const QString& in, bool* ok) {
    if (ok != nullptr)
        *ok = false;
    reset();

    size_t pos = 0;
    for (const auto& value : m_options) {
        if (std::get<0>(value) == in) {
            if (ok != nullptr)
                *ok = true;
            m_currentValue = pos;
            m_dirtyValue = m_currentValue;
            return;
        }
        pos++;
    }
}

QWidget* LegacyControllerEnumSetting::buildInputWidget(QWidget* parent) {
    QComboBox* comboBox = new QComboBox(parent);

    for (const auto& value : m_options) {
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
    // TODO(acolombier) improve the function so it can detect the type from the
    // spec if there is no type attribute
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "enum",
                    Qt::CaseInsensitive) == 0;
}

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerEnumSetting);
