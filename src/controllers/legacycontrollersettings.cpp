#include "controllers/legacycontrollersettings.h"

#include "moc_legacycontrollersettings.cpp"

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::__self = nullptr;

LegacyControllerSettingBuilder* LegacyControllerSettingBuilder::instance() {
    if (__self == nullptr)
        __self = new LegacyControllerSettingBuilder();

    return __self;
}

QWidget* LegacyControllerIntegerSetting::buildWidget(QWidget* parent) {
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

    QSpinBox* spinBox = new QSpinBox(root);
    spinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    root->layout()->addWidget(spinBox);

    spinBox->setRange(this->m_minValue, this->m_maxValue);
    spinBox->setSingleStep(this->m_stepValue);
    spinBox->setValue(this->m_currentValue);

    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] {
        emit changed();
    });

    static_cast<QHBoxLayout*>(root->layout())->setStretch(0, 3);
    static_cast<QHBoxLayout*>(root->layout())->setStretch(1, 1);
    return root;
}

AbstractLegacyControllerSetting* LegacyControllerIntegerSetting::createFrom(
        const QDomElement& element) {
    int defaultValue, minValue, maxValue, stepValue;

    bool isOk = false;
    minValue = element.attribute("min").toInt(&isOk);
    if (!isOk) {
        minValue = std::numeric_limits<int>::min();
    }
    maxValue = element.attribute("max").toInt(&isOk);
    if (!isOk) {
        maxValue = std::numeric_limits<int>::max();
    }
    stepValue = element.attribute("step").toInt(&isOk);
    if (!isOk) {
        stepValue = 1;
    }
    defaultValue = element.attribute("default").toInt(&isOk);
    if (!isOk) {
        defaultValue = 0;
    }

    return new LegacyControllerIntegerSetting(
            element, defaultValue, defaultValue, minValue, maxValue, stepValue);
}

bool LegacyControllerIntegerSetting::match(const QDomElement& element) {
    return element.hasAttribute("type") &&
            QString::compare(element.attribute("type"),
                    "integer",
                    Qt::CaseInsensitive) == 0;
};

REGISTER_LEGACY_CONTROLLER_SETTING(LegacyControllerIntegerSetting);
