#ifndef AUDIOTAGGER_H
#define AUDIOTAGGER_H

#include <QFileInfo>

#include "metadata/trackmetadata.h"
#include "util/result.h"
#include "util/sandbox.h"

class AudioTagger {
public:
    AudioTagger(const QString& file, SecurityTokenPointer pToken);
    virtual ~AudioTagger();

    Result save(const Mixxx::TrackMetadata& trackMetadata);

private:
    QFileInfo m_file;
    SecurityTokenPointer m_pSecurityToken;
};

#endif // AUDIOTAGGER_H
