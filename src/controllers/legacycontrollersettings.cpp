#include "controllers/legacycontrollersettings.h"

#include <util/assert.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>

#include "moc_legacycontrollersettings.cpp"
#include "widget/wsettingscheckboxlabel.h"

namespace {

// Regex that allows to use :hwbtn:`LABELSTRING` in xml controller settings xml
const QRegularExpression kHwbtnRe(QStringLiteral(":hwbtn:`([^`]*)`"));

const QString kHwbtnStyleWrapper = QStringLiteral(
        "<span style='"
        "background: #343131;"
        "color: #d9d9d9;"
        "font-size: 70%;"
        "font-weight: 600;"
        "line-height: 120%;"
        "text-transform: uppercase;"
        // Padding does only work with block and table-cell elements, not
        // inline span. Hence, wrapping the string in `&nbsp;` is the only way
        // to get the desired look here. See the Qt documentation for details:
        // https://doc.qt.io/qt-6/richtext-html-subset.html#css-properties
        // \\1 is the RegEx match group 1.
        "'>&nbsp;\\1&nbsp;</span>");

QString replaceMarkupStyleStr(QString str) {
    return str.replace(kHwbtnRe, kHwbtnStyleWrapper);
}

} // namespace

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::instance() {
    static LegacyControllerSettingBuilder* s_self = nullptr;

    if (s_self == nullptr) {
        s_self = new LegacyControllerSettingBuilder();
    }

    return s_self;
}

LegacyControllerSettingBuilder::LegacyControllerSettingBuilder() {
    // Each possible setting types must be added there. This will allow the
    // builder to know each type of supported setting
    registerType<LegacyControllerBooleanSetting>();
    registerType<LegacyControllerIntegerSetting>();
    registerType<LegacyControllerRealSetting>();
    registerType<LegacyControllerEnumSetting>();
}

AbstractLegacyControllerSetting::AbstractLegacyControllerSetting(const QDomElement& element) {
    m_variableName = element.attribute("variable").trimmed();
    m_label = replaceMarkupStyleStr(element.attribute("label", m_variableName).trimmed());

    QDomElement description = element.firstChildElement("description");
    if (!description.isNull()) {
        m_description = replaceMarkupStyleStr(description.text().trimmed());
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
    m_savedValue = m_defaultValue;
    m_editedValue = m_defaultValue;
}

QWidget* LegacyControllerBooleanSetting::buildWidget(
        QWidget* pParent, LegacyControllerSettingsLayoutContainer::Disposition) {
    return buildInputWidget(pParent);
}

QWidget* LegacyControllerBooleanSetting::buildInputWidget(QWidget* pParent) {
    auto pWidget = make_parented<QWidget>(pParent);

    auto* pCheckBox = new QCheckBox(pWidget);
    pCheckBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    if (m_editedValue) {
        pCheckBox->setCheckState(Qt::Checked);
    }

    if (!description().isEmpty()) {
        pCheckBox->setToolTip(QString("<p>%1</p>").arg(description()));
    }

    connect(this, &AbstractLegacyControllerSetting::valueReset, pCheckBox, [this, pCheckBox]() {
        pCheckBox->setCheckState(m_editedValue ? Qt::Checked : Qt::Unchecked);
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(pCheckBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
#else
    connect(pCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
#endif
        m_editedValue = state == Qt::Checked;
        emit changed();
    });

    // We want to format the checkbox label with html styles. This is not possible
    // so we attach a separate label. In order to get a clickable label like
    // with QCheckBox, we use a custom QLabel that toggles its buddy QCheckBox
    // (on left-click, like QCheckBox) and sets focus on it.
    auto pLabelWidget = make_parented<WSettingsCheckBoxLabel>(pWidget);
    pLabelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    pLabelWidget->setText(label());
    pLabelWidget->setBuddy(pCheckBox);

    QBoxLayout* pLayout = new QHBoxLayout();

    pLayout->addWidget(pCheckBox);
    pLayout->addWidget(pLabelWidget);

    pLayout->setStretch(0, 3);
    pLayout->setStretch(1, 1);

    pWidget->setLayout(pLayout);

    return pWidget;
}

bool LegacyControllerBooleanSetting::match(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "boolean",
                    Qt::CaseInsensitive) == 0;
}

template<class SettingType,
        Serializer<SettingType> ValueSerializer,
        Deserializer<SettingType> ValueDeserializer,
        class InputWidget>
QWidget* LegacyControllerNumberSetting<SettingType,
        ValueSerializer,
        ValueDeserializer,
        InputWidget>::buildInputWidget(QWidget* pParent) {
    auto* pSpinBox = new InputWidget(pParent);
    pSpinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    pSpinBox->setRange(m_minValue, m_maxValue);
    pSpinBox->setSingleStep(m_stepValue);
    pSpinBox->setValue(m_editedValue);

    connect(this, &AbstractLegacyControllerSetting::valueReset, pSpinBox, [this, pSpinBox]() {
        pSpinBox->setValue(m_editedValue);
    });

    connect(pSpinBox,
            QOverload<SettingType>::of(&InputWidget::valueChanged),
            this,
            [this](SettingType value) {
                m_editedValue = value;
                emit changed();
            });

    return pSpinBox;
}

QWidget* LegacyControllerRealSetting::buildInputWidget(QWidget* pParent) {
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(
            LegacyControllerNumberSetting::buildInputWidget(pParent));
    VERIFY_OR_DEBUG_ASSERT(spinBox != nullptr) {
        qWarning() << "Unable to set precision on the input widget "
                      "input. It does not appear to be a valid QDoubleSpinBox";
        return spinBox;
    }
    spinBox->setDecimals(m_precisionValue);

    return spinBox;
}

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
    save();
}

void LegacyControllerEnumSetting::parse(const QString& in, bool* ok) {
    if (ok != nullptr) {
        *ok = false;
    }
    reset();
    save();

    size_t pos = 0;
    for (const auto& value : std::as_const(m_options)) {
        if (std::get<0>(value) == in) {
            if (ok != nullptr) {
                *ok = true;
            }
            m_savedValue = pos;
            m_editedValue = m_savedValue;
            return;
        }
        pos++;
    }
}

QWidget* LegacyControllerEnumSetting::buildInputWidget(QWidget* pParent) {
    auto* pComboBox = new QComboBox(pParent);

    for (const auto& value : std::as_const(m_options)) {
        pComboBox->addItem(std::get<1>(value));
    }
    pComboBox->setCurrentIndex(static_cast<int>(m_editedValue));

    connect(this, &AbstractLegacyControllerSetting::valueReset, pComboBox, [this, pComboBox]() {
        pComboBox->setCurrentIndex(static_cast<int>(m_editedValue));
    });

    connect(pComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int selected) {
                m_editedValue = selected;
                emit changed();
            });

    return pComboBox;
}
