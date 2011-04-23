#include "glwaveformrendererfilteredsignal.h"

#include "waveformwidget.h"
#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "waveform.h"

#include <QLinearGradient>
#include <QLineF>

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidget) :
    WaveformRendererFilteredSignal( waveformWidget)
{
}

void GLWaveformRendererFilteredSignal::setup( const QDomNode& node)
{
    m_signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    //TODO vRince: fetch color from skin
    int h,s,l;
    m_signalColor.getHsl(&h,&s,&l);
    m_lowColor = QColor::fromHsl( h, s, 50, 120);
    m_midColor = QColor::fromHsl( h-5,s,100, 100);
    m_highColor =  QColor::fromHsl( h+5,s,200, 100);

    QLinearGradient gradientLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientLow.setColorAt(0.0,QColor::fromHsl( h, s, 70, 120));
    gradientLow.setColorAt(0.1,m_lowColor);
    gradientLow.setColorAt(0.5,m_lowColor);
    gradientLow.setColorAt(0.9,m_lowColor);
    gradientLow.setColorAt(1.0,QColor::fromHsl( h, s, 70, 120));
    m_lowBrush = QBrush(gradientLow);
    //m_lowBrush = QBrush(m_lowColor);

    /*QLinearGradient gradientMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientMid.setColorAt(0.0,m_highColor);
    gradientMid.setColorAt(0.2,m_midColor);
    gradientMid.setColorAt(0.5,m_midColor);
    gradientMid.setColorAt(0.8,m_midColor);
    gradientMid.setColorAt(1.0,m_highColor);
    m_midBrush = QBrush(gradientMid);*/
    m_midBrush = QBrush(m_midColor);

    /*QLinearGradient gradientHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientHigh.setColorAt(0.0,m_lowColor);
    gradientHigh.setColorAt(0.2,m_highColor);
    gradientHigh.setColorAt(0.5,m_highColor);
    gradientHigh.setColorAt(0.8,m_highColor);
    gradientHigh.setColorAt(1.0,m_lowColor);
    m_highBrush = QBrush(gradientHigh);*/
    m_highBrush = QBrush(m_highColor);
}

void GLWaveformRendererFilteredSignal::draw(QPainter *painter, QPaintEvent *event)
{
    const TrackInfoObject* trackInfo = m_waveformWidget->getTrackInfo().data();

    if( !trackInfo)
        return;

    const Waveform* waveform = trackInfo->getWaveForm();
    const QVector<unsigned char>& waveformData = waveform->getConstData();

    double samplesPerPixel = m_waveformWidget->getVisualSamplePerPixel();
    int numberOfSamples = m_waveformWidget->getWidth() * samplesPerPixel;

    int currentPosition = 0;
    if( m_waveformWidget->getPlayPos() >= 0)
    {
        currentPosition = m_waveformWidget->getPlayPos()*waveformData.size();
        currentPosition -= currentPosition%(2*(int)samplesPerPixel);
    }

    //m_lowLines.resize(numberOfSamples);
    //m_midLines.resize(numberOfSamples);
    //m_highLines.resize(numberOfSamples);

    painter->save();
    painter->translate(0.0,m_waveformWidget->getHeight()/2.0);
    painter->scale(1.0,(double)m_waveformWidget->getHeight()/255.0);

    QVector<QPointF> polygon[3];
    polygon[0].reserve(2*m_waveformWidget->getWidth()+3);
    polygon[1].reserve(2*m_waveformWidget->getWidth()+3);
    polygon[2].reserve(2*m_waveformWidget->getWidth()+3);

    int sampleOffset = 0;
    //vRince test in multi pass with no max comtupted (good looking, but disactivated need more work on perfs)
    //for( int sampleOffset = 0; sampleOffset < samplesPerPixel; ++sampleOffset)
    //{
        polygon[0].clear();
        polygon[1].clear();
        polygon[2].clear();

        polygon[0].push_back(QPointF(0.0,0.0));
        polygon[1].push_back(QPointF(0.0,0.0));
        polygon[2].push_back(QPointF(0.0,0.0));

        for( int i = sampleOffset; i < numberOfSamples; i += int(samplesPerPixel))
        {
            float xPos = (float)i/samplesPerPixel;
            int thisIndex = currentPosition + 2*i - numberOfSamples;
            if(thisIndex >= 0 && (thisIndex+samplesPerPixel+1) < waveformData.size())
                //vRince we coudl lost some data at the end but never more than samplesPerPixel+1 ...
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

                polygon[0].push_back(QPointF(xPos,(float)maxLow[0]));
                polygon[1].push_back(QPointF(xPos,(float)maxBand[0]));
                polygon[2].push_back(QPointF(xPos,(float)maxHigh[0]));
            }
            else
            {
                polygon[0].push_back(QPointF(xPos,0.0));
                polygon[1].push_back(QPointF(xPos,0.0));
                polygon[2].push_back(QPointF(xPos,0.0));
            }
        }

        polygon[0].push_back(QPointF(m_waveformWidget->getWidth(),0.0));
        polygon[1].push_back(QPointF(m_waveformWidget->getWidth(),0.0));
        polygon[2].push_back(QPointF(m_waveformWidget->getWidth(),0.0));

        for( int i = numberOfSamples - 1; i >= 0; i -= int(samplesPerPixel))
        {
            float xPos = (float)i/samplesPerPixel;
            int thisIndex = currentPosition + 2*i - numberOfSamples;
            if(thisIndex >= 0 && (thisIndex+samplesPerPixel+1) < waveformData.size())
            {
                unsigned char maxLow[2] = {0,0};
                unsigned char maxBand[2] = {0,0};
                unsigned char maxHigh[2] = {0,0};

                for( int sampleIndex = 0; sampleIndex < samplesPerPixel; ++sampleIndex)
                {
                    maxLow[0] = std::max( maxLow[0], waveform->getConstLowData()[thisIndex+sampleIndex+1]);
                    maxBand[0] = std::max( maxBand[0], waveform->getConstBandData()[thisIndex+sampleIndex+1]);
                    maxHigh[0] = std::max( maxHigh[0], waveform->getConstHighData()[thisIndex+sampleIndex+1]);
                }

                polygon[0].push_back(QPointF(xPos,-(float)maxLow[0]));
                polygon[1].push_back(QPointF(xPos,-(float)maxBand[0]));
                polygon[2].push_back(QPointF(xPos,-(float)maxHigh[0]));
            }
            else
            {
                polygon[0].push_back(QPointF(xPos,0.0));
                polygon[1].push_back(QPointF(xPos,0.0));
                polygon[2].push_back(QPointF(xPos,0.0));
            }
        }

        polygon[0].push_back(QPointF(0.0,0.0));
        polygon[1].push_back(QPointF(0.0,0.0));
        polygon[2].push_back(QPointF(0.0,0.0));

        painter->setPen( QPen( m_lowBrush, 0.0));
        painter->setBrush( m_lowBrush);
        painter->drawPolygon(polygon[0].data(),polygon[0].size());

        painter->setPen( QPen( m_midBrush, 0.0));
        painter->setBrush( m_midBrush);
        painter->drawPolygon(polygon[1].data(),polygon[1].size());

        painter->setPen( QPen( m_highBrush, 0.0));
        painter->setBrush( m_highBrush);
        painter->drawPolygon(polygon[2].data(),polygon[2].size());

    //}

    painter->restore();
}
