#include "preferences/dialog/dlgprefinterface.h"

#include <QDir>
#include <QDoubleSpinBox>
#include <QList>
#include <QLocale>
#include <QScreen>
#include <QToolTip>
#include <QWidget>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "mixxx.h"
#include "moc_dlgprefinterface.cpp"
#include "preferences/usersettings.h"
#include "skin/legacy/legacyskinparser.h"
#include "skin/skin.h"
#include "skin/skinloader.h"
#include "util/screensaver.h"
#include "util/widgethelper.h"

using mixxx::skin::SkinManifest;
using mixxx::skin::SkinPointer;

DlgPrefInterface::DlgPrefInterface(
        QWidget* parent,
        MixxxMainWindow* mixxx,
        std::shared_ptr<mixxx::skin::SkinLoader> pSkinLoader,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_mixxx(mixxx),
          m_pSkinLoader(pSkinLoader),
          m_pSkin(pSkinLoader->getConfiguredSkin()),
          m_dScaleFactorAuto(1.0),
          m_bUseAutoScaleFactor(false),
          m_dScaleFactor(1.0),
          m_bStartWithFullScreen(false),
          m_bRebootMixxxView(false) {
    setupUi(this);

    VERIFY_OR_DEBUG_ASSERT(m_pSkin != nullptr) {
        qWarning() << "Skipping creation of DlgPrefInterface because there is no skin available.";
        return;
    }

    // Locale setting
    // Iterate through the available locales and add them to the combobox
    // Borrowed following snippet from http://qt-project.org/wiki/How_to_create_a_multi_language_application
    const auto translationsDir = QDir(
            m_pConfig->getResourcePath() +
            QStringLiteral("translations/"));
    DEBUG_ASSERT(translationsDir.exists());

    QStringList fileNames = translationsDir.entryList(QStringList("mixxx_*.qm"));
    // Add source language as a fake value
    DEBUG_ASSERT(!fileNames.contains(QStringLiteral("mixxx_en_US.qm")));
    fileNames.push_back(QStringLiteral("mixxx_en_US.qm"));

    for (const auto& fileName : qAsConst(fileNames)) {
        // Extract locale name from file name
        QString localeName = fileName;
        // Strip prefix
        DEBUG_ASSERT(localeName.startsWith(QStringLiteral("mixxx_")));
        localeName.remove(0, QStringLiteral("mixxx_").length());
        // Strip file extension
        localeName.truncate(localeName.lastIndexOf('.'));
        // Convert to QLocale name format. Unfortunately the translation files
        // use inconsistent language/country separators, i.e. both '-' and '_'.
        auto localeNameFixed = localeName;
        localeNameFixed.replace('-', '_');
        const auto locale = QLocale(localeNameFixed);

        const QString languageName = QLocale::languageToString(locale.language());
        // Ugly hack to skip non-resolvable locales
        if (languageName == QStringLiteral("C")) {
            qWarning() << "Unsupported locale" << localeNameFixed;
            continue;
        }
        QString countryName;
        // Ugly hack to detect locales with an explicitly specified country.
        // https://doc.qt.io/qt-5/qlocale.html#QLocale-1
        // "If country is not present, or is not a valid ISO 3166 code, the most
        // appropriate country is chosen for the specified language."
        if (localeNameFixed.contains('_')) {
            countryName = QLocale::countryToString(locale.country());
            DEBUG_ASSERT(!countryName.isEmpty());
        }
        QString displayName = languageName;
        if (!countryName.isEmpty()) {
            displayName += QStringLiteral(" (") + countryName + ')';
        }
        // The locale name is stored in the config
        ComboBoxLocale->addItem(displayName, localeName);
    }
    // Sort languages list...
    ComboBoxLocale->model()->sort(0);
    // ...and then insert entry for default system locale at the top
    ComboBoxLocale->insertItem(0, QStringLiteral("System"), "");

    // Skin configurations
    QString sizeWarningString =
            "<img src=\":/images/preferences/ic_preferences_warning.svg\") "
            "width=16 height=16 />   " +
            tr("The minimum size of the selected skin is bigger than your "
               "screen resolution.");
    warningLabel->setText(sizeWarningString);

    ComboBoxSkinconf->clear();
    // align left edge of preview image and skin description with comboboxes
    skinPreviewLabel->setStyleSheet("QLabel { margin-left: 4px; }");
    skinPreviewLabel->setText("");
    skinDescriptionText->setStyleSheet("QLabel { margin-left: 2px; }");
    skinDescriptionText->setText("");
    skinDescriptionText->hide();

    const QList<SkinPointer> skins = m_pSkinLoader->getSkins();
    int index = 0;
    for (const SkinPointer& pSkin : skins) {
        ComboBoxSkinconf->insertItem(index, pSkin->name());
        m_skins.insert(pSkin->name(), pSkin);
        index++;
    }

    ComboBoxSkinconf->setCurrentIndex(index);
    // schemes must be updated here to populate the drop-down box and set m_colorScheme
    slotUpdateSchemes();
    skinPreviewLabel->setPixmap(m_pSkin->preview(m_colorScheme));
    const auto* const pScreen = getScreen();
    if (m_pSkin->fitsScreenSize(*pScreen)) {
        warningLabel->hide();
    } else {
        warningLabel->show();
    }
    slotSetSkinDescription();

    connect(ComboBoxSkinconf,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefInterface::slotSetSkin);
    connect(ComboBoxSchemeconf,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefInterface::slotSetScheme);

    checkBoxScaleFactorAuto->hide();
    spinBoxScaleFactor->hide();
    labelScaleFactor->hide();

    // Start in fullscreen mode
    checkBoxStartFullScreen->setChecked(m_pConfig->getValueString(
                    ConfigKey("[Config]", "StartInFullscreen")).toInt()==1);

    // Screensaver mode
    comboBoxScreensaver->clear();
    comboBoxScreensaver->addItem(tr("Allow screensaver to run"),
            static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_OFF));
    comboBoxScreensaver->addItem(tr("Prevent screensaver from running"),
            static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON));
    comboBoxScreensaver->addItem(tr("Prevent screensaver while playing"),
            static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON_PLAY));

    int inhibitsettings = static_cast<int>(mixxx->getInhibitScreensaver());
    comboBoxScreensaver->setCurrentIndex(comboBoxScreensaver->findData(inhibitsettings));

    // Tooltip configuration
    // Initialize checkboxes to match config
    loadTooltipPreferenceFromConfig();
    slotSetTooltips();  // Update disabled status of "only library" checkbox
    connect(buttonGroupTooltips,
            QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this,
            [this](QAbstractButton* button) {
                Q_UNUSED(button);
                slotSetTooltips();
            });

    slotUpdate();
}

QScreen* DlgPrefInterface::getScreen() const {
    auto* pScreen =
            mixxx::widgethelper::getScreen(*this);
    if (!pScreen) {
        // Obtain the screen from the main widget as a fallback. This
        // is necessary if no window is available before the widget
        // is displayed.
        pScreen = mixxx::widgethelper::getScreen(*m_mixxx);
    }
    DEBUG_ASSERT(pScreen);
    return pScreen;
}

void DlgPrefInterface::slotUpdateSchemes() {
    // Re-populates the scheme combobox and attempts to pick the color scheme from config file.
    // Since this involves opening a file we won't do this as part of regular slotUpdate
    const QList<QString> schlist = m_pSkin->colorschemes();

    ComboBoxSchemeconf->clear();

    if (schlist.size() == 0) {
        ComboBoxSchemeconf->setEnabled(false);
        ComboBoxSchemeconf->addItem(tr("This skin does not support color schemes", nullptr));
        ComboBoxSchemeconf->setCurrentIndex(0);
        // clear m_colorScheme so that the correct skin preview is loaded
        m_colorScheme = QString();
    } else {
        ComboBoxSchemeconf->setEnabled(true);
        QString configScheme = m_pConfig->getValueString(ConfigKey("[Config]", "Scheme"));
        bool foundConfigScheme = false;
        for (int i = 0; i < schlist.size(); i++) {
            ComboBoxSchemeconf->addItem(schlist[i]);

            if (schlist[i] == configScheme) {
                ComboBoxSchemeconf->setCurrentIndex(i);
                m_colorScheme = configScheme;
                foundConfigScheme = true;
            }
        }
        // There might be a skin configured that has color schemes but none of them
        // matches the configured color scheme.
        // The combobox would pick the first item then. Also choose this item for
        // m_colorScheme to avoid an empty skin preview.
        if (!foundConfigScheme) {
            m_colorScheme = schlist[0];
        }
    }
}

void DlgPrefInterface::slotUpdate() {
    const QString skinNameOnUpdate =
            m_pConfig->getValueString(ConfigKey("[Config]", "ResizableSkin"));
    const SkinPointer pSkinOnUpdate = m_skins[skinNameOnUpdate];
    if (pSkinOnUpdate != nullptr && pSkinOnUpdate->isValid()) {
        m_skinNameOnUpdate = pSkinOnUpdate->name();
    } else {
        m_skinNameOnUpdate = m_pSkinLoader->getDefaultSkinName();
    }
    ComboBoxSkinconf->setCurrentIndex(ComboBoxSkinconf->findText(m_skinNameOnUpdate));
    slotUpdateSchemes();
    m_bRebootMixxxView = false;

    m_localeOnUpdate = m_pConfig->getValueString(ConfigKey("[Config]", "Locale"));
    ComboBoxLocale->setCurrentIndex(ComboBoxLocale->findData(m_localeOnUpdate));

    checkBoxScaleFactorAuto->setChecked(m_pConfig->getValue(
            ConfigKey("[Config]", "ScaleFactorAuto"), m_bUseAutoScaleFactor));

    // The spinbox shows a percentage but Mixxx stores a multiplication factor
    // with 1.00 as no scaling, so multiply the stored value by 100.
    spinBoxScaleFactor->setValue(m_pConfig->getValue(
                    ConfigKey("[Config]", "ScaleFactor"), m_dScaleFactor) * 100);

    checkBoxStartFullScreen->setChecked(m_pConfig->getValue(
            ConfigKey("[Config]", "StartInFullscreen"), m_bStartWithFullScreen));

    loadTooltipPreferenceFromConfig();

    int inhibitsettings = static_cast<int>(m_mixxx->getInhibitScreensaver());
    comboBoxScreensaver->setCurrentIndex(comboBoxScreensaver->findData(inhibitsettings));
}

void DlgPrefInterface::slotResetToDefaults() {
    int index = ComboBoxSkinconf->findText(m_pSkinLoader->getDefaultSkinName());
    ComboBoxSkinconf->setCurrentIndex(index);
    slotSetSkin(index);

    // Use System locale
    ComboBoxLocale->setCurrentIndex(0);

    // Default to normal size widgets
    // The spinbox shows a percentage with 100% as no scaling.
    spinBoxScaleFactor->setValue(100);
    if (m_dScaleFactorAuto > 0) {
        checkBoxScaleFactorAuto->setChecked(true);
    }

    // Don't start in full screen.
    checkBoxStartFullScreen->setChecked(false);

    // Inhibit the screensaver
    comboBoxScreensaver->setCurrentIndex(comboBoxScreensaver->findData(
        static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON)));

    // Tooltips on everywhere.
    radioButtonTooltipsLibraryAndSkin->setChecked(true);
}

void DlgPrefInterface::slotSetScaleFactor(double newValue) {
    // The spinbox shows a percentage, but Mixxx stores a multiplication factor
    // with 1.00 as no change.
    newValue /= 100.0;
    if (m_dScaleFactor != newValue) {
        m_dScaleFactor = newValue;
        m_bRebootMixxxView = true;
    }
}

void DlgPrefInterface::slotSetScaleFactorAuto(bool newValue) {
    if (newValue) {
        if (!m_bUseAutoScaleFactor) {
            m_bRebootMixxxView = true;
        }
    } else {
        slotSetScaleFactor(newValue);
    }

    m_bUseAutoScaleFactor = newValue;
    spinBoxScaleFactor->setEnabled(!newValue);
}

void DlgPrefInterface::slotSetTooltips() {
    m_tooltipMode = mixxx::TooltipsPreference::TOOLTIPS_ON;
    if (radioButtonTooltipsOff->isChecked()) {
        m_tooltipMode = mixxx::TooltipsPreference::TOOLTIPS_OFF;
    } else if (radioButtonTooltipsLibrary->isChecked()) {
        m_tooltipMode = mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY;
    }
}

void DlgPrefInterface::notifyRebootNecessary() {
    // make the fact that you have to restart mixxx more obvious
    QMessageBox::information(
        this, tr("Information"),
        tr("Mixxx must be restarted before the new locale setting will take effect."));
}

void DlgPrefInterface::slotSetScheme(int) {
    // This slot can be triggered by opening the preferences. If the current
    // skin does not support color schemes, this would treat the string in the
    // combobox as color scheme name. Therefore we need to check if the
    // checkbox is actually enabled.
    QString newScheme = ComboBoxSchemeconf->isEnabled()
            ? ComboBoxSchemeconf->currentText()
            : QString();

    if (m_colorScheme != newScheme) {
        m_colorScheme = newScheme;
        m_bRebootMixxxView = true;
    }
    QPixmap preview = m_pSkin->preview(m_colorScheme);
    skinPreviewLabel->setPixmap(preview.scaled(
            QSize(640, 360), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void DlgPrefInterface::slotSetSkinDescription() {
    const QString description = m_pSkin->description();
    if (!description.isEmpty()) {
        skinDescriptionText->show();
        skinDescriptionText->setText(description);
    } else {
        skinDescriptionText->hide();
    }
}

void DlgPrefInterface::slotSetSkin(int) {
    QString newSkinName = ComboBoxSkinconf->currentText();
    if (newSkinName == m_pSkin->name()) {
        return;
    }

    const SkinPointer pNewSkin = m_skins[newSkinName];
    VERIFY_OR_DEBUG_ASSERT(pNewSkin != nullptr && pNewSkin->isValid()) {
        return;
    }
    m_pSkin = pNewSkin;
    m_bRebootMixxxView = newSkinName != m_skinNameOnUpdate;
    const auto* const pScreen = getScreen();
    if (pScreen && m_pSkin->fitsScreenSize(*pScreen)) {
        warningLabel->hide();
    } else {
        warningLabel->show();
    }
    slotUpdateSchemes();
    slotSetSkinDescription();
    QPixmap preview = m_pSkin->preview(m_colorScheme);
    skinPreviewLabel->setPixmap(preview.scaled(
            QSize(640, 360), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void DlgPrefInterface::slotApply() {
    m_pConfig->set(ConfigKey("[Config]", "ResizableSkin"), m_pSkin->name());
    m_pConfig->set(ConfigKey("[Config]", "Scheme"), m_colorScheme);

    QString locale = ComboBoxLocale->itemData(
            ComboBoxLocale->currentIndex()).toString();
    m_pConfig->set(ConfigKey("[Config]", "Locale"), locale);

    m_pConfig->setValue(
            ConfigKey("[Config]", "ScaleFactorAuto"), m_bUseAutoScaleFactor);
    if (m_bUseAutoScaleFactor) {
        m_pConfig->setValue(
                ConfigKey("[Config]", "ScaleFactor"), m_dScaleFactorAuto);
    } else {
        m_pConfig->setValue(ConfigKey("[Config]", "ScaleFactor"), m_dScaleFactor);
    }

    m_pConfig->set(ConfigKey("[Config]", "StartInFullscreen"),
            ConfigValue(checkBoxStartFullScreen->isChecked()));

    m_mixxx->setToolTipsCfg(m_tooltipMode);

    // screensaver mode update
    int screensaverComboBoxState = comboBoxScreensaver->itemData(
            comboBoxScreensaver->currentIndex()).toInt();
    int screensaverConfiguredState = static_cast<int>(m_mixxx->getInhibitScreensaver());
    if (screensaverComboBoxState != screensaverConfiguredState) {
        m_mixxx->setInhibitScreensaver(
                static_cast<mixxx::ScreenSaverPreference>(screensaverComboBoxState));
    }

    if (locale != m_localeOnUpdate) {
        notifyRebootNecessary();
        // hack to prevent showing the notification when pressing "Okay" after "Apply"
        m_localeOnUpdate = locale;
    }

    if (m_bRebootMixxxView) {
        m_mixxx->rebootMixxxView();
        // Allow switching skins multiple times without closing the dialog
        m_skinNameOnUpdate = m_pSkin->name();
    }
    m_bRebootMixxxView = false;
}

void DlgPrefInterface::loadTooltipPreferenceFromConfig() {
    mixxx::TooltipsPreference configTooltips = m_mixxx->getToolTipsCfg();
    switch (configTooltips) {
        case mixxx::TooltipsPreference::TOOLTIPS_OFF:
            radioButtonTooltipsOff->setChecked(true);
            break;
        case mixxx::TooltipsPreference::TOOLTIPS_ON:
            radioButtonTooltipsLibraryAndSkin->setChecked(true);
            break;
        case mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY:
            radioButtonTooltipsLibrary->setChecked(true);
            break;
    }
    m_tooltipMode = configTooltips;
}
