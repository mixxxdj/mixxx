//
// C++ Implementation: woverview
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QBrush>
#include <QtDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <qpainter.h>
#include <QtDebug>
#include <qpixmap.h>
#include <qapplication.h>

#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "woverview.h"
#include "wskincolor.h"
#include "trackinfoobject.h"
#include "mathstuff.h"
#include "util/timer.h"

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

WOverview::WOverview(const char *pGroup, ConfigObject<ConfigValue>* pConfig, QWidget * parent) :
        WWidget(parent),
        m_pGroup(pGroup),
        m_pConfig(pConfig),
        m_pWaveform(NULL),
        m_pWaveformSourceImage(NULL),
        m_pixmapDone(false),
        m_waveformPeak(-1.0),
        m_actualCompletion(0),
        m_bDrag(false),
        m_iPos(0),
        m_a(1.0),
        m_b(0.0),
        m_analyserProgress(0),
        m_trackLoaded(false),
        m_diffGain(0) {

    m_endOfTrackControl = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_pGroup,"end_of_track")));
    connect(m_endOfTrackControl, SIGNAL(valueChanged(double)),
             this, SLOT( onEndOfTrackChange(double)));
    m_endOfTrack = false;

    m_trackSamplesControl = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pGroup, "track_samples")));
    setAcceptDrops(true);

    m_playControl = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pGroup, "play")));
}

WOverview::~WOverview() {
    delete m_endOfTrackControl;
    delete m_trackSamplesControl;
    delete m_playControl;
    if (m_pWaveformSourceImage) {
        delete m_pWaveformSourceImage;
    }
}

void WOverview::setup(QDomNode node) {
    m_signalColors.setup(node);

    m_qColorBackground = m_signalColors.getBgColor();

    // Clear the background pixmap, if it exists.
    m_backgroundPixmap = QPixmap();
    m_backgroundPixmapPath = WWidget::selectNodeQString(node, "BgPixmap");
    if (m_backgroundPixmapPath != "") {
        m_backgroundPixmap = QPixmap(WWidget::getPath(m_backgroundPixmapPath));
        if (m_backgroundPixmap.size() != size()) {
            qDebug() << "WOverview: BgPixmap does not fit. Widget size:" << size()
                     << "BgPixmap size: << m_backgroundPixmap.size()";
        }
    }

    m_endOfTrackColor = QColor(200, 25, 20);
    const QString endOfTrackColorName = WWidget::selectNodeQString(node, "EndOfTrackColor");
    if (!endOfTrackColorName.isNull()) {
        m_endOfTrackColor.setNamedColor(endOfTrackColorName);
        m_endOfTrackColor = WSkinColor::getCorrectColor(m_endOfTrackColor);
    }

    QPalette palette; //Qt4 update according to http://doc.trolltech.com/4.4/qwidget-qt3.html#setBackgroundColor (this could probably be cleaner maybe?)
    palette.setColor(this->backgroundRole(), m_qColorBackground);
    setPalette(palette);

    //setup hotcues and cue and loop(s)
    m_marks.setup(m_pGroup, node, m_signalColors);

    for (int i = 0; i < m_marks.size(); ++i) {
        WaveformMark& mark = m_marks[i];
        connect(mark.m_pointControl, SIGNAL(valueChanged(double)),
                this, SLOT(onMarkChanged(double)));
    }

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "MarkRange") {
            m_markRanges.push_back(WaveformMarkRange());
            WaveformMarkRange& markRange = m_markRanges.back();
            markRange.setup(m_pGroup, child, m_signalColors);

            connect(markRange.m_markEnabledControl, SIGNAL(valueChanged(double)),
                     this, SLOT(onMarkRangeChange(double)));
            connect(markRange.m_markStartPointControl, SIGNAL(valueChanged(double)),
                     this, SLOT(onMarkRangeChange(double)));
            connect(markRange.m_markEndPointControl, SIGNAL(valueChanged(double)),
                     this, SLOT(onMarkRangeChange(double)));
        }
        child = child.nextSibling();
    }

    //qDebug() << "WOverview : m_marks" << m_marks.size();
    //qDebug() << "WOverview : m_markRanges" << m_markRanges.size();
}

void WOverview::setValue(double fValue) {
    if (!m_bDrag)
    {
        // Calculate handle position
        int iPos = valueToPosition(fValue);
        if (iPos != m_iPos) {
            m_iPos = iPos;
            //qDebug() << "WOverview::setValue" << fValue << ">>" << m_iPos;
            update();
        }
    }
}

void WOverview::slotWaveformSummaryUpdated() {
    if (!m_pCurrentTrack) {
        return;
    }
    m_pWaveform = m_pCurrentTrack->getWaveformSummary();
    // If the waveform is already complete, just draw it.
    if (m_pWaveform && m_pWaveform->getCompletion() == m_pWaveform->getDataSize()) {
        m_actualCompletion = 0;
        if (drawNextPixmapPart()) {
            update();
        }
    }
}

void WOverview::slotAnalyserProgress(int progress) {
    if (!m_pCurrentTrack) {
        return;
    }
    const int analyserProgress = width() * progress / 1000;
    bool updateNeeded = drawNextPixmapPart();
    // progress 0 .. 1000
    if (updateNeeded || (m_analyserProgress != analyserProgress)) {
        m_analyserProgress = analyserProgress;
        update();
    }
}

void WOverview::slotLoadNewTrack(TrackPointer pTrack) {
    //qDebug() << "WOverview::slotLoadNewTrack(TrackPointer pTrack)";
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), SIGNAL(waveformSummaryUpdated()),
                   this, SLOT(slotWaveformSummaryUpdated()));
        disconnect(m_pCurrentTrack.data(), SIGNAL(analyserProgress(int)),
                   this, SLOT(slotAnalyzerProgress(int)));
    }

    if (m_pWaveformSourceImage) {
        delete m_pWaveformSourceImage;
        m_pWaveformSourceImage = NULL;
    }

    m_analyserProgress = 0;
    m_actualCompletion = 0;
    m_waveformPeak = -1.0;
    m_pixmapDone = false;
    m_trackLoaded = false;

    if (pTrack) {
        m_pCurrentTrack = pTrack;
        m_pWaveform = pTrack->getWaveformSummary();

        connect(pTrack.data(), SIGNAL(waveformSummaryUpdated()),
                this, SLOT(slotWaveformSummaryUpdated()));
        connect(pTrack.data(), SIGNAL(analyserProgress(int)),
                this, SLOT(slotAnalyserProgress(int)));

        slotAnalyserProgress(pTrack->getAnalyserProgress());
    }
}

void WOverview::slotTrackLoaded(TrackPointer pTrack) {
    if (m_pCurrentTrack == pTrack) {
        m_trackLoaded = true;
        update();
    }
}

void WOverview::slotUnloadTrack(TrackPointer /*pTrack*/) {
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), SIGNAL(waveformSummaryUpdated()),
                   this, SLOT(slotWaveformSummaryUpdated()));
        disconnect(m_pCurrentTrack.data(), SIGNAL(analyserProgress(int)),
                   this, SLOT(slotAnalyserProgress(int)));
    }
    m_pCurrentTrack.clear();
    m_pWaveform = NULL;
    m_actualCompletion = 0;
    m_waveformPeak = -1.0;
    m_pixmapDone = false;
    m_trackLoaded = false;

    update();
}

void WOverview::onEndOfTrackChange(double v) {
    //qDebug() << "WOverview::onEndOfTrackChange()" << v;
    m_endOfTrack = v > 0.5;
    update();
}

void WOverview::onMarkChanged(double /*v*/) {
    //qDebug() << "WOverview::onMarkChanged()" << v;
    update();
}

void WOverview::onMarkRangeChange(double /*v*/) {
    //qDebug() << "WOverview::onMarkRangeChange()" << v;
    update();
}

bool WOverview::drawNextPixmapPart() {
    ScopedTimer t("WOverview::drawNextPixmapPart");

    //qDebug() << "WOverview::drawNextPixmapPart() - m_waveform" << m_waveform;

    int currentCompletion;

    if (!m_pWaveform) {
        return false;
    }

    const int dataSize = m_pWaveform->getDataSize();
    if (dataSize == 0 ) {
        return false;
    }

    if (!m_pWaveformSourceImage) {
        //waveform pixmap twice the height of the viewport to be scalable by total_gain
        //we keep full range waveform data to scale it on paint
        m_pWaveformSourceImage = new QImage(dataSize / 2, 2 * 255, QImage::Format_ARGB32_Premultiplied);
        m_pWaveformSourceImage->fill(QColor(0,0,0,0).value());
    }

    const int waveformCompletion = m_pWaveform->getCompletion(); // always multiple of 2
    // test if there is some new to draw (at least of pixel width)
    const int completionIncrement = waveformCompletion - m_actualCompletion;

    int visiblePixelIncrement = completionIncrement * width() / dataSize; 
    if (completionIncrement < 2 || visiblePixelIncrement == 0) {
        return false;
    }

    if (!m_pWaveform->getMutex()->tryLock()) {
        return false;
    }

    const int nextCompletion = m_actualCompletion + completionIncrement;

    //qDebug() << "WOverview::drawNextPixmapPart() - nextCompletion:" << nextCompletion
    //         << "m_actualCompletion:" << m_actualCompletion
    //         << "waveformCompletion:" << waveformCompletion
    //         << "completionIncrement:" << completionIncrement;


    QPainter painter(m_pWaveformSourceImage);
    painter.translate(0.0,(double)m_pWaveformSourceImage->height()/2.0);

    QColor lowColor = m_signalColors.getLowColor();
    QPen lowColorPen(QBrush(lowColor), 1);

    QColor midColor = m_signalColors.getMidColor();
    QPen midColorPen(QBrush(midColor), 1);

    QColor highColor = m_signalColors.getHighColor();
    QPen highColorPen(QBrush(highColor), 1);

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        unsigned char lowNeg = m_pWaveform->getLow(currentCompletion);
        unsigned char lowPos = m_pWaveform->getLow(currentCompletion+1);
        if (lowPos || lowNeg) {
            painter.setPen(lowColorPen);
            painter.drawLine(QPoint(currentCompletion / 2, -lowNeg),
                             QPoint(currentCompletion / 2, lowPos));
        }
    }

    for (currentCompletion = m_actualCompletion; 
            currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen(midColorPen);
        painter.drawLine(QPoint(currentCompletion / 2, - m_pWaveform->getMid(currentCompletion)),
                         QPoint(currentCompletion / 2, m_pWaveform->getMid(currentCompletion+1)));
    }

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen(highColorPen);
        painter.drawLine(QPoint(currentCompletion / 2, - m_pWaveform->getHigh(currentCompletion)),
                         QPoint(currentCompletion / 2, m_pWaveform->getHigh(currentCompletion+1)));
    }

    //evaluate waveform ratio peak

    for (currentCompletion = m_actualCompletion;
            currentCompletion < nextCompletion; currentCompletion += 2) {
        m_waveformPeak = math_max(m_waveformPeak, (float)m_pWaveform->getAll(currentCompletion));
        m_waveformPeak = math_max(m_waveformPeak, (float)m_pWaveform->getAll(currentCompletion+1));
    }

    m_actualCompletion = nextCompletion;
    m_waveformImageScaled = QImage();
    m_diffGain = 0;

    //test if the complete waveform is done
    if (m_actualCompletion >= dataSize - 2) {
        m_pixmapDone = true;
        //qDebug() << "m_waveformPeakRatio" << m_waveformPeak;
    }

    m_pWaveform->getMutex()->unlock();
    return true;
}

void WOverview::mouseMoveEvent(QMouseEvent* e) {
    m_iPos = e->x();
    m_iPos = math_max(0, math_min(m_iPos,width() - 1));
    //qDebug() << "WOverview::mouseMoveEvent" << e->pos() << m_iPos;
    update();
}

void WOverview::mouseReleaseEvent(QMouseEvent* e) {
    mouseMoveEvent(e);

    float fValue = positionToValue(m_iPos);

    //qDebug() << "WOverview::mouseReleaseEvent" << e->pos() << m_iPos << ">>" << fValue;

    if (e->button() == Qt::RightButton) { 
        emit(valueChangedRightUp(fValue));
    } else {
        emit(valueChangedLeftUp(fValue));
    }
    m_bDrag = false;
}

void WOverview::mousePressEvent(QMouseEvent* e) {
    //qDebug() << "WOverview::mousePressEvent" << e->pos();
    mouseMoveEvent(e);
    m_bDrag = true;
}

void WOverview::paintEvent(QPaintEvent *) {
    ScopedTimer t("WOverview::paintEvent");

    QPainter painter(this);
    // Fill with transparent pixels
    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }

    //Display viewer contour if end of track
    if (m_endOfTrack) {
        painter.setOpacity(0.8);
        painter.setPen(QPen(QBrush(m_endOfTrackColor),1.5));
        painter.setBrush(QColor(0,0,0,0));
        painter.drawRect(rect().adjusted(0,0,-1,-1));
        painter.setOpacity(0.3);
        painter.setBrush(m_endOfTrackColor);
        painter.drawRect(rect().adjusted(1,1,-2,-2));
        painter.setOpacity(1);
    }

    //Draw waveform pixmap
    WaveformWidgetFactory* widgetFactory = WaveformWidgetFactory::instance();
    if (m_pWaveform) {
        // Draw Axis
        painter.setPen(QPen(m_signalColors.getAxesColor(), 1));
        painter.drawLine(0, height()/2, width(), height()/2);

        if (m_analyserProgress <= 50) { // remove text after progress by wf is recognizable (10 pixel)
            // We have a valid m_waveform, so here we have a track in analysis queue
            QColor lowColor = m_signalColors.getLowColor();
            lowColor.setAlphaF(0.5);
            QPen lowColorPen(QBrush(lowColor), 1.25, Qt::SolidLine, Qt::RoundCap);
            painter.setPen(lowColorPen);
            QString text;
            if (m_trackLoaded) {
                //: Text on waveform overview when file is cached from source                
                text = tr("Ready to play, analyzing ..");
            } else {
                //: Text on waveform overview when file is playable but no waveform is visible
                text = tr("Loading track ..");
            }
            QFont font = painter.font();
            QFontMetrics fm(font);
            int textWidth = fm.width(text);
            if (textWidth > width()) {
                qreal pointSize = font.pointSizeF();
                pointSize = pointSize * (width() - 5) / textWidth;
                if (pointSize < 6) {
                    pointSize = 6;
                }
                font.setPointSizeF(pointSize);
                painter.setFont(font);
            }
            painter.drawText(1, 12, text);
        }

        if (m_pWaveformSourceImage) {
            int diffGain;
            bool normalize = widgetFactory->isOverviewNormalized();
            if (normalize && m_pixmapDone && m_waveformPeak > 1) {
                diffGain = 255 - m_waveformPeak - 1;
            } else {
                const double visualGain = widgetFactory->getVisualGain(WaveformWidgetFactory::All);
                diffGain = 255.0 - 255.0 / visualGain;
            }

            if (m_diffGain != diffGain || m_waveformImageScaled.isNull() ) {
                QRect sourceRect(0, diffGain, m_pWaveformSourceImage->width(),
                    m_pWaveformSourceImage->height() - 2 * diffGain);
                m_waveformImageScaled = m_pWaveformSourceImage->copy(
                    sourceRect).scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                m_diffGain = diffGain;
            }

            painter.drawImage(rect(), m_waveformImageScaled);

            // Paint analyzer Progress
            if (m_analyserProgress < width()) {
                painter.setPen(QPen(m_signalColors.getAxesColor(), 3));
                painter.drawLine(m_analyserProgress, height()/2, width(), height()/2);
            }
        }
    }

    double trackSamples = m_trackSamplesControl->get();
    if (trackSamples > 0) {
        const float offset = 1.0f;
        const float gain = (float)(width()-2) / trackSamples;

        // Draw range (loop)
        for( unsigned int i = 0; i < m_markRanges.size(); i++) {
            WaveformMarkRange& currentMarkRange = m_markRanges[i];

            // If the mark range is not active we should not draw it.
            if (!currentMarkRange.active()) {
                continue;
            }

            // Active mark ranges by definition have starts/ends that are not
            // disabled.
            const double startValue = currentMarkRange.start();
            const double endValue = currentMarkRange.end();

            const float startPosition = offset + startValue * gain;
            const float endPosition = offset + endValue * gain;

            if (startPosition < 0.0 && endPosition < 0.0) {
                continue;
            }

            if (currentMarkRange.enabled()) {
                painter.setOpacity(0.4);
                painter.setPen(currentMarkRange.m_activeColor);
                painter.setBrush(currentMarkRange.m_activeColor);
            } else {
                painter.setOpacity(0.2);
                painter.setPen(currentMarkRange.m_disabledColor);
                painter.setBrush(currentMarkRange.m_disabledColor);
            }

            //let top and bottom of the rect out of the widget
            painter.drawRect(QRectF(QPointF(startPosition, -2.0),
                                    QPointF(endPosition,height() + 1.0)));
        }

        //Draw markers (Cue & hotcues)
        QPen shadowPen(QBrush(m_qColorBackground), 2.5);

        QFont markerFont = painter.font();
        markerFont.setPixelSize(10);

        QFont shadowFont = painter.font();
        shadowFont.setWeight(99);
        shadowFont.setPixelSize(10);

        painter.setOpacity(0.9);

        for( int i = 0; i < m_marks.size(); i++) {
            WaveformMark& currentMark = m_marks[i];
            if (currentMark.m_pointControl && currentMark.m_pointControl->get() >= 0.0) {
                //const float markPosition = 1.0 +
                //        (currentMark.m_pointControl->get() / (float)m_trackSamplesControl->get()) * (float)(width()-2);
                const float markPosition = offset + currentMark.m_pointControl->get() * gain;

                const QLineF line(markPosition, 0.0, markPosition, (float)height());
                painter.setPen(shadowPen);
                painter.drawLine(line);

                painter.setPen(currentMark.m_color);
                painter.drawLine(line);

                if (!currentMark.m_text.isEmpty()) {
                    QPointF textPoint;
                    textPoint.setX(markPosition+0.5f);

                    if( currentMark.m_align == Qt::AlignTop) {
                        QFontMetricsF metric(markerFont);
                        textPoint.setY(metric.tightBoundingRect(currentMark.m_text).height()+0.5f);
                    } else {
                        textPoint.setY(float(height())-0.5f);
                    }

                    painter.setPen(shadowPen);
                    painter.setFont(shadowFont);
                    painter.drawText(textPoint,currentMark.m_text);

                    painter.setPen(currentMark.m_textColor);
                    painter.setFont(markerFont);
                    painter.drawText(textPoint,currentMark.m_text);
                }
            }
        }

        //draw current position
        painter.setPen(QPen(QBrush(m_qColorBackground),1));
        painter.setOpacity(0.5);
        painter.drawLine(m_iPos + 1, 0, m_iPos + 1, height());
        painter.drawLine(m_iPos - 1, 0, m_iPos - 1, height());

        painter.setPen(QPen(m_signalColors.getPlayPosColor(),1));
        painter.setOpacity(1.0);
        painter.drawLine(m_iPos, 0, m_iPos, height());

        painter.drawLine(m_iPos - 2, 0, m_iPos, 2);
        painter.drawLine(m_iPos, 2, m_iPos + 2, 0);
        painter.drawLine(m_iPos - 2, 0, m_iPos + 2, 0);

        painter.drawLine(m_iPos - 2, height() - 1, m_iPos, height() - 3);
        painter.drawLine(m_iPos, height() - 3, m_iPos + 2, height() - 1);
        painter.drawLine(m_iPos - 2, height() - 1, m_iPos + 2, height() - 1);
    }
    painter.end();
}

void WOverview::resizeEvent(QResizeEvent *) {
    //Those coeficient map position from [0;width-1] to value [14;114]
    m_a = (float)((width()-1))/( 114.f - 14.f);
    m_b = 14.f * m_a;
    m_waveformImageScaled = QImage();
    m_diffGain = 0;
}

void WOverview::dragEnterEvent(QDragEnterEvent* event) {
    // Accept the enter event if the thing is a filepath and nothing's playing
    // in this deck or the settings allow to interrupt the playing deck.
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        if ((m_playControl->get() == 0.0 ||
            m_pConfig->getValueString(ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck")).toInt()) || !(m_pGroup=="[PreviewDeck1]")) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
}

void WOverview::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls() &&
            event->mimeData()->urls().size() > 0) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url = urls.first();
        QString name = url.toLocalFile();
        //If the file is on a network share, try just converting the URL to a string...
        if (name == "") {
            name = url.toString();
        }
        event->accept();
        emit(trackDropped(name, m_pGroup));
    } else {
        event->ignore();
    }
}
