
#ifndef WAVEFORMRENDERSIGNALTILES_H
#define WAVEFORMRENDERSIGNALTILES_H

#include <QObject>
#include <QColor>
#include <QVector>
#include <QList>
#include <QLineF>
#include <QDomNode>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>

#include "renderobject.h"

class ControlObjectThreadMain;
class WaveformRenderer;
class SoundSourceProxy;

struct Tile {
    int start_subpixel;
    int end_subpixel;
    QPixmap pixmap;
};

class WaveformRenderSignalTiles : public RenderObject {
    Q_OBJECT
public:
    WaveformRenderSignalTiles(const char *group, WaveformRenderer *parent);
    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

private:
    Tile* getTileForSubpixel(int subpixel, QVector<float> *buffer);
    void resetTiles();

    WaveformRenderer *m_pParent;
    int m_iWidth, m_iHeight;
    TrackPointer m_pTrack;
    QColor signalColor;

    QHash<int, Tile*> m_qTileMap;
    QList<Tile*> m_qFreeTiles;
    QList<Tile*> m_qTiles;
    int kiTileWidth;
};

#endif
