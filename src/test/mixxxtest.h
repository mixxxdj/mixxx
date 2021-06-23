#pragma once

#include <gtest/gtest.h>

#include <QDir>
#include <QScopedPointer>
#include <QTemporaryDir>

#include "mixxxapplication.h"
#include "preferences/usersettings.h"

#define EXPECT_QSTRING_EQ(expected, test) EXPECT_STREQ(qPrintable(expected), qPrintable(test))
#define ASSERT_QSTRING_EQ(expected, test) ASSERT_STREQ(qPrintable(expected), qPrintable(test))

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

  private:
    static QScopedPointer<MixxxApplication> s_pApplication;

    const QTemporaryDir m_testDataDir;

  protected:
    UserSettingsPointer m_pConfig;
};

namespace mixxxtest {

/// Returns the full, non-empty file path on success.
///
/// For the format of fileNameTemplate refer to QTemporaryFile.
QString generateTemporaryFileName(const QString& fileNameTemplate);

/// Returns the full, non-empty file path on success.
///
/// For the format of fileNameTemplate refer to QTemporaryFile.
QString createEmptyTemporaryFile(const QString& fileNameTemplate);

bool copyFile(const QString& srcFileName, const QString& dstFileName);

class FileRemover final {
  public:
    explicit FileRemover(const QString& fileName)
            : m_fileName(fileName) {
    }
    ~FileRemover();

    void keepFile() {
        m_fileName = QString();
    }

  private:
    QString m_fileName;
};

} // namespace mixxxtest
