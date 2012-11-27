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

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

WOverview::WOverview(const char *pGroup, ConfigObject<ConfigValue>* pConfig, QWidget * parent)
    : WWidget(parent),
      m_pGroup(pGroup),
      m_pConfig(pConfig),
      m_analyserProgress(0) {
    m_iPos = 0;
    m_bDrag = false;

    m_totalGainControl = new ControlObjectThreadMain(
                ControlObject::getControl( ConfigKey(m_pGroup,"total_gain")));
    connect(m_totalGainControl, SIGNAL(valueChanged(double)),
             this, SLOT(onTotalGainChange(double)));
    m_totalGain = 1.0;

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

    m_waveform = NULL;
    m_waveformPixmap = QPixmap();
    m_actualCompletion = 0;
    m_visualSamplesByPixel = 0.0;
    m_waveformPeak = -1.0;
    m_pixmapDone = false;

    m_renderSampleLimit = 1000;

    m_a = 1.0;
    m_b = 0.0;
}

WOverview::~WOverview() {
    delete m_totalGainControl;
    delete m_endOfTrackControl;
    delete m_trackSamplesControl;
    delete m_playControl;
}

void WOverview::setup(QDomNode node) {
    // Background color and pixmap, default background color to transparent
    m_qColorBackground = QColor(0, 0, 0, 0);
    const QString bgColorName = WWidget::selectNodeQString(node, "BgColor");
    if (!bgColorName.isNull()) {
        m_qColorBackground.setNamedColor(bgColorName);
        m_qColorBackground = WSkinColor::getCorrectColor(m_qColorBackground);
    }

    // Clear the background pixmap, if it exists.
    m_backgroundPixmap = QPixmap();
    m_backgroundPixmapPath = WWidget::selectNodeQString(node, "BgPixmap");
    if (m_backgroundPixmapPath != "") {
        m_backgroundPixmap = QPixmap(WWidget::getPath(m_backgroundPixmapPath));
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

    m_signalColors.setup(node);

    m_qColorMarker.setNamedColor(selectNodeQString(node, "MarkerColor"));
    m_qColorMarker = WSkinColor::getCorrectColor(m_qColorMarker);

    //setup hotcues and cue and loop(s)
    m_marks.setup(m_pGroup,node);

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "MarkRange") {
            m_markRanges.push_back(WaveformMarkRange());
            WaveformMarkRange& markRange = m_markRanges.back();
            markRange.setup(m_pGroup, child);

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

    //init waveform pixmap
    //waveform pixmap twice the heigth of the viewport to be scalable by total_gain
    //NOTE: vrince we keep full vertical range waveform data to scale it on paint
    m_waveformPixmap = QPixmap(width(),2*255);
    m_waveformPixmap.fill( QColor(0,0,0,0));
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
    m_waveform = m_pCurrentTrack->getWaveformSummary();
    // If the waveform is already complete, just draw it.
    if (m_waveform && m_waveform->getCompletion() == m_waveform->getDataSize()) {
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
    m_analyserProgress = progress;
    // progress 0 .. 1000
    if (drawNextPixmapPart()) {
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

    m_actualCompletion = 0;
    m_visualSamplesByPixel = 0.0;
    m_waveformPixmap.fill(QColor(0, 0, 0, 0));
    m_waveformPeak = -1.0;
    m_pixmapDone = false;

    if (pTrack) {
        m_pCurrentTrack = pTrack;
        m_analyserProgress = pTrack->getAnalyserProgress();
        m_waveform = pTrack->getWaveformSummary();

        connect(pTrack.data(), SIGNAL(waveformSummaryUpdated()),
                this, SLOT(slotWaveformSummaryUpdated()));

        connect(pTrack.data(), SIGNAL(analyserProgress(int)),
                this, SLOT(slotAnalyserProgress(int)));
        //qDebug() << "WOverview::slotLoadNewTrack - startTimer";

        drawNextPixmapPart();
    }
    update();
}

void WOverview::slotUnloadTrack(TrackPointer /*pTrack*/) {
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.data(), SIGNAL(waveformSummaryUpdated()),
                   this, SLOT(slotWaveformSummaryUpdated()));
        disconnect(m_pCurrentTrack.data(), SIGNAL(analyserProgress(int)),
                   this, SLOT(slotAnalyserProgress(int)));
    }
    m_pCurrentTrack.clear();
    m_waveform = NULL;
    m_actualCompletion = 0;
    m_visualSamplesByPixel = 0.0;
    m_waveformPeak = -1.0;
    m_pixmapDone = false;

    update();
}

void WOverview::onTotalGainChange(double v) {
    //qDebug() << "WOverview::onTotalGainChange()" << v;
    m_totalGain = v;
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
    //qDebug() << "WOverview::drawNextPixmapPart() - m_waveform" << m_waveform;

    m_visualSamplesByPixel = static_cast<double>(m_waveform->getDataSize()) /
            static_cast<double>(width());

    if (!m_waveform || m_visualSamplesByPixel < 0.0001) {
        return false;
    }

    const int dataSize = m_waveform->getDataSize();
    const int analyserCompletion = (int)((float)dataSize * m_analyserProgress / 1000);
    const int waveformCompletion = m_waveform->getCompletion();

    // test if there is some new to draw (at least of pixel width)
    int completionIncrement;
    if (analyserCompletion < waveformCompletion) {
        // over all analyzer progress is slower than the pure waveform analysis
        completionIncrement = analyserCompletion - m_actualCompletion;
    } else {
        completionIncrement = waveformCompletion - m_actualCompletion;
    }
    if (dataSize == 0 || completionIncrement < m_visualSamplesByPixel) {
        return false;
    }

    if (!m_waveform->getMutex()->tryLock()) {
        return false;
    }

    //qDebug() << "WOverview::drawNextPixmapPart() - nextCompletion" << nextCompletion;
    //qDebug() << "WOverview::drawNextPixmapPart() - m_actualCompletion" << m_actualCompletion
    //         << "m_waveform->getCompletion()" << waveformCompletion
    //         << "nextCompletion" << completionIncrement;

    // TODO(rryan) was this limit for a reason?
    //completionIncrement = std::min(completionIncrement,m_renderSampleLimit);
    const int nextCompletion = m_actualCompletion + completionIncrement;

    QPainter painter(&m_waveformPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.translate(0.0,(double)m_waveformPixmap.height()/2.0);
    //painter.scale(1.0,(double)(m_waveformPixmap.height())/(2*255.0));

    //draw only the new part
    const float pixelStartPosition = 1.0 + (float)m_actualCompletion / (float)dataSize * (float)(width()-2);
    const float pixelByVisualSamples = 1.0 / m_visualSamplesByPixel;

    const float alpha = 0.5; /*= math_min( 1.0, 3.0*math_max( 0.1, pixelByVisualSamples));*/

    QColor lowColor = m_signalColors.getLowColor();
    lowColor.setAlphaF(alpha);
    QPen lowColorPen( QBrush(lowColor), 1.25, Qt::SolidLine, Qt::RoundCap);

    QColor midColor = m_signalColors.getMidColor();
    midColor.setAlphaF(alpha);
    QPen midColorPen( QBrush(midColor), 1.25, Qt::SolidLine, Qt::RoundCap);

    QColor highColor = m_signalColors.getHighColor();
    highColor.setAlphaF(alpha);
    QPen highColorPen( QBrush(highColor), 1.25, Qt::SolidLine, Qt::RoundCap);

    int currentCompletion = m_actualCompletion;
    float pixelPosition = pixelStartPosition;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen( lowColorPen);
        painter.drawLine( QPointF(pixelPosition, - m_waveform->getLow(currentCompletion+1)),
                          QPointF(pixelPosition, m_waveform->getLow(currentCompletion)));
        pixelPosition += 2.0*pixelByVisualSamples;
    }

    currentCompletion = m_actualCompletion;
    pixelPosition = pixelStartPosition;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen( midColorPen);
        painter.drawLine( QPointF(pixelPosition, - m_waveform->getMid(currentCompletion+1)),
                          QPointF(pixelPosition, m_waveform->getMid(currentCompletion)));
        pixelPosition += 2.0*pixelByVisualSamples;
    }

    currentCompletion = m_actualCompletion;
    pixelPosition = pixelStartPosition;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        painter.setPen( highColorPen);
        painter.drawLine( QPointF(pixelPosition, - m_waveform->getHigh(currentCompletion+1)),
                          QPointF(pixelPosition, m_waveform->getHigh(currentCompletion)));
        pixelPosition += 2.0*pixelByVisualSamples;
    }

    //evaluate waveform ratio peak
    currentCompletion = m_actualCompletion;
    for( ; currentCompletion < nextCompletion; currentCompletion += 2) {
        m_waveformPeak = math_max( m_waveformPeak, (float)m_waveform->getAll(currentCompletion+1));
        m_waveformPeak = math_max( m_waveformPeak, (float)m_waveform->getAll(currentCompletion));
    }

    m_actualCompletion = nextCompletion;

    //test if the complete waveform is done
    if( m_actualCompletion >= dataSize - 2) {
        m_pixmapDone = true;
        //qDebug() << "m_waveformPeakRatio" << m_waveformPeak;
    }

    m_waveform->getMutex()->unlock();
    return true;
}

void WOverview::mouseMoveEvent(QMouseEvent * e)
{
    m_iPos = e->x()-m_iStartMousePos;
    m_iPos = math_max(1,math_min(m_iPos,width()-1));

    //qDebug() << "WOverview::mouseMoveEvent" << e->pos() << m_iPos;

    update();
}

void WOverview::mouseReleaseEvent(QMouseEvent * e)
{
    mouseMoveEvent(e);

    float fValue = positionToValue(m_iPos);

    //qDebug() << "WOverview::mouseReleaseEvent" << e->pos() << m_iPos << ">>" << fValue;

    if (e->button()==Qt::RightButton)
        emit(valueChangedRightUp(fValue));
    else
        emit(valueChangedLeftUp(fValue));

    m_bDrag = false;
}

void WOverview::mousePressEvent(QMouseEvent * e)
{
    //qDebug() << "WOverview::mousePressEvent" << e->pos();

    m_iStartMousePos = 0;
    mouseMoveEvent(e);
    m_bDrag = true;
}

void WOverview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.resetTransform();
    // Fill with transparent pixels
    if( !m_backgroundPixmap.isNull())
        painter.drawPixmap(rect(), m_backgroundPixmap);

    //Display viewer contour if end of track
    if (m_endOfTrack) {
        painter.setOpacity(0.8);
        painter.setPen(QPen(QBrush(m_endOfTrackColor),1.5));
        painter.setBrush(QColor(0,0,0,0));
        painter.drawRect(rect().adjusted(0,0,-1,-1));
        painter.setOpacity(0.3);
        painter.setBrush(m_endOfTrackColor);
        painter.drawRect(rect().adjusted(1,1,-2,-2));
    }

    //Draw waveform pixmap
    WaveformWidgetFactory* widgetFactory = WaveformWidgetFactory::instance();
    if (m_waveform) {
        painter.setOpacity(1.0);

        bool normalize = widgetFactory->isOverviewNormalized();
        if( normalize && m_pixmapDone && m_waveformPeak > 1) {
            int diffPeak = 255 - m_waveformPeak - 1;
            QRect sourceRect( 0, diffPeak,
                              m_waveformPixmap.width(), m_waveformPixmap.height()-2*diffPeak);
            painter.drawPixmap(rect(), m_waveformPixmap, sourceRect);
        }
        else {
            const double visualGain = widgetFactory->getVisualGain(WaveformWidgetFactory::All);
            const int diffGain = 255.0 - 255.0/visualGain;
            QRect sourceRect( 0, diffGain,
                              m_waveformPixmap.width(), m_waveformPixmap.height()-2*diffGain);
            painter.drawPixmap(rect(), m_waveformPixmap, sourceRect);
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
                painter.setPen( shadowPen);
                painter.drawLine( line);

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
        painter.setPen(m_qColorMarker);
        painter.setOpacity(0.9);
        painter.drawLine(m_iPos,  0, m_iPos,  height());

        painter.drawLine(m_iPos-2,0,m_iPos,2);
        painter.drawLine(m_iPos,2,m_iPos+2,0);
        painter.drawLine(m_iPos-2,0,m_iPos+2,0);

        painter.drawLine(m_iPos-2,height()-1,m_iPos,height()-3);
        painter.drawLine(m_iPos,height()-3,m_iPos+2,height()-1);
        painter.drawLine(m_iPos-2,height()-1,m_iPos+2,height()-1);

        painter.setPen(m_qColorBackground);
        painter.setOpacity(0.5);
        painter.drawLine(m_iPos+1, 0, m_iPos+1, height());
        painter.drawLine(m_iPos-1, 0, m_iPos-1, height());
    }
    painter.end();
}

void WOverview::resizeEvent(QResizeEvent *) {
    //Those coeficient map position from [1;width-1] to value [14;114]
    m_a = float( (width()-1) - 1)/( 114.f - 14.f);
    m_b = 1 - 14.f*m_a;
}

QColor WOverview::getMarkerColor() {
    return m_qColorMarker;
}

void WOverview::dragEnterEvent(QDragEnterEvent* event) {
    // Accept the enter event if the thing is a filepath and nothing's playing
    // in this deck or the settings allow to interrupt the playing deck.
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        if (m_playControl->get() == 0.0 ||
            m_pConfig->getValueString(ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck")).toInt()) {
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
