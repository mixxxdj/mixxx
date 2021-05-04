#pragma once

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QString>
#include <QTranslator>
#include <QtDebug>

#include "preferences/usersettings.h"

namespace mixxx {

class Translations {
  public:
    static void initializeTranslations(UserSettingsPointer pConfig,
            QCoreApplication* pApp,
            const QString& cmdlineLocaleString) {
        QString translationsPath = pConfig->getResourcePath() + "translations/";

        // Set the default locale, if specified via command line or config
        // file. If neither is the case, the default locale will be the
        // system's locale.
        {
            QString customLocaleString = QString();
            if (!cmdlineLocaleString.isEmpty()) {
                customLocaleString = cmdlineLocaleString;
            } else {
                const QString configLocale = pConfig->getValueString(
                        ConfigKey("[Config]", "Locale"));
                if (!configLocale.isEmpty()) {
                    customLocaleString = configLocale;
                }
            }

            if (!customLocaleString.isEmpty()) {
                const QLocale customLocale = QLocale(customLocaleString);
                // If the customLocaleString is not a valid locale, Qt will
                // automatically use the "C" locale instead. In that case,
                // let's print a warning.
                if (customLocaleString.compare(
                            QStringLiteral("C"), Qt::CaseInsensitive) != 0 &&
                        customLocale.language() == QLocale::C) {
                    qWarning() << "Custom locale not found, using 'C' locale "
                                  "(check your configuration to avoid this "
                                  "warning).";
                }
                QLocale::setDefault(customLocale);
            }
        }

        QLocale locale = QLocale();

        // Do not try to load translations if we're using 'C' locale.
        if (locale.language() == QLocale::C) {
            qDebug() << "Skipping loading of translations because the 'C' locale is used.";
            return;
        }

        // Don't bother loading a translation if the first ui-langauge is
        // English because the interface is already in English. This fixes
        // the case where the user's install of Qt doesn't have an explicit
        // English translation file and the fact that we don't ship a
        // mixxx_en.qm.
        if (locale.language() == QLocale::English &&
                (locale.country() == QLocale::UnitedStates ||
                        locale.country() == QLocale::AnyCountry)) {
            qDebug() << "Skipping loading of translations because the locale is 'en' or 'en_US'.";
            return;
        }

        // Load Qt translations for this locale from the system translation
        // path. This is the lowest precedence QTranslator.
        QTranslator* pQtTranslator = new QTranslator(pApp);
        if (pQtTranslator->load(locale,
                    "qt",
                    "_",
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
            pApp->installTranslator(pQtTranslator);
        } else {
            delete pQtTranslator;
        }

        // Load Qt translations for this locale from the Mixxx translations
        // folder.
        QTranslator* pMixxxQtTranslator = new QTranslator(pApp);
        if (pMixxxQtTranslator->load(locale, "qt", "_", translationsPath)) {
            pApp->installTranslator(pMixxxQtTranslator);
        } else {
            delete pMixxxQtTranslator;
        }

        // Load Mixxx specific translations for this locale from the Mixxx
        // translations folder.
        QTranslator* pMixxxTranslator = new QTranslator(pApp);
        bool mixxxLoaded = pMixxxTranslator->load(locale, "mixxx", "_", translationsPath);
        if (mixxxLoaded) {
            qDebug() << "Loaded translations for locale"
                     << locale.name()
                     << "from" << translationsPath;
            pApp->installTranslator(pMixxxTranslator);
        } else {
            qWarning() << "Failed to load translations for locale"
                       << locale.name()
                       << "from" << translationsPath;
            delete pMixxxTranslator;
        }
    }
    Translations() = delete;
};

}  // namespace mixxx
