#include "sources/soundsourcemodplug.h"

#include "sources/audiosourcemodplug.h"
#include "metadata/trackmetadata.h"
#include "util/timer.h"

#include <stdlib.h>
#include <unistd.h>

#include <QtDebug>

QString SoundSourceModPlug::getTypeFromUrl(QUrl url) {
    const QString type(SoundSource::getTypeFromUrl(url));
    if (type == "mod") {
        return "Protracker";
    } else if (type == "med") {
        return "OctaMed";
    } else if (type == "okt") {
        return "Oktalyzer";
    } else if (type == "s3m") {
        return "Scream Tracker 3";
    } else if (type == "stm") {
        return "Scream Tracker";
    } else if (type == "xm") {
        return "FastTracker2";
    } else if (type == "it") {
        return "Impulse Tracker";
    } else {
        return "Module";
    }
}

QList<QString> SoundSourceModPlug::supportedFileExtensions() {
    QList<QString> list;
    // ModPlug supports more formats but file name
    // extensions are not always present with modules.
    list.push_back("mod");
    list.push_back("med");
    list.push_back("okt");
    list.push_back("s3m");
    list.push_back("stm");
    list.push_back("xm");
    list.push_back("it");
    return list;
}

SoundSourceModPlug::SoundSourceModPlug(QUrl url) :
        SoundSource(url, getTypeFromUrl(url)) {
}

Result SoundSourceModPlug::parseMetadata(
        Mixxx::TrackMetadata* pMetadata) const {
    QFile modFile(getLocalFilePath());
    modFile.open(QIODevice::ReadOnly);
    const QByteArray fileBuf(modFile.readAll());
    modFile.close();

    ModPlug::ModPlugFile* pModFile = ModPlug::ModPlug_Load(fileBuf.constData(),
            fileBuf.length());
    if (NULL != pModFile) {
        pMetadata->setComment(QString(ModPlug::ModPlug_GetMessage(pModFile)));
        pMetadata->setTitle(QString(ModPlug::ModPlug_GetName(pModFile)));
        pMetadata->setDuration(ModPlug::ModPlug_GetLength(pModFile) / 1000);
        pMetadata->setBitrate(8); // not really, but fill in something...
        ModPlug::ModPlug_Unload(pModFile);
    }

    return pModFile ? OK : ERR;
}

QImage SoundSourceModPlug::parseCoverArt() const {
    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    return QImage();
}

Mixxx::AudioSourcePointer SoundSourceModPlug::open() const {
    return Mixxx::AudioSourceModPlug::create(getUrl());
}
