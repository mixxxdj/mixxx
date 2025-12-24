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
        const QString translationsPath =
                pConfig->getResourcePath() + QStringLiteral("translations/");

        // Set the default locale, if specified via command line or config
        // file. If neither is the case, the default locale will be the
        // system's locale.
        QString customLocaleString;
        if (!cmdlineLocaleString.isEmpty()) {
            customLocaleString = cmdlineLocaleString;
        } else {
            customLocaleString = pConfig->getValue(
                    ConfigKey("[Config]", "Locale"), QString());
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
#if defined(__APPLE__) && (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
        else {
            // Workaround https://github.com/mixxxdj/mixxx/issues/11195 and
            // QTBUG-90971 fixed in Qt 6.2
            const auto sysLocale = QLocale::system();
            QLocale::setDefault(QLocale(sysLocale.language(), sysLocale.country()));
        }
#endif

        // Constructs a QLocale object initialized with the default locale. If
        // no default locale was set using setDefault(), this locale will be
        // the same as the one returned by system().
        QLocale locale;

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                (locale.territory() == QLocale::UnitedStates ||
                        locale.territory() == QLocale::AnyCountry)
#else
                (locale.country() == QLocale::UnitedStates ||
                        locale.country() == QLocale::AnyCountry)
#endif
        ) {
            qDebug() << "Skipping loading of translations because the locale is 'en' or 'en_US'.";
            return;
        }
        // Load Qt translations for this locale from the system translation
        // path. This is the lowest precedence QTranslator.
        bool qtFound = installTranslations(pApp,
                locale,
                QStringLiteral("qt"),
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QLibraryInfo::path(QLibraryInfo::TranslationsPath),
#else
                QLibraryInfo::location(QLibraryInfo::TranslationsPath),
#endif
                false);

        if (!qtFound) {
            // Alternative, load Qt translations for this locale from the Mixxx translations
            // folder.
            installTranslations(pApp, locale, QStringLiteral("qt"), translationsPath, true);
        }

        // Load Mixxx specific translations for this locale from the Mixxx
        // translations folder.
        installTranslations(pApp, locale, QStringLiteral("mixxx"), translationsPath, true);
    }
    Translations() = delete;

  private:
    static bool installTranslations(QCoreApplication* pApp,
            const QLocale& locale,
            const QString& translation,
            const QString& translationsPath,
            bool warnOnFailure) {
        QTranslator* pTranslator = new QTranslator(pApp);
        const bool success = pTranslator->load(
                locale, translation, QStringLiteral("_"), translationsPath);
        if (!success) {
            if (warnOnFailure) {
                qWarning()
                        << "Failed to load" << translation << "translations for locale"
                        << locale.name()
                        << "from" << translationsPath;
            }
            delete pTranslator;
            return false;
        }

        qDebug() << "Loaded" << translation << "translations for locale"
                 << locale.name()
                 << "from" << pTranslator->filePath();
        pApp->installTranslator(pTranslator);
        return true;
    }
};

}  // namespace mixxx
