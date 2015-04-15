#include "soundsourceplugin.h"

namespace Mixxx {

char** SoundSourcePlugin::allocFileExtensions(
        const QList<QString>& supportedFileExtensions) {
    //Convert to C string array.
    char** fileExtensions = (char**) malloc(
            (supportedFileExtensions.count() + 1) * sizeof(char*));
    for (int i = 0; i < supportedFileExtensions.count(); i++) {
        QByteArray qba = supportedFileExtensions[i].toUtf8();
        fileExtensions[i] = strdup(qba.constData());
        qDebug() << fileExtensions[i];
    }
    fileExtensions[supportedFileExtensions.count()] = NULL; //NULL terminate the list

    return fileExtensions;
}

void SoundSourcePlugin::freeFileExtensions(char** fileExtensions) {
    if (fileExtensions) {
        for (int i(0); fileExtensions[i]; ++i) {
            free(fileExtensions[i]);
        }
        free(fileExtensions);
    }
}

} // namespace Mixxx
