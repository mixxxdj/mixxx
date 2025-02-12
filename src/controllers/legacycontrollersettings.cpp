#include "controllers/legacycontrollersettings.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QStringLiteral>

#include "moc_legacycontrollersettings.cpp"
#include "util/assert.h"
#include "util/parented_ptr.h"

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
    registerType<LegacyControllerColorSetting>();
    registerType<LegacyControllerFileSetting>();
}

AbstractLegacyControllerSetting::AbstractLegacyControllerSetting(const QDomElement& element) {
    m_variableName = element.attribute(QStringLiteral("variable")).trimmed();
    m_label = replaceMarkupStyleStr(
            element.attribute(QStringLiteral("label"), m_variableName)
                    .trimmed());

    QDomElement description = element.firstChildElement(QStringLiteral("description"));
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
        pRoot->setToolTip(QStringLiteral("<p>%1</p>").arg(description()));
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
        : LegacyControllerSettingMixin(element) {
    m_defaultValue = parseValue(element.attribute(QStringLiteral("default")));
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
        pCheckBox->setToolTip(QStringLiteral("<p>%1</p>").arg(description()));
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
    // so we attach a separate QLabel and, in order to get a clickable label like
    // with QCheckBox, we associate the label with the checkbox (buddy).
    // When the label is clicked we toggle the checkbox in eventFilter().
    auto pLabelWidget = make_parented<QLabel>(pWidget);
    pLabelWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    pLabelWidget->setText(label());
    pLabelWidget->setBuddy(pCheckBox);
    pLabelWidget->installEventFilter(this);

    QBoxLayout* pLayout = new QHBoxLayout();

    pLayout->addWidget(pCheckBox);
    pLayout->addWidget(pLabelWidget);

    pLayout->setStretch(0, 3);
    pLayout->setStretch(1, 1);

    pWidget->setLayout(pLayout);

    return pWidget;
}

bool LegacyControllerBooleanSetting::match(const QDomElement& element) {
    return element.hasAttribute(QStringLiteral("type")) &&
            QString::compare(element.attribute(QStringLiteral("type")),
                    QStringLiteral("boolean"),
                    Qt::CaseInsensitive) == 0;
}

bool LegacyControllerBooleanSetting::eventFilter(QObject* pObj, QEvent* pEvent) {
    QLabel* pLabel = qobject_cast<QLabel*>(pObj);
    if (pLabel && pEvent->type() == QEvent::MouseButtonPress) {
        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pLabel->buddy());
        if (pCheckBox) {
            pCheckBox->toggle();
        }
    }
    return QObject::eventFilter(pObj, pEvent);
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
        : LegacyControllerSettingMixin(element, 0),
          m_options() {
    size_t pos = 0;
    for (QDomElement value = element.firstChildElement(QStringLiteral("value"));
            !value.isNull();
            value = value.nextSiblingElement(QStringLiteral("value"))) {
        QString val = value.text();
        QColor color = QColor(value.attribute(QStringLiteral("color")));
        // TODO: Remove once we mandate GCC 10/Clang 16
#if defined(__cpp_aggregate_paren_init) &&       \
        __cpp_aggregate_paren_init >= 201902L && \
        !defined(_MSC_VER) // FIXME: Bug in MSVC preventing the use of this feature
        m_options.emplace_back(val, value.attribute(QStringLiteral("label"), val), color);
#else
        m_options.emplace_back(Item{val, value.attribute(QStringLiteral("label"), val), color});
#endif
        if (value.hasAttribute(QStringLiteral("default"))) {
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
    for (const auto& item : std::as_const(m_options)) {
        if (item.value == in) {
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

    for (const auto& item : std::as_const(m_options)) {
        if (item.color.isValid()) {
            QPixmap icon(24, 24);
            QPainter painter(&icon);
            painter.fillRect(0, 0, 24, 24, item.color);
            pComboBox->addItem(QIcon(icon), item.label);
        } else {
            pComboBox->addItem(item.label);
        }
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

LegacyControllerColorSetting::LegacyControllerColorSetting(
        const QDomElement& element)
        : LegacyControllerSettingMixin(element,
                  QColor(element.attribute(QStringLiteral("default")))) {
}

LegacyControllerColorSetting::~LegacyControllerColorSetting() = default;

void LegacyControllerColorSetting::parse(const QString& in, bool* ok) {
    if (ok != nullptr) {
        *ok = false;
    }
    reset();
    save();

    m_savedValue = QColor(in);
    if (ok != nullptr) {
        *ok = m_editedValue.isValid();
    }
    if (!m_editedValue.isValid()) {
        return;
    }
    m_editedValue = m_savedValue;
}

QWidget* LegacyControllerColorSetting::buildInputWidget(QWidget* pParent) {
    auto* pPushButton = new QPushButton(tr("Change color"), pParent);

    auto setColorIcon = [pPushButton](const QColor& color) {
        QPixmap icon(24, 24);
        QPainter painter(&icon);
        painter.fillRect(0, 0, 24, 24, color);
        pPushButton->setIcon(QIcon(icon));
    };

    connect(this,
            &AbstractLegacyControllerSetting::valueReset,
            pPushButton,
            [this, pPushButton, setColorIcon]() {
                if (m_editedValue.isValid()) {
                    setColorIcon(m_editedValue);
                } else {
                    pPushButton->setIcon(QIcon());
                }
            });

    connect(pPushButton, &QPushButton::clicked, this, [this, pPushButton, setColorIcon](bool) {
        auto color = QColorDialog::getColor(m_editedValue, pPushButton, tr("Choose a new color"));
        if (color.isValid()) {
            m_editedValue = color;
            setColorIcon(color);
            emit changed();
        }
    });

    if (m_savedValue.isValid()) {
        setColorIcon(m_savedValue);
    }

    return pPushButton;
}

LegacyControllerFileSetting::LegacyControllerFileSetting(
        const QDomElement& element)
        : LegacyControllerSettingMixin(element,
                  QFileInfo(element.attribute(QStringLiteral("default")))),
          m_fileFilter(element.attribute(QStringLiteral("pattern"))) {
}
LegacyControllerFileSetting::~LegacyControllerFileSetting() = default;

void LegacyControllerFileSetting::parse(const QString& in, bool* ok) {
    if (ok != nullptr) {
        *ok = false;
    }
    reset();
    save();

    m_editedValue = QFileInfo(in);
    if (ok != nullptr) {
        *ok = m_editedValue.exists();
    }
    if (!m_editedValue.exists()) {
        return;
    }
    m_savedValue = m_editedValue;
}

QWidget* LegacyControllerFileSetting::buildInputWidget(QWidget* pParent) {
    auto* pWidget = new QWidget(pParent);
    pWidget->setLayout(new QHBoxLayout);
    auto* pPushButton = new QPushButton(tr("Browse..."), pWidget);
    auto* pLabel = new QLabel(pWidget);
    auto setLabelText = [pLabel](QString&& text) {
        pLabel->setText(QStringLiteral("<i>%1</i>").arg(text));
    };
    pWidget->layout()->addWidget(pLabel);
    pWidget->layout()->addWidget(pPushButton);

    connect(this, &AbstractLegacyControllerSetting::valueReset, pLabel, [this, setLabelText]() {
        if (m_editedValue.exists()) {
            setLabelText(m_editedValue.absoluteFilePath());
        } else {
            setLabelText(tr("No file selected"));
        }
    });

    connect(pPushButton,
            &QPushButton::clicked,
            this,
            [this, pLabel, pPushButton](bool) {
                auto file = QFileInfo(QFileDialog::getOpenFileName(pPushButton,
                        tr("Select a file"),
                        QString(),
                        m_fileFilter));
                if (file.exists()) {
                    m_editedValue = file;
                    pLabel->setText(QStringLiteral("<i>%1</i>").arg(file.absoluteFilePath()));
                    emit changed();
                }
            });

    if (m_savedValue.exists()) {
        setLabelText(m_savedValue.absoluteFilePath());
    } else {
        setLabelText(tr("No file selected"));
    }

    return pWidget;
}
