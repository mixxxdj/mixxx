#ifndef LEARNINGUTILS_H
#define LEARNINGUTILS_H

#include <QList>
#include <QPair>

#include "controllers/midi/midimessage.h"

class LearningUtils {
  public:
    static QList<QPair<MidiKey, MidiOptions> > guessMidiInputMappings(
        const QList<QPair<MidiKey, unsigned char> >& messages);
};

#endif /* LEARNINGUTILS_H */
