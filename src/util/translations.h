#pragma once

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QString>
#include <QtDebug>

#include "preferences/usersettings.h"

namespace mixxx {

class Translations {
  public:
    static void initializeTranslations(UserSettingsPointer pConfig,
                                       QCoreApplication* pApp,
                                       const QString& forcedLocale) {
        QString resourcePath = pConfig->getResourcePath();
        QString translationsFolder = resourcePath + "translations/";

        // Load Qt base translations
        QString userLocale = forcedLocale;
        QLocale systemLocale = QLocale::system();

        // Attempt to load user locale from config
        if (userLocale.isEmpty()) {
            userLocale = pConfig->getValueString(ConfigKey("[Config]", "Locale"));
        }

        if (userLocale.isEmpty()) {
            QLocale::setDefault(QLocale(systemLocale));
        } else {
            QLocale::setDefault(QLocale(userLocale));
        }

        // source language
        if (userLocale == "en_US") {
            return;
        }

        // Load Qt translations for this locale from the system translation
        // path. This is the lowest precedence QTranslator.
        QTranslator* qtTranslator = new QTranslator(pApp);
        if (loadTranslations(systemLocale, userLocale, "qt", "_",
                             QLibraryInfo::location(QLibraryInfo::TranslationsPath),
                             qtTranslator)) {
            pApp->installTranslator(qtTranslator);
        } else {
            delete qtTranslator;
        }

        // Load Qt translations for this locale from the Mixxx translations
        // folder.
        QTranslator* mixxxQtTranslator = new QTranslator(pApp);
        if (loadTranslations(systemLocale, userLocale, "qt", "_",
                             translationsFolder,
                             mixxxQtTranslator)) {
            pApp->installTranslator(mixxxQtTranslator);
        } else {
            delete mixxxQtTranslator;
        }

        // Load Mixxx specific translations for this locale from the Mixxx
        // translations folder.
        QTranslator* mixxxTranslator = new QTranslator(pApp);
        bool mixxxLoaded = loadTranslations(systemLocale, userLocale, "mixxx", "_",
                                            translationsFolder, mixxxTranslator);
        qDebug() << "Loading translations for locale"
                 << (userLocale.size() > 0 ? userLocale : systemLocale.name())
                 << "from translations folder" << translationsFolder << ":"
                 << (mixxxLoaded ? "success" : "fail");
        if (mixxxLoaded) {
            pApp->installTranslator(mixxxTranslator);
        } else {
            delete mixxxTranslator;
        }
    }

  private:
    static bool loadTranslations(const QLocale& systemLocale,
            const QString& userLocale,
            const QString& translation,
            const QString& prefix,
            const QString& translationPath,
            QTranslator* pTranslator) {
        if (userLocale.size() == 0) {
            QStringList uiLanguages = systemLocale.uiLanguages();
            if (uiLanguages.size() > 0 && uiLanguages.first() == "en") {
                // Don't bother loading a translation if the first ui-langauge is
                // English because the interface is already in English. This fixes
                // the case where the user's install of Qt doesn't have an explicit
                // English // TODO: ranslation file and the fact that we don't ship a
                // mixxx_en.qm.
                return false;
            }
            return pTranslator->load(systemLocale, translation, prefix, translationPath);
        }
        return pTranslator->load(translation + prefix + userLocale, translationPath);
    }

    Translations() = delete;
};

}  // namespace mixxx
