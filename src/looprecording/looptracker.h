//  looptracker.h
//  Created by Carl Pillot on 8/23/13.

#ifndef __LOOPTRACKER_H__
#define __LOOPTRACKER_H__

#include <QtCore>

class LoopTracker {
  public:
    LoopTracker();
    virtual ~LoopTracker();
    
    void addLoopLayer(QString path, unsigned int length);
    void clear();
    bool finalizeLoop(QString newPath);
    QString getCurrentPath();
    void setCurrentLength(unsigned int length);

  private:
    struct LayerInfo {
        QString path;
        unsigned int length;
    };
    QList<LayerInfo*> m_layers;
    int currentLayer;
    bool m_bIsUndoAvailable;
    bool m_bIsRedoAvailable;
};

#endif