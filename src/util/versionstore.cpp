#include "util/versionstore.h"

#include <soundtouch/SoundTouch.h>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QStringList>
#include <QtDebug>
#include <QtGlobal>

// shout.h checks for WIN32 to see if we are on Windows.
#ifdef WIN64
#define WIN32
#endif
#ifdef __BROADCAST__
#include <shoutidjc/shout.h>
#endif
#ifdef WIN64
#undef WIN32
#endif

#include <FLAC/format.h>
#include <chromaprint.h>
#include <lame/lame.h>
#include <portaudio.h>
#include <rubberband/RubberBandStretcher.h>
#include <sndfile.h>
#include <taglib/taglib.h>
#include <vorbis/codec.h>

#define VERSION_STORE
#include "version.h"

namespace {

const QString kMixxxVersion = QStringLiteral(MIXXX_VERSION);
const QString kMixxx = QStringLiteral("Mixxx");
#ifdef BUILD_BRANCH
const QString kBuildBranch = QStringLiteral(BUILD_BRANCH);
#else
const QString kBuildBranch;
#endif
#ifdef BUILD_REV
const QString kBuildRev = QStringLiteral(BUILD_REV);
#else
const QString kBuildRev;
#endif
#ifdef BUILD_FLAGS
const QString kBuildFlags = QStringLiteral(BUILD_FLAGS);
#else
const QString kBuildFlags;
#endif

} // namespace

// static
QString VersionStore::version() {
    return MIXXX_VERSION;
}

// static
QString VersionStore::applicationName() {
    return kMixxx;
}

// static
QString VersionStore::applicationTitle() {
#ifdef __APPLE__
    QString base = kMixxx;
#elif defined(AMD64) || defined(EM64T) || defined(x86_64)
    QString base("Mixxx " MIXXX_VERSION " x64");
#elif defined(IA64)
    QString base("Mixxx " MIXXX_VERSION " Itanium");
#else
    QString base("Mixxx " MIXXX_VERSION);
#endif

#ifdef MIXXX_BUILD_NUMBER_IN_TITLE_BAR
    QString branch = developmentBranch();
    QString branch_revision = developmentRevision();
    if (!branch.isEmpty() && !branch_revision.isEmpty()) {
        base.append(QString(" (build %1-r%2)")
                            .arg(branch, branch_revision));
    }
#endif
    return base;
}

// static
QString VersionStore::developmentBranch() {
    return kBuildBranch;
}

// static
QString VersionStore::developmentRevision() {
    return kBuildRev;
}

// static
QString VersionStore::buildFlags() {
    return kBuildFlags;
}

QStringList VersionStore::dependencyVersions() {
    char sndfile_version[128];
    sf_command(nullptr, SFC_GET_LIB_VERSION, sndfile_version, sizeof(sndfile_version));
    // Null-terminate just in case.
    sndfile_version[sizeof(sndfile_version) - 1] = '\0';
    // WARNING: may be inaccurate since some come from compile-time header
    // definitions instead of the actual dynamically loaded library).
    QStringList result;
    result
            // Should be accurate.
            << QString("Qt: %1").arg(qVersion())
#ifdef __BROADCAST__
            // Should be accurate.
            << QString("libshout: %1")
                       .arg(shout_version(nullptr, nullptr, nullptr))
#endif
            << QString("PortAudio: %1 %2")
                       .arg(Pa_GetVersion())
                       .arg(Pa_GetVersionText())
            // The version of the RubberBand headers Mixxx was compiled with.
            << QString("RubberBand: %1").arg(RUBBERBAND_VERSION)
            // The version of the SoundTouch headers Mixxx was compiled with.
            << QString("SoundTouch: %1").arg(SOUNDTOUCH_VERSION)
            // The version of the TagLib headers Mixxx was compiled with.
            << QString("TagLib: %1.%2.%3")
                       .arg(QString::number(TAGLIB_MAJOR_VERSION),
                               QString::number(TAGLIB_MINOR_VERSION),
                               QString::number(TAGLIB_PATCH_VERSION))
            // The version of the ChromaPrint headers Mixxx was compiled with.
            << QString("ChromaPrint: %1.%2.%3")
                       .arg(QString::number(CHROMAPRINT_VERSION_MAJOR),
                               QString::number(CHROMAPRINT_VERSION_MINOR),
                               QString::number(CHROMAPRINT_VERSION_PATCH))
            // Should be accurate.
            << QString("Vorbis: %1").arg(vorbis_version_string())
            // Should be accurate.
            << QString("libsndfile: %1").arg(sndfile_version)
            // The version of the FLAC headers Mixxx was compiled with.
            << QString("FLAC: %1").arg(FLAC__VERSION_STRING)
            << QString("libmp3lame: %1").arg(get_lame_version());

    return result;
}

void VersionStore::logBuildDetails() {
    QString version = VersionStore::version();
    QString buildBranch = developmentBranch();
    QString buildRevision = developmentRevision();
    QString buildFlags = VersionStore::buildFlags();

    QStringList buildInfo;
    if (!buildBranch.isEmpty() && !buildRevision.isEmpty()) {
        buildInfo.append(
                QString("git %1 r%2").arg(buildBranch, buildRevision));
    } else if (!buildRevision.isEmpty()) {
        buildInfo.append(
                QString("git r%2").arg(buildRevision));
    }
#ifndef DISABLE_BUILDTIME // buildtime=1, on by default
    buildInfo.append("built on: " __DATE__ " @ " __TIME__);
#endif
    if (!buildFlags.isEmpty()) {
        buildInfo.append(QString("flags: %1").arg(buildFlags.trimmed()));
    }
    QString buildInfoFormatted = QString("(%1)").arg(buildInfo.join("; "));

    // This is the first line in mixxx.log
    qDebug() << applicationName() << version << buildInfoFormatted << "is starting...";

    QStringList depVersions = dependencyVersions();
    qDebug() << "Compile time library versions:";
    foreach (const QString& depVersion, depVersions) {
        qDebug() << qPrintable(depVersion);
    }

    qDebug() << "QStandardPaths::writableLocation(HomeLocation):"
             << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "QStandardPaths::writableLocation(DataLocation):"
             << QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    qDebug() << "QCoreApplication::applicationDirPath()"
             << QCoreApplication::applicationDirPath();
}
