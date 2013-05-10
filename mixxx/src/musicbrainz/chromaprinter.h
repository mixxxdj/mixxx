/*****************************************************************************
 *  Copyright © 2012 John Maguire <john.maguire@gmail.com>                   *
 *                   David Sansome <me@davidsansome.com>                     *
 *  This work is free. You can redistribute it and/or modify it under the    *
 *  terms of the Do What The Fuck You Want To Public License, Version 2,     *
 *  as published by Sam Hocevar.                                             *
 *  See http://www.wtfpl.net/ for more details.                              *
 *****************************************************************************/
    
#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <QObject>
#include <QFile>

#include "trackinfoobject.h"

class SoundSourceProxy;

class chromaprinter: public QObject {
  Q_OBJECT

  public:
    chromaprinter(QObject* parent=NULL);
    QString getFingerPrint(TrackPointer pTrack, bool mixxx=true);
    QString getFingerPrint(QString location, bool mixxx=true);

  private:

    QString calcFingerPrint(SoundSourceProxy& soundSource, bool mixxx);
    unsigned int m_NumSamples;
    unsigned int m_SampleRate;
};

#endif //CHROMAPRINTER_H
