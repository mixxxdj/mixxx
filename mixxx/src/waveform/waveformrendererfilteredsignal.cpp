#include "waveformrendererfilteredsignal.h"

#include "waveformwidget.h"
#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "waveform.h"

#include <QLineF>

WaveformRendererFilteredSignal::WaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidget) :
    WaveformRendererAbstract( waveformWidget)
{
}

void WaveformRendererFilteredSignal::init()
{
}

void WaveformRendererFilteredSignal::onResize()
{
    qDebug() << "WaveformRendererFilteredSignal::onResize";
    m_lowLines.resize(2*m_waveformWidget->getWidth());
    m_midLines.resize(2*m_waveformWidget->getWidth());
    m_highLines.resize(2*m_waveformWidget->getWidth());
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

    int samplesPerPixel = 2;/*m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);*/
    int numberOfSamples = m_waveformWidget->getWidth() * samplesPerPixel;

    int currentPosition = 0;
    if( m_waveformWidget->getPlayPos() >= 0)
    {
        currentPosition = (int)( m_waveformWidget->getPlayPos()*waveformData.size());
        currentPosition -= (currentPosition%(2*samplesPerPixel));
    }

    painter->save();
    painter->translate(0.0,m_waveformWidget->getHeight()/2.0);
    painter->scale(1.0,(double)m_waveformWidget->getHeight()/255.0);

    for( int i = 0; i < numberOfSamples; i += samplesPerPixel)
    {
        int xPos = i/samplesPerPixel;
        int thisIndex = currentPosition + 2*i - numberOfSamples;
        if(thisIndex >= 0 && (thisIndex+1) < waveformData.size())
        {
            unsigned char maxLow[2] = {0,0};
            unsigned char maxBand[2] = {0,0};
            unsigned char maxHigh[2] = {0,0};

            for( int sampleIndex = 0; sampleIndex < samplesPerPixel; ++sampleIndex)
            {
                maxLow[0] = std::max( maxLow[0], waveform->getConstLowData()[thisIndex+sampleIndex]);
                maxLow[1] = std::max( maxLow[1], waveform->getConstLowData()[thisIndex+sampleIndex+1]);
                maxBand[0] = std::max( maxBand[0], waveform->getConstBandData()[thisIndex+sampleIndex]);
                maxBand[1] = std::max( maxBand[1], waveform->getConstBandData()[thisIndex+sampleIndex+1]);
                maxHigh[0] = std::max( maxHigh[0], waveform->getConstHighData()[thisIndex+sampleIndex]);
                maxHigh[1] = std::max( maxHigh[1], waveform->getConstHighData()[thisIndex+sampleIndex+1]);
            }

            m_lowLines[xPos].setLine( xPos, (float)-maxLow[0], xPos, (float)maxLow[1]);
            m_midLines[xPos].setLine( xPos, (float)-maxBand[0] * 2.0f, xPos, (float)maxBand[1] * 2.0f);
            m_highLines[xPos].setLine( xPos, (float)-maxHigh[0] * 4.0f, xPos, (float)maxHigh[1] * 4.0f);
        }
        else
        {
            m_lowLines[xPos].setLine( xPos, 0, xPos, 0);
            m_midLines[xPos].setLine( xPos, 0, xPos, 0);
            m_highLines[xPos].setLine( xPos, 0, xPos, 0);
        }
    }

    painter->setPen( QPen( QBrush(m_lowColor), 1));
    painter->drawLines( m_lowLines.data(), numberOfSamples);
    painter->setPen( QPen( QBrush(m_midColor), 1));
    painter->drawLines( m_midLines.data(), numberOfSamples);
    painter->setPen( QPen( QBrush(m_highColor), 1));
    painter->drawLines( m_highLines.data(), numberOfSamples);

    painter->restore();
}
