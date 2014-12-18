
#ifndef AUDIOTAGGER_H
#define AUDIOTAGGER_H

#include "metadata/trackmetadata.h"
#include "util/sandbox.h"

#include <QFileInfo>

class AudioTagger {
  public:
    AudioTagger(const QString& file, SecurityTokenPointer pToken);
    virtual ~AudioTagger();

    bool save(const Mixxx::TrackMetadata& trackMetadata);

  private:
    QFileInfo m_file;
    SecurityTokenPointer m_pSecurityToken;
};

#endif // AUDIOTAGGER_H
