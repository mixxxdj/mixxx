
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>
#include <QLine>

#include "waveformrendersignaltiles.h"
#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderSignalTiles::WaveformRenderSignalTiles(const char* group, WaveformRenderer *parent)
        : m_pParent(parent),
          m_iWidth(0),
          m_iHeight(0),
          m_pTrack(NULL),
          signalColor(255,255,255),
          kiTileWidth(300) {
}

void WaveformRenderSignalTiles::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
    resetTiles();
}

void WaveformRenderSignalTiles::newTrack(TrackPointer pTrack) {
    m_pTrack = pTrack;
}

void WaveformRenderSignalTiles::setup(QDomNode node) {
    signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    signalColor = WSkinColor::getCorrectColor(signalColor);
}

void WaveformRenderSignalTiles::draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(buffer == NULL)
        return;

    int numBufferSamples = buffer->size();
    int iCurPos = 0;
    if(dPlayPos >= 0) {
        iCurPos = (int)(dPlayPos*numBufferSamples);
    }

    if((iCurPos % 2) != 0)
        iCurPos--;

    pPainter->save();

    pPainter->setPen(signalColor);

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel();

    int subpixelWidth = int(m_iWidth * subpixelsPerPixel);

    // For now only deal with rate == 1.
    Q_ASSERT(1.0 + rateAdjust == 1.0);
    pPainter->scale((1.0+rateAdjust)/subpixelsPerPixel, 1.0);

    int halfw = subpixelWidth/2;
    for(int i=0;i<subpixelWidth;) {
        // Start at curPos minus half the waveform viewer
        int thisIndex = iCurPos+2*(i-halfw);
        Q_ASSERT(thisIndex % 2 == 0);
        int thisSubpixel = thisIndex / 2;

        if(thisIndex >= 0 && (thisIndex+1) < numBufferSamples) {
            Tile* tile = getTileForSubpixel(thisSubpixel, buffer);

            if (tile != NULL) {
                Q_ASSERT(thisSubpixel >= tile->start_subpixel &&
                         thisSubpixel < tile->end_subpixel);
                int tile_width = tile->end_subpixel - tile->start_subpixel;
                int tile_offset = thisSubpixel - tile->start_subpixel;
                int tile_remaining = tile->end_subpixel - thisSubpixel;
                int screenStartSubpixel = i - tile_remaining;
                int tileVisibleSubpixels = tile->end_subpixel - thisSubpixel;
                //qDebug() << "Drawing tile for" << thisSubpixel << "screenStart" << screenStartSubpixel << "screenVisible" << tileVisibleSubpixels << "tile start" << tile->start_subpixel << "tile end" << tile->end_subpixel;

                qDebug() << "Drawing" << thisSubpixel << i << tile->start_subpixel << screenStartSubpixel;
                QRectF source(0, 0, tile_width, m_iHeight);
                QRectF target(i - tile_offset, -m_iHeight/2,
                              tile_width, m_iHeight);


                pPainter->drawPixmap(target, tile->pixmap, source);
                pPainter->drawRect(target);
                // pPainter->drawPixmap(screenStartSubpixel,
                //                      -m_iHeight/2,
                //                      tile->pixmap);
                i += tileVisibleSubpixels;
            } else {
                QPixmap pm = QPixmap(kiTileWidth, m_iHeight);
                pm.fill(Qt::red);
                pPainter->drawPixmap(i, -1.0, pm);
                i += kiTileWidth;
            }
        } else {
            i+=1;
        }
    }

    pPainter->restore();
}

void WaveformRenderSignalTiles::resetTiles() {
}

void drawTile(Tile* tile, QVector<float>* buffer) {
    QPixmap& pixmap = tile->pixmap;
    float* rawBuffer = buffer->data();
    int bufferSize = buffer->size();

    QPainter painter(&pixmap);

    painter.fillRect(0, 0, pixmap.width(), pixmap.height(), Qt::black);

    painter.setPen(Qt::green);
    // Translate coordinates frame to the center of the pixmap
    painter.translate(0.0, pixmap.height()/2.0);

    // Now scale so that positive-y points up.
    painter.scale(1.0, -1.0);

    painter.scale(1.0, pixmap.height()*0.44);

    qDebug() << "Pre-rendering tile" << tile->start_subpixel;
    for (int i = 0; i < pixmap.width(); ++i) {
        int thisIndex = (tile->start_subpixel+i)*2;
        if (thisIndex+1 < bufferSize) {
            float sampl = rawBuffer[thisIndex];
            float sampr = rawBuffer[thisIndex+1];
            QLineF line = QLineF(i, sampl, i, -sampr);
            painter.drawLine(line);
        }
    }
    painter.end();

    QImage image = tile->pixmap.toImage();
    image.save(QString("/home/rryan/%1.jpg").arg(tile->start_subpixel));

}

Tile* WaveformRenderSignalTiles::getTileForSubpixel(int subpixel,
                                                    QVector<float>* buffer) {
    int tileForSubpixel = subpixel / kiTileWidth;

    if (m_qTileMap.contains(tileForSubpixel)) {
        return m_qTileMap[tileForSubpixel];
    }

    Tile* newTile = new Tile();
    newTile->start_subpixel = tileForSubpixel * kiTileWidth;
    newTile->end_subpixel = newTile->start_subpixel + kiTileWidth;
    newTile->pixmap = QPixmap(kiTileWidth, m_iHeight);
    drawTile(newTile, buffer);
    //newTile->pixmap.fill(Qt::green);
    m_qTiles.append(newTile);
    m_qTileMap[tileForSubpixel] = newTile;
    return newTile;
}
