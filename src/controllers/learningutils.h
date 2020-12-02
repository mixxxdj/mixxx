#ifndef LEARNINGUTILS_H
#define LEARNINGUTILS_H

#include <QList>
#include <QPair>

#include "controllers/midi/midimessage.h"

class ConfigKey;
template<class T1, class T2>
struct QPair;

class LearningUtils {
  public:
    static MidiInputMappings guessMidiInputMappings(
        const ConfigKey& control,
        const QList<QPair<MidiKey, unsigned char> >& messages);
};

#endif /* LEARNINGUTILS_H */
