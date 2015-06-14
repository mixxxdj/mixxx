#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QFile>
#include <QTemporaryFile>

#include "test/mixxxtest.h"

#include "util/config-parser.h"

const QString kConfigLocation(QDir::currentPath() +
                              "/src/test/settings-dummy-data/mixxx.cfg");

TEST(QSETTINGS, readConfigtest) {
    QSettings::SettingsMap map;
    QFile configFile(kConfigLocation);
    EXPECT_TRUE(configFile.open(QIODevice::ReadOnly));
    EXPECT_TRUE(readConfig(configFile, map));
    EXPECT_EQ(265, map.size());
}

TEST(QSETTINGS, writeConfigtest) {
    QSettings::SettingsMap map;
    QFile configFile(kConfigLocation);
    EXPECT_TRUE(configFile.open(QIODevice::ReadOnly));
    EXPECT_TRUE(readConfig(configFile, map));

    QTemporaryFile saveFile;
    saveFile.open();
    writeConfig(saveFile, map);
    saveFile.close();

    auto map2 = QSettings::SettingsMap();
    saveFile.open();
    readConfig(saveFile, map2);

    EXPECT_EQ(map.size(), map2.size());

    for (const auto& key : map.keys()) {
        EXPECT_QSTRING_EQ(map[key].toString(), map2[key].toString());
    }
}
