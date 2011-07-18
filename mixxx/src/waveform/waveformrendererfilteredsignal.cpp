#include "waveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "waveform.h"

#include <QLineF>

WaveformRendererFilteredSignal::WaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract( waveformWidgetRenderer)
{
}

void WaveformRendererFilteredSignal::init()
{
}

void WaveformRendererFilteredSignal::onResize()
{
    qDebug() << "WaveformRendererFilteredSignal::onResize";
    m_lowLines.reserve(2*m_waveformWidget->getWidth());
    m_midLines.reserve(2*m_waveformWidget->getWidth());
    m_highLines.reserve(2*m_waveformWidget->getWidth());
}

void WaveformRendererFilteredSignal::setup( const QDomNode& node)
{
    m_signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    //TODO vRince: fetch color from skin
    int h,s,l;
    m_signalColor.getHsl(&h,&s,&l);
    m_lowColor = QColor::fromHsl( h, s, 50, 128);
    m_midColor = QColor::fromHsl( h-2,s,100, 128);
    m_highColor =  QColor::fromHsl( h+2,s,200, 128);
}

void WaveformRendererFilteredSignal::draw( QPainter* painter, QPaintEvent* /*event*/)
{
    const TrackInfoObject* trackInfo = m_waveformWidget->getTrackInfo().data();

    if( !trackInfo)
        return;

    const Waveform* waveform = trackInfo->getWaveForm();
    const QVector<unsigned char>& waveformData = waveform->getConstData();

    m_lowLines.clear();
    m_midLines.clear();
    m_highLines.clear();

    int samplesPerPixel = m_waveformWidget->getZoomFactor();/*m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);*/
    samplesPerPixel = std::min(2,samplesPerPixel);
    int numberOfSamples = m_waveformWidget->getWidth() * samplesPerPixel;

    int currentPosition = 0;
    if( m_waveformWidget->getPlayPos() >= 0)
    {
        currentPosition = (int)( m_waveformWidget->getPlayPos()*waveformData.size());
        currentPosition -= (currentPosition%(2*samplesPerPixel));
    }

    painter->save();
    painter->setWorldMatrixEnabled(false);

    float halfHeight = m_waveformWidget->getHeight()/2.0;
    float heightFactor = halfHeight/255.0;

    for( int i = 0; i < numberOfSamples; i += 2*samplesPerPixel)
    {
        int xPos = i/samplesPerPixel;
        int thisIndex = currentPosition + 2*i - numberOfSamples;
        if(thisIndex >= 0 && (thisIndex+1) < waveformData.size())
        {
            unsigned char maxLow[2] = {0,0};
            unsigned char maxBand[2] = {0,0};
            unsigned char maxHigh[2] = {0,0};

            for( int sampleIndex = 0; sampleIndex < 2*samplesPerPixel; ++sampleIndex)
            {
                maxLow[0] = std::max( maxLow[0], waveform->getConstLowData()[thisIndex+sampleIndex]);
                maxLow[1] = std::max( maxLow[1], waveform->getConstLowData()[thisIndex+sampleIndex+1]);
                maxBand[0] = std::max( maxBand[0], waveform->getConstMidData()[thisIndex+sampleIndex]);
                maxBand[1] = std::max( maxBand[1], waveform->getConstMidData()[thisIndex+sampleIndex+1]);
                maxHigh[0] = std::max( maxHigh[0], waveform->getConstHighData()[thisIndex+sampleIndex]);
                maxHigh[1] = std::max( maxHigh[1], waveform->getConstHighData()[thisIndex+sampleIndex+1]);
            }

            m_lowLines.push_back( QLine(xPos, (int)(halfHeight-heightFactor*(float)maxLow[0]), xPos, (int)(halfHeight+heightFactor*(float)maxLow[1])));
            m_midLines.push_back( QLine(xPos, (int)(halfHeight-heightFactor*(float)maxBand[0]), xPos, (int)(halfHeight+heightFactor*(float)maxBand[1])));
            m_highLines.push_back( QLine(xPos, (int)(halfHeight-heightFactor*(float)maxHigh[0]), xPos, (int)(halfHeight+heightFactor*(float)maxHigh[1])));
        }
        else
        {
            m_lowLines.push_back( QLine(xPos, 0, xPos, 0));
            m_midLines.push_back( QLine(xPos, 0, xPos, 0));
            m_highLines.push_back( QLine(xPos, 0, xPos, 0));
        }
    }

    painter->setPen( QPen( QBrush(m_lowColor), 2));
    painter->drawLines( m_lowLines.data(), m_lowLines.size());
    painter->setPen( QPen( QBrush(m_midColor), 2));
    painter->drawLines( m_midLines.data(), m_midLines.size());
    painter->setPen( QPen( QBrush(m_highColor), 2));
    painter->drawLines( m_highLines.data(), m_highLines.size());

    painter->restore();
}
