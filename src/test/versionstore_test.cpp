#include "util/versionstore.h"

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QSysInfo>
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
#endif
}

TEST_F(VersionStoreTest, KernelAndCpuArchitectureAreAvailable) {
    EXPECT_FALSE(VersionStore::kernel().isEmpty());
    EXPECT_FALSE(VersionStore::cpuArchitecture().isEmpty());
}

TEST_F(VersionStoreTest, MaskedBuildFlagsMasksPrefixMapValues) {
    EXPECT_EQ(QStringLiteral("-fmacro-prefix-map=<masked> -Wall"),
            VersionStore::maskedBuildFlags(
                    QStringLiteral("-fmacro-prefix-map=/home/dev/mixxx=. -Wall")));
    EXPECT_EQ(QStringLiteral("-ffile-prefix-map=<masked>;-fmacro-prefix-map=<masked>"),
            VersionStore::maskedBuildFlags(QStringLiteral(
                    "-ffile-prefix-map=/home/dev/mixxx=.;-fmacro-prefix-map=/home/dev/mixxx=")));
    // Build flags without prefix-map flags are returned unchanged.
    EXPECT_EQ(QStringLiteral("-O2 -Wall"),
            VersionStore::maskedBuildFlags(QStringLiteral("-O2 -Wall")));
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
    // A binary in a system-wide binary directory is assumed to be installed
    // by a distribution package.
    EXPECT_EQ(QStringLiteral("distro package"),
            VersionStore::installationMethod(QStringLiteral("/usr/bin")));
#ifdef MIXXX_BUILD_DIR
    // mixxx-test is executed from inside the CMake build directory, so the
    // running build is detected as a source build.
    EXPECT_EQ(QStringLiteral("source build"),
            VersionStore::installationMethod(
                    QCoreApplication::applicationDirPath()));
#endif
    // The installation method of a binary in an unknown location is omitted.
    EXPECT_TRUE(VersionStore::installationMethod(
            QStringLiteral("/nonexistent-mixxx-test-dir"))
                    .isNull());
#else
    GTEST_SKIP() << "Only relevant on Linux";
#endif
}

TEST_F(VersionStoreTest, DiagnosticInfoContainsBuildDetails) {
    {
        // Mock a Flatpak installation running on Wayland.
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", "wayland");
        ScopedEnvironmentVariable flatpakId("FLATPAK_ID", "org.mixxx.Mixxx");

        const QString diagnosticInfo = VersionStore::diagnosticInfo(
                QStringLiteral("LateNight / PaleMoon (Classic)"));

        EXPECT_EQ(VersionStore::version(),
                valueForKey(diagnosticInfo, QStringLiteral("Version")));
        EXPECT_EQ(VersionStore::qtVersion(),
                valueForKey(diagnosticInfo, QStringLiteral("Qt")));
        EXPECT_EQ(QStringLiteral("Wayland"),
                valueForKey(diagnosticInfo, QStringLiteral("Session type")));
        EXPECT_EQ(QStringLiteral("Flatpak"),
                valueForKey(diagnosticInfo,
                        QStringLiteral("Installation method")));
        EXPECT_EQ(QStringLiteral("LateNight / PaleMoon (Classic)"),
                valueForKey(diagnosticInfo, QStringLiteral("Skin")));

#ifdef __LINUX__
        EXPECT_EQ(VersionStore::distribution(),
                valueForKey(diagnosticInfo, QStringLiteral("Distribution")));
        EXPECT_TRUE(
                valueForKey(diagnosticInfo, QStringLiteral("OS")).isNull());
#else
        // On other platforms the OS is reported instead of the distribution.
        EXPECT_FALSE(
                valueForKey(diagnosticInfo, QStringLiteral("OS")).isEmpty());
        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Distribution"))
                        .isNull());
#endif

        // The kernel and the CPU architecture are included when available.
        if (!VersionStore::kernel().isEmpty()) {
            EXPECT_EQ(VersionStore::kernel(),
                    valueForKey(diagnosticInfo, QStringLiteral("Kernel")));
        }
        if (!VersionStore::cpuArchitecture().isEmpty()) {
            EXPECT_EQ(VersionStore::cpuArchitecture(),
                    valueForKey(diagnosticInfo, QStringLiteral("Architecture")));
        }
    }
    {
        // Without the environment variables of the package formats and the
        // session type, no sandboxed installation method or session type is
        // reported. Without a skin the skin entry is omitted.
        ScopedEnvironmentVariable sessionType("XDG_SESSION_TYPE", nullptr);
        ScopedEnvironmentVariable flatpakId("FLATPAK_ID", nullptr);
        ScopedEnvironmentVariable snapName("SNAP_NAME", nullptr);
        ScopedEnvironmentVariable appImage("APPIMAGE", nullptr);

        const QString diagnosticInfo = VersionStore::diagnosticInfo();

        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Session type"))
                        .isNull());
        EXPECT_TRUE(
                valueForKey(diagnosticInfo, QStringLiteral("Skin")).isNull());
        const QString installationMethod = valueForKey(
                diagnosticInfo, QStringLiteral("Installation method"));
        EXPECT_NE(QStringLiteral("Flatpak"), installationMethod);
        EXPECT_NE(QStringLiteral("Snap"), installationMethod);
        EXPECT_NE(QStringLiteral("AppImage"), installationMethod);
    }
}

TEST_F(VersionStoreTest, DiagnosticInfoMasksPrivateBuildPaths) {
    const QString diagnosticInfo = VersionStore::diagnosticInfo();

    // The values of the -ffile-prefix-map and -fmacro-prefix-map compiler
    // flags contain private file system paths of the build machine, which
    // must not be leaked into the dump.
    EXPECT_FALSE(
            diagnosticInfo.contains(QStringLiteral("-fmacro-prefix-map=/")));
    EXPECT_FALSE(
            diagnosticInfo.contains(QStringLiteral("-ffile-prefix-map=/")));
    EXPECT_FALSE(
            diagnosticInfo.contains(QStringLiteral("-fmacro-prefix-map=C:")));
    EXPECT_FALSE(
            diagnosticInfo.contains(QStringLiteral("-ffile-prefix-map=C:")));
}

TEST_F(VersionStoreTest, DiagnosticInfoOmitsUnavailableFields) {
    const QString diagnosticInfo = VersionStore::diagnosticInfo();

    // Mandatory fields are always present.
    EXPECT_FALSE(
            valueForKey(diagnosticInfo, QStringLiteral("Version")).isNull());
    EXPECT_FALSE(valueForKey(diagnosticInfo, QStringLiteral("Qt")).isNull());

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
        EXPECT_FALSE(valueForKey(diagnosticInfo, QStringLiteral("Last commit"))
                        .isEmpty());
    } else {
        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Last commit"))
                        .isNull());
    }

#ifdef __LINUX__
    // The distribution name is only included if it can be determined.
    if (VersionStore::distribution().isEmpty()) {
        EXPECT_TRUE(valueForKey(diagnosticInfo, QStringLiteral("Distribution"))
                        .isNull());
    } else {
        EXPECT_EQ(VersionStore::distribution(),
                valueForKey(diagnosticInfo, QStringLiteral("Distribution")));
    }
#else
    // The OS name is only included if it can be determined.
    const QString prettyProductName = QSysInfo::prettyProductName();
    if (prettyProductName.isEmpty() ||
            prettyProductName == QLatin1String("unknown")) {
        EXPECT_TRUE(
                valueForKey(diagnosticInfo, QStringLiteral("OS")).isNull());
    } else {
        EXPECT_EQ(prettyProductName,
                valueForKey(diagnosticInfo, QStringLiteral("OS")));
    }
#endif
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
        EXPECT_FALSE(line.mid(keyWidth + 1).isEmpty());
    }
    // The dump does not end with a line break.
    EXPECT_FALSE(lines.last().isEmpty());
}

} // namespace
