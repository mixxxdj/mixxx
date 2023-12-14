#include "preferences/dialog/dlgprefinterface.h"

#include <QDir>
#include <QList>
#include <QLocale>
#include <QScreen>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "defs_urls.h"
#include "moc_dlgprefinterface.cpp"
#include "preferences/usersettings.h"
#include "skin/legacy/legacyskinparser.h"
#include "skin/skin.h"
#include "skin/skinloader.h"
#include "util/cmdlineargs.h"
#include "util/screensaver.h"
#include "util/screensavermanager.h"
#include "util/widgethelper.h"

using mixxx::skin::SkinManifest;
using mixxx::skin::SkinPointer;

namespace {

const QString kConfigGroup = QStringLiteral("[Config]");
const QString kControlsGroup = QStringLiteral("[Controls]");
const QString kScaleFactorKey = QStringLiteral("ScaleFactor");
const QString kStartInFullscreenKey = QStringLiteral("StartInFullscreen");
const QString kSchemeKey = QStringLiteral("Scheme");
const QString kResizableSkinKey = QStringLiteral("ResizableSkin");
const QString kLocaleKey = QStringLiteral("Locale");
const QString kTooltipsKey = QStringLiteral("Tooltips");

} // namespace

DlgPrefInterface::DlgPrefInterface(
        QWidget* parent,
        std::shared_ptr<mixxx::ScreensaverManager> pScreensaverManager,
        std::shared_ptr<mixxx::skin::SkinLoader> pSkinLoader,
        UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_pConfig(pConfig),
          m_pScreensaverManager(pScreensaverManager),
          m_pSkinLoader(pSkinLoader),
          m_pSkin(pSkinLoader ? pSkinLoader->getConfiguredSkin() : nullptr),
          m_dScaleFactor(1.0),
          m_minScaleFactor(1.0),
          m_dDevicePixelRatio(1.0) {
    setupUi(this);

    // get the pixel ratio to display a crisp skin preview when Mixxx is scaled
    m_dDevicePixelRatio = devicePixelRatioF();

    // Calculate the minimum scale factor that leads to a device pixel ratio of 1.0
    // m_dDevicePixelRatio must not drop below 1.0 because this creates an
    // unusable GUI with visual artefacts
    double initialScaleFactor = CmdlineArgs::Instance().getScaleFactor();
    if (initialScaleFactor <= 0) {
        initialScaleFactor = 1.0;
    }
    double unscaledDevicePixelRatio = m_dDevicePixelRatio / initialScaleFactor;
    m_minScaleFactor = 1 / unscaledDevicePixelRatio;

    // Locale setting
    // Iterate through the available locales and add them to the combobox
    // Borrowed following snippet from http://qt-project.org/wiki/How_to_create_a_multi_language_application
    const auto translationsDir = QDir(
            m_pConfig->getResourcePath() +
            QStringLiteral("translations/"));
    if (!translationsDir.exists()) {
        qWarning() << "Translations directory does not exist" << translationsDir.absolutePath();
    }

    QStringList fileNames = translationsDir.entryList(QStringList("mixxx_*.qm"));
    // Add source language as a fake value
    DEBUG_ASSERT(!fileNames.contains(QStringLiteral("mixxx_en_US.qm")));
    fileNames.push_back(QStringLiteral("mixxx_en_US.qm"));

    for (const auto& fileName : std::as_const(fileNames)) {
        // Extract locale name from file name
        QString localeName = fileName;
        // Strip prefix
        DEBUG_ASSERT(localeName.startsWith(QStringLiteral("mixxx_")));
        localeName.remove(0, QStringLiteral("mixxx_").length());
        // Strip file extension
        localeName.truncate(localeName.lastIndexOf('.'));
        // Convert to QLocale name format. Unfortunately the translation files
        // use inconsistent language/territory separators, i.e. both '-' and '_'.
        auto localeNameFixed = localeName;
        localeNameFixed.replace('-', '_');
        const auto locale = QLocale(localeNameFixed);

        const QString languageName = QLocale::languageToString(locale.language());
        // Ugly hack to skip non-resolvable locales
        if (languageName == QStringLiteral("C")) {
            qWarning() << "Preferences: skipping unsupported locale" << localeNameFixed;
            continue;
        }
        QString territoryName;
        // Ugly hack to detect locales with an explicitly specified country/territory.
        // https://doc.qt.io/qt-5/qlocale.html#QLocale-1
        // "If country is not present, or is not a valid ISO 3166 code, the most
        // appropriate country is chosen for the specified language."
        if (localeNameFixed.contains('_')) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            territoryName = QLocale::countryToString(locale.country());
#else
            territoryName = QLocale::territoryToString(locale.territory());
#endif
            DEBUG_ASSERT(!territoryName.isEmpty());
        }
        QString displayName = languageName;
        if (!territoryName.isEmpty()) {
            displayName += QStringLiteral(" (") + territoryName + ')';
        }
        // The locale name is stored in the config
        ComboBoxLocale->addItem(displayName, localeName);
    }
    // Sort languages list...
    ComboBoxLocale->model()->sort(0);
    // ...and then insert entry for default system locale at the top
    ComboBoxLocale->insertItem(0, QStringLiteral("System"), "");
    ComboBoxLocale->insertSeparator(1);

    if (pSkinLoader) {
        // Skin configurations
        QString sizeWarningString =
                "<img src=\":/images/preferences/ic_preferences_warning.svg\") "
                "width=16 height=16 />   " +
                tr("The minimum size of the selected skin is bigger than your "
                   "screen resolution.");
        warningLabel->setText(sizeWarningString);

        ComboBoxSkinconf->clear();
        skinPreviewLabel->setText("");
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
        slotSetSkinPreview();
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
    } else {
        groupBoxSkinOptions->hide();
    }

    // Screensaver mode
    comboBoxScreensaver->clear();
    comboBoxScreensaver->addItem(tr("Allow screensaver to run"),
            static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_OFF));
    comboBoxScreensaver->addItem(tr("Prevent screensaver from running"),
            static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON));
    comboBoxScreensaver->addItem(tr("Prevent screensaver while playing"),
            static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON_PLAY));

    int inhibitsettings = static_cast<int>(m_pScreensaverManager->status());
    comboBoxScreensaver->setCurrentIndex(comboBoxScreensaver->findData(inhibitsettings));

    // Tooltip configuration
    connect(buttonGroupTooltips,
            QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this,
            [this](QAbstractButton* button) {
                Q_UNUSED(button);
                slotSetTooltips();
            });

    setScrollSafeGuardForAllInputWidgets(this);

    slotUpdate();
}

QScreen* DlgPrefInterface::getScreen() const {
    auto* pScreen =
            mixxx::widgethelper::getScreen(*this);
    if (!pScreen) {
        // Obtain the primary screen. This is necessary if no window is
        // available before the widget is displayed.
        pScreen = qGuiApp->primaryScreen();
    }
    DEBUG_ASSERT(pScreen);
    return pScreen;
}

void DlgPrefInterface::slotUpdateSchemes() {
    if (!m_pSkinLoader) {
        return;
    }

    // Re-populates the scheme combobox and attempts to pick the color scheme from config file.
    // Since this involves opening a file we won't do this as part of regular slotUpdate
    const QList<QString> schlist = m_pSkin->colorschemes();

    ComboBoxSchemeconf->clear();
    m_colorSchemeOnUpdate = QString();

    if (schlist.size() == 0) {
        ComboBoxSchemeconf->setEnabled(false);
        ComboBoxSchemeconf->addItem(tr("This skin does not support color schemes", nullptr));
        ComboBoxSchemeconf->setCurrentIndex(0);
        // clear m_colorScheme so that the correct skin preview is loaded
        m_colorScheme = QString();
    } else {
        ComboBoxSchemeconf->setEnabled(true);
        QString configScheme = m_pConfig->getValue(ConfigKey(kConfigGroup, kSchemeKey));
        bool foundConfigScheme = false;
        for (int i = 0; i < schlist.size(); i++) {
            ComboBoxSchemeconf->addItem(schlist[i]);

            if (schlist[i] == configScheme) {
                ComboBoxSchemeconf->setCurrentIndex(i);
                m_colorScheme = configScheme;
                m_colorSchemeOnUpdate = configScheme;
                foundConfigScheme = true;
            }
        }
        // There might be a skin configured that has color schemes but none of them
        // matches the configured color scheme.
        // The combobox would pick the first item then. Also choose this item for
        // m_colorScheme to avoid an empty skin preview.
        if (!foundConfigScheme) {
            m_colorScheme = schlist[0];
            m_colorSchemeOnUpdate = schlist[0];
        }
    }
}

void DlgPrefInterface::slotUpdate() {
    if (m_pSkinLoader) {
        const SkinPointer pSkinOnUpdate = m_pSkinLoader->getConfiguredSkin();
        if (pSkinOnUpdate != nullptr && pSkinOnUpdate->isValid()) {
            m_skinNameOnUpdate = pSkinOnUpdate->name();
        } else {
            m_skinNameOnUpdate = m_pSkinLoader->getDefaultSkinName();
        }
        ComboBoxSkinconf->setCurrentIndex(ComboBoxSkinconf->findText(m_skinNameOnUpdate));
        slotUpdateSchemes();
    }

    m_localeOnUpdate = m_pConfig->getValue(ConfigKey(kConfigGroup, kLocaleKey));
    ComboBoxLocale->setCurrentIndex(ComboBoxLocale->findData(m_localeOnUpdate));

    // The spinbox shows a percentage but Mixxx stores a multiplication factor
    // with 1.00 as no scaling, so multiply the stored value by 100.
    m_dScaleFactor = m_pConfig->getValue(
            ConfigKey(kConfigGroup, kScaleFactorKey), m_dScaleFactor);
    spinBoxScaleFactor->setValue(m_dScaleFactor * 100);
    spinBoxScaleFactor->setMinimum(m_minScaleFactor * 100);

    checkBoxStartFullScreen->setChecked(m_pConfig->getValue(
            ConfigKey(kConfigGroup, kStartInFullscreenKey), false));

    loadTooltipPreferenceFromConfig();

    int inhibitsettings = static_cast<int>(m_pScreensaverManager->status());
    comboBoxScreensaver->setCurrentIndex(comboBoxScreensaver->findData(inhibitsettings));
}

void DlgPrefInterface::slotResetToDefaults() {
    if (m_pSkinLoader) {
        int index = ComboBoxSkinconf->findText(m_pSkinLoader->getDefaultSkinName());
        ComboBoxSkinconf->setCurrentIndex(index);
        slotSetSkin(index);
    }

    // Use System locale
    ComboBoxLocale->setCurrentIndex(0);

    // Default to normal size widgets
    // The spinbox shows a percentage with 100% as no scaling.
    spinBoxScaleFactor->setValue(100);

    // Don't start in full screen.
    checkBoxStartFullScreen->setChecked(false);

    // Inhibit the screensaver
    comboBoxScreensaver->setCurrentIndex(comboBoxScreensaver->findData(
        static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON)));

    // Tooltips on everywhere.
    radioButtonTooltipsLibraryAndSkin->setChecked(true);
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
    QMessageBox::information(this,
            tr("Information"),
            tr("Mixxx must be restarted before the new locale or scaling "
               "settings will take effect."));
}

void DlgPrefInterface::slotSetScheme(int) {
    // This slot can be triggered by opening the preferences. If the current
    // skin does not support color schemes, this would treat the string in the
    // combobox as color scheme name. Therefore we need to check if the
    // checkbox is actually enabled.
    QString newScheme = ComboBoxSchemeconf->isEnabled()
            ? ComboBoxSchemeconf->currentText()
            : QString();

    m_colorScheme = newScheme;
    slotSetSkinPreview();
}

void DlgPrefInterface::slotSetSkinDescription() {
    if (!m_pSkinLoader) {
        return;
    }

    const QString description = m_pSkin->description();
    if (!description.isEmpty()) {
        skinDescriptionText->show();
        skinDescriptionText->setText(description);
    } else {
        skinDescriptionText->hide();
    }
}

void DlgPrefInterface::slotSetSkinPreview() {
    if (!m_pSkinLoader) {
        return;
    }

    QPixmap preview = m_pSkin->preview(m_colorScheme);
    preview.setDevicePixelRatio(m_dDevicePixelRatio);
    skinPreviewLabel->setPixmap(preview.scaled(
            QSize(640, 360) * m_dDevicePixelRatio,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
}

void DlgPrefInterface::slotSetSkin(int) {
    if (!m_pSkinLoader) {
        return;
    }

    QString newSkinName = ComboBoxSkinconf->currentText();
    if (newSkinName == m_pSkin->name()) {
        return;
    }

    const SkinPointer pNewSkin = m_skins[newSkinName];
    VERIFY_OR_DEBUG_ASSERT(pNewSkin != nullptr && pNewSkin->isValid()) {
        return;
    }
    m_pSkin = pNewSkin;
    const auto* const pScreen = getScreen();
    if (pScreen && m_pSkin->fitsScreenSize(*pScreen)) {
        warningLabel->hide();
    } else {
        warningLabel->show();
    }
    slotUpdateSchemes();
    slotSetSkinDescription();
    slotSetSkinPreview();
}

void DlgPrefInterface::slotApply() {
    if (m_pSkinLoader) {
        m_pConfig->set(ConfigKey(kConfigGroup, kResizableSkinKey), m_pSkin->name());
        m_pConfig->set(ConfigKey(kConfigGroup, kSchemeKey), m_colorScheme);
    }

    QString locale = ComboBoxLocale->itemData(
            ComboBoxLocale->currentIndex()).toString();
    m_pConfig->set(ConfigKey(kConfigGroup, kLocaleKey), locale);

    double scaleFactor = spinBoxScaleFactor->value() / 100;
    m_pConfig->setValue(ConfigKey(kConfigGroup, kScaleFactorKey), scaleFactor);

    m_pConfig->set(ConfigKey(kConfigGroup, kStartInFullscreenKey),
            ConfigValue(checkBoxStartFullScreen->isChecked()));

    m_pConfig->set(ConfigKey(kControlsGroup, kTooltipsKey),
            ConfigValue(static_cast<int>(m_tooltipMode)));
    emit tooltipModeChanged(m_tooltipMode);

    // screensaver mode update
    int screensaverComboBoxState = comboBoxScreensaver->itemData(
            comboBoxScreensaver->currentIndex()).toInt();
    int screensaverConfiguredState = static_cast<int>(m_pScreensaverManager->status());
    if (screensaverComboBoxState != screensaverConfiguredState) {
        m_pScreensaverManager->setStatus(
                static_cast<mixxx::ScreenSaverPreference>(screensaverComboBoxState));
    }

    if (locale != m_localeOnUpdate || scaleFactor != m_dScaleFactor) {
        notifyRebootNecessary();
        // hack to prevent showing the notification when pressing "Okay" after "Apply"
        m_localeOnUpdate = locale;
        m_dScaleFactor = scaleFactor;
    }

    // load skin/scheme if necessary
    if (m_pSkin &&
            (m_pSkin->name() != m_skinNameOnUpdate ||
                    m_colorScheme != m_colorSchemeOnUpdate)) {
        // ColorSchemeParser::setupLegacyColorSchemes() reads scheme from config
        emit reloadUserInterface();
        // Allow switching skins multiple times without closing the dialog
        m_skinNameOnUpdate = m_pSkin->name();
    }
}

void DlgPrefInterface::loadTooltipPreferenceFromConfig() {
    const auto tooltipMode = static_cast<mixxx::TooltipsPreference>(
            m_pConfig->getValue(ConfigKey(kControlsGroup, kTooltipsKey),
                    static_cast<int>(mixxx::TooltipsPreference::TOOLTIPS_ON)));
    switch (tooltipMode) {
    case mixxx::TooltipsPreference::TOOLTIPS_OFF:
        radioButtonTooltipsOff->setChecked(true);
        break;
    case mixxx::TooltipsPreference::TOOLTIPS_ONLY_IN_LIBRARY:
        radioButtonTooltipsLibrary->setChecked(true);
        break;
    case mixxx::TooltipsPreference::TOOLTIPS_ON:
    default:
        radioButtonTooltipsLibraryAndSkin->setChecked(true);
        break;
    }
    m_tooltipMode = tooltipMode;
}
