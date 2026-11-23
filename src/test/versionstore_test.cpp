#include "util/versionstore.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QtGlobal>

namespace {

/// RAII helper that sets an environment variable for the duration of the
/// current scope and restores the previous state on destruction. A null value
/// removes the variable from the environment.
class ScopedEnvironmentVariable final {
  public:
    ScopedEnvironmentVariable(const char* name, const char* value)
            : m_name(name),
              m_wasSet(qEnvironmentVariableIsSet(name)),
              m_previousValue(qgetenv(name)) {
        if (value != nullptr) {
            qputenv(name, QByteArray(value));
        } else {
            qunsetenv(name);
        }
    }

    ScopedEnvironmentVariable(const ScopedEnvironmentVariable&) = delete;
    ScopedEnvironmentVariable& operator=(const ScopedEnvironmentVariable&) = delete;

    ~ScopedEnvironmentVariable() {
        if (m_wasSet) {
            qputenv(m_name.constData(), m_previousValue);
        } else {
            qunsetenv(m_name.constData());
        }
    }

  private:
    const QByteArray m_name;
    const bool m_wasSet;
    const QByteArray m_previousValue;
};

/// Returns the value of the line starting with the given key or the null
/// string if the dump does not contain the key.
QString valueForKey(const QString& diagnosticInfo, const QString& key) {
    const QStringList lines = diagnosticInfo.split(QChar('\n'));
    for (const QString& line : lines) {
        if (line.startsWith(key + QChar(' '))) {
            return line.mid(key.length()).trimmed();
        }
    }
    return QString();
}

class VersionStoreTest : public testing::Test {
  protected:
    void SetUp() override {
        // Keep the tests independent of the environment they are run in, e.g.
        // the Flatpak build sandbox of the CI or an AppImage bundle.
        qunsetenv("FLATPAK_ID");
        qunsetenv("SNAP_NAME");
        qunsetenv("APPIMAGE");
    }
};

TEST_F(VersionStoreTest, SessionTypeFollowsXDGSessionType) {
#ifdef __LINUX__
    {
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", "wayland");
        EXPECT_EQ(QStringLiteral("Wayland"), VersionStore::sessionType());
    }
    {
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", "x11");
        EXPECT_EQ(QStringLiteral("X11"), VersionStore::sessionType());
    }
    {
        // Session types other than Wayland and X11 are omitted.
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", "tty");
        EXPECT_TRUE(VersionStore::sessionType().isNull());
    }
#endif
    {
        // Without XDG_SESSION_TYPE there is nothing to report.
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", nullptr);
        EXPECT_TRUE(VersionStore::sessionType().isNull());
    }
}

TEST_F(VersionStoreTest, DistributionPrefersOsReleasePrettyName) {
#ifdef __LINUX__
    QTemporaryFile osReleaseFile;
    ASSERT_TRUE(osReleaseFile.open());
    osReleaseFile.write("NAME=customdistro\n");
    osReleaseFile.write("PRETTY_NAME=\"Custom Distro 1.2 LTS\"\n");
    osReleaseFile.flush();
    EXPECT_EQ(QStringLiteral("Custom Distro 1.2 LTS"),
            VersionStore::distribution(osReleaseFile.fileName()));

    // The real os-release file of the test machine provides a distribution
    // name as well.
    EXPECT_FALSE(VersionStore::distribution().isEmpty());
#else
    // On other platforms there is no distribution to report.
    EXPECT_TRUE(VersionStore::distribution(QStringLiteral("/nonexistent"))
                    .isNull());
#endif
}

TEST_F(VersionStoreTest, DistributionFallsBackToQSysInfo) {
#ifdef __LINUX__
    // Without a readable os-release file (or without a PRETTY_NAME field),
    // the distribution falls back to QSysInfo.
    const QString expectedFallback = QSysInfo::productType() +
            QStringLiteral(" ") + QSysInfo::productVersion();
    EXPECT_EQ(expectedFallback,
            VersionStore::distribution(QStringLiteral("/nonexistent/os-release")));
#else
    GTEST_SKIP() << "Only relevant on Linux";
#endif
}

TEST_F(VersionStoreTest, CpuArchitectureIsAvailable) {
    EXPECT_FALSE(VersionStore::cpuArchitecture().isEmpty());
}

TEST_F(VersionStoreTest, InstallationMethodFromEnvironmentVariables) {
    // Use a directory that is neither a system-wide binary directory nor the
    // CMake build directory, so that the result only depends on the
    // environment variables.
    const QString unrelatedDir = QStringLiteral("/nonexistent-mixxx-test-dir");
#ifdef __LINUX__
    {
        ScopedEnvironmentVariable flatpakId("FLATPAK_ID", "org.mixxx.Mixxx");
        EXPECT_EQ(QStringLiteral("Flatpak"),
                VersionStore::installationMethod(unrelatedDir));
    }
    {
        ScopedEnvironmentVariable snapName("SNAP_NAME", "mixxx");
        EXPECT_EQ(QStringLiteral("Snap"),
                VersionStore::installationMethod(unrelatedDir));
    }
    {
        ScopedEnvironmentVariable appImage(
                "APPIMAGE", "/nonexistent/mixxx.AppImage");
        EXPECT_EQ(QStringLiteral("AppImage"),
                VersionStore::installationMethod(unrelatedDir));
    }
    {
        // Flatpak takes precedence over Snap and AppImage.
        ScopedEnvironmentVariable flatpakId("FLATPAK_ID", "org.mixxx.Mixxx");
        ScopedEnvironmentVariable snapName("SNAP_NAME", "mixxx");
        ScopedEnvironmentVariable appImage(
                "APPIMAGE", "/nonexistent/mixxx.AppImage");
        EXPECT_EQ(QStringLiteral("Flatpak"),
                VersionStore::installationMethod(unrelatedDir));
    }
#else
    // On other platforms the installation method is always omitted, even if
    // the environment variables of the various package formats are set.
    ScopedEnvironmentVariable flatpakId("FLATPAK_ID", "org.mixxx.Mixxx");
    ScopedEnvironmentVariable snapName("SNAP_NAME", "mixxx");
    ScopedEnvironmentVariable appImage("APPIMAGE", "/nonexistent/mixxx.AppImage");
    EXPECT_TRUE(VersionStore::installationMethod(unrelatedDir).isNull());
#endif
}

TEST_F(VersionStoreTest, InstallationMethodFromExecutableLocation) {
#ifdef __LINUX__
    // Exercise the path based detection without relying on the layout of the
    // file system the test runs on. The test binary is not necessarily
    // executed from inside the CMake build directory (e.g. in the Flatpak
    // build sandbox of the CI) and /usr/bin is optional.
    const QTemporaryDir temporaryDir;
    ASSERT_TRUE(temporaryDir.isValid());

    // An executable location other than /usr/bin is taken as a build of the
    // sources.
    EXPECT_EQ(QStringLiteral("source build"),
            VersionStore::installationMethod(temporaryDir.path()));

    // A path that does not exist is omitted, even though it and a missing
    // system-wide binary directory both have the null string as canonical
    // path.
    EXPECT_TRUE(VersionStore::installationMethod(
            temporaryDir.filePath(QStringLiteral("not-existing")))
                    .isNull());

    // An executable in /usr/bin is assumed to be installed by a distribution
    // package, provided that the directory exists on this machine.
    const QString systemBinDir = QStringLiteral("/usr/bin");
    if (QFileInfo::exists(systemBinDir)) {
        EXPECT_EQ(QStringLiteral("distro package"),
                VersionStore::installationMethod(systemBinDir));
    }
#else
    GTEST_SKIP() << "Only relevant on Linux";
#endif
}

TEST_F(VersionStoreTest, DiagnosticInfoContents) {
    {
        // Mock a Flatpak installation running on Wayland with a configured
        // skin.
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", "wayland");
        ScopedEnvironmentVariable flatpakId("FLATPAK_ID", "org.mixxx.Mixxx");

        const QString diagnosticInfo = VersionStore::diagnosticInfo(
                QStringLiteral("LateNight / PaleMoon"), QStringLiteral("Legacy"));

        // Mandatory fields are always present.
        EXPECT_EQ(VersionStore::version(),
                valueForKey(diagnosticInfo, QStringLiteral("Version")));
        EXPECT_EQ(VersionStore::qtVersion(),
                valueForKey(diagnosticInfo, QStringLiteral("Qt")));
        EXPECT_EQ(VersionStore::cpuArchitecture(),
                valueForKey(diagnosticInfo, QStringLiteral("Architecture")));
        EXPECT_EQ(QStringLiteral("LateNight / PaleMoon"),
                valueForKey(diagnosticInfo, QStringLiteral("Skin")));
        EXPECT_EQ(QStringLiteral("Legacy"),
                valueForKey(diagnosticInfo, QStringLiteral("Skin type")));

        // The Git version is only included if the build provides it.
        if (VersionStore::gitDescribe().isEmpty()) {
            EXPECT_TRUE(
                    valueForKey(diagnosticInfo, QStringLiteral("Git version"))
                            .isNull());
        } else {
            EXPECT_EQ(VersionStore::gitVersion(),
                    valueForKey(diagnosticInfo, QStringLiteral("Git version")));
        }

        // The last commit date is only included if it is available.
        if (VersionStore::date().isValid()) {
            EXPECT_EQ(VersionStore::date().toUTC().toString(Qt::ISODate),
                    valueForKey(diagnosticInfo, QStringLiteral("Last commit")));
        } else {
            EXPECT_TRUE(
                    valueForKey(diagnosticInfo, QStringLiteral("Last commit"))
                            .isNull());
        }

#ifdef __LINUX__
        // The distribution name is only included if it can be determined.
        if (VersionStore::distribution().isEmpty()) {
            EXPECT_TRUE(
                    valueForKey(diagnosticInfo, QStringLiteral("Distribution"))
                            .isNull());
        } else {
            EXPECT_EQ(VersionStore::distribution(),
                    valueForKey(diagnosticInfo,
                            QStringLiteral("Distribution")));
        }
        EXPECT_EQ(QStringLiteral("Wayland"),
                valueForKey(diagnosticInfo, QStringLiteral("Session type")));
        EXPECT_EQ(QStringLiteral("Flatpak"),
                valueForKey(diagnosticInfo,
                        QStringLiteral("Installation method")));
        EXPECT_TRUE(
                valueForKey(diagnosticInfo, QStringLiteral("OS")).isNull());
#else
        // On other platforms the OS is reported instead of the distribution.
        EXPECT_FALSE(
                valueForKey(diagnosticInfo, QStringLiteral("OS")).isEmpty());
        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Distribution"))
                        .isNull());
#endif
    }
    {
        // The variables of the package formats are cleared by SetUp(). Without
        // a session type that entry is omitted and the installation method is
        // not a sandboxed one; without a skin the skin entries are omitted.
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", nullptr);

        const QString diagnosticInfo = VersionStore::diagnosticInfo();

        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Session type"))
                        .isNull());
        EXPECT_TRUE(
                valueForKey(diagnosticInfo, QStringLiteral("Skin")).isNull());
        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Skin type"))
                        .isNull());
        const QString installationMethod = valueForKey(
                diagnosticInfo, QStringLiteral("Installation method"));
        EXPECT_NE(QStringLiteral("Flatpak"), installationMethod);
        EXPECT_NE(QStringLiteral("Snap"), installationMethod);
        EXPECT_NE(QStringLiteral("AppImage"), installationMethod);
    }
    {
        // Details that are not helpful for triaging bug reports, or that must
        // not be published, are not part of the dump. The compile options
        // contain the private file system paths of the build machine (in the
        // value of -fmacro-prefix-map) and unevaluated CMake generator
        // expressions.
        const QString diagnosticInfo = VersionStore::diagnosticInfo();

        EXPECT_TRUE(
                valueForKey(diagnosticInfo, QStringLiteral("Kernel")).isNull());
        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Build flags"))
                        .isNull());
        EXPECT_FALSE(diagnosticInfo.contains(QStringLiteral("$<")));
        EXPECT_FALSE(
                diagnosticInfo.contains(QStringLiteral("-fmacro-prefix-map")));
        EXPECT_FALSE(
                diagnosticInfo.contains(QStringLiteral("-ffile-prefix-map")));
    }
}

TEST_F(VersionStoreTest, DiagnosticInfoIsAlignedKeyValueText) {
    const QString diagnosticInfo = VersionStore::diagnosticInfo();
    const QStringList lines = diagnosticInfo.split(QChar('\n'));
    ASSERT_GT(lines.size(), 3);

    // Compute the width of the longest key, i.e. the position of the first
    // space in any line never exceeds this width.
    qsizetype keyWidth = 0;
    for (const QString& line : lines) {
        const qsizetype firstSpace = line.indexOf(QChar(' '));
        ASSERT_GT(firstSpace, 0);
        keyWidth = qMax(keyWidth, firstSpace);
    }

    // Every key is left-justified to the width of the longest key followed by
    // a single space, so the value of every line starts at the same column.
    for (const QString& line : lines) {
        EXPECT_EQ(QChar(' '), line.at(keyWidth));
        EXPECT_LT(keyWidth + 1, line.size());
    }
    // The dump does not end with a line break.
    EXPECT_FALSE(lines.last().isEmpty());
}

} // namespace
