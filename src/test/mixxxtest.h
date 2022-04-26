#pragma once

#include <gtest/gtest.h>

#include <QDir>
#include <QScopedPointer>
#include <QTemporaryDir>

#include "mixxxapplication.h"
#include "preferences/usersettings.h"

#define EXPECT_QSTRING_EQ(expected, test) EXPECT_STREQ(qPrintable(expected), qPrintable(test))
#define ASSERT_QSTRING_EQ(expected, test) ASSERT_STREQ(qPrintable(expected), qPrintable(test))

namespace {

// We assume that the test folder is a sibling to the res folder
const QString kTestPath = QStringLiteral("../src/test");

} // namespace

class MixxxTest : public testing::Test {
  public:
    MixxxTest();
    ~MixxxTest() override;

    // ApplicationScope creates QApplication as a singleton and keeps
    // it alive during all tests. This prevents issues with creating
    // and destroying the QApplication multiple times in the same process.
    // http://stackoverflow.com/questions/14243858/qapplication-segfaults-in-googletest
    class ApplicationScope final {
      public:
        ApplicationScope(int& argc, char** argv);
        ~ApplicationScope();
    };
    friend class ApplicationScope;

    static const QDir& getOrInitTestDir(const QString& resourcePath = QString()) {
        if (s_TestDir.path() == ".") {
            s_TestDir.setPath(
                    (resourcePath.isEmpty() ? ConfigObject<ConfigValue>::
                                                      computeResourcePath()
                                            : resourcePath) +
                    QChar('/') + kTestPath);
        }
        return s_TestDir;
    }

  protected:
    static QApplication* application() {
        return s_pApplication.data();
    }

    UserSettingsPointer config() const {
        return m_pConfig;
    }

    // Simulate restarting Mixxx by saving and reloading the UserSettings.
    void saveAndReloadConfig();

    QDir getTestDataDir() const {
        return m_testDataDir.path();
    }

    const QDir& getTestDir() const {
        return getOrInitTestDir(m_pConfig->getResourcePath());
    }

  private:
    static QScopedPointer<MixxxApplication> s_pApplication;
    static QDir s_TestDir;
    const QTemporaryDir m_testDataDir;

  protected:
    UserSettingsPointer m_pConfig;
};

namespace mixxxtest {

void copyFile(const QString& srcFileName, const QString& dstFileName);

} // namespace mixxxtest
