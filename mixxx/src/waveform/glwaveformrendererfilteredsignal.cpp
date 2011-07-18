#include "glwaveformrendererfilteredsignal.h"

#include "waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "trackinfoobject.h"
#include "widget/wwidget.h"
#include "waveform.h"
#include "controlobject.h"

#include <QLinearGradient>
#include <QLineF>

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal( WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract( waveformWidgetRenderer)
{
    m_lowFilterControlObject = 0;
    m_midFilterControlObject = 0;
    m_highFilterControlObject = 0;
    m_lowKillControlObject = 0;
    m_midKillControlObject = 0;
    m_highKillControlObject = 0;
}

void GLWaveformRendererFilteredSignal::init()
{
    //create controls
    m_lowFilterControlObject = ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"filterLow"));
    m_midFilterControlObject = ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"filterMid"));
    m_highFilterControlObject = ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"filterHigh"));
    m_lowKillControlObject = ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"filterLowKill"));
    m_midKillControlObject = ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"filterMidKill"));
    m_highKillControlObject = ControlObject::getControl( ConfigKey(m_waveformWidget->getGroup(),"filterHighKill"));
}

void GLWaveformRendererFilteredSignal::setup( const QDomNode& node)
{
    m_signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    //TODO vRince: fetch color from skin
    int h,s,l;
    m_signalColor.getHsl(&h,&s,&l);

    QLinearGradient gradientLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientLow.setColorAt(0.0, QColor::fromHsl(h,s,60,120));
    gradientLow.setColorAt(0.25,QColor::fromHsl(h,s,50,120));
    gradientLow.setColorAt(0.5, QColor::fromHsl(h,s,30,120));
    gradientLow.setColorAt(0.75,QColor::fromHsl(h,s,50,120));
    gradientLow.setColorAt(1.0, QColor::fromHsl(h,s,60,120));
    m_lowBrush = QBrush(gradientLow);

    QLinearGradient gradientMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientMid.setColorAt(0.0, QColor::fromHsl(h-5,s,110,120));
    gradientMid.setColorAt(0.25,QColor::fromHsl(h-5,s,100,120));
    gradientMid.setColorAt(0.5, QColor::fromHsl(h-5,s, 80,120));
    gradientMid.setColorAt(0.75,QColor::fromHsl(h-5,s,100,120));
    gradientMid.setColorAt(1.0, QColor::fromHsl(h-5,s,100,120));
    m_midBrush = QBrush(gradientMid);

    QLinearGradient gradientHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientHigh.setColorAt(0.0, QColor::fromHsl(h+5,s,210,120));
    gradientHigh.setColorAt(0.25,QColor::fromHsl(h+5,s,200,120));
    gradientHigh.setColorAt(0.5, QColor::fromHsl(h+5,s,180,120));
    gradientHigh.setColorAt(0.75,QColor::fromHsl(h+5,s,200,120));
    gradientHigh.setColorAt(1.0, QColor::fromHsl(h+5,s,210,120));
    m_highBrush = QBrush(gradientHigh);

    QLinearGradient gradientKilledLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledLow.setColorAt(0.0, QColor::fromHsl(h,s,30,80));
    gradientKilledLow.setColorAt(0.5, QColor(200,200,200,10));
    gradientKilledLow.setColorAt(1.0, QColor::fromHsl(h,s,30,70));
    m_lowKilledBrush = QBrush(gradientKilledLow);

    QLinearGradient gradientKilledMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledMid.setColorAt(0.0, QColor::fromHsl(h-5,s, 80,80));
    gradientKilledMid.setColorAt(0.5, QColor(200,200,200,10));
    gradientKilledMid.setColorAt(1.0, QColor::fromHsl(h-5,s, 80,70));
    m_midKilledBrush = QBrush(gradientKilledMid);

    QLinearGradient gradientKilledHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledHigh.setColorAt(0.0, QColor::fromHsl(h+5,s,180,80));
    gradientKilledHigh.setColorAt(0.5, QColor(200,200,200,10));
    gradientKilledHigh.setColorAt(1.0, QColor::fromHsl(h+5,s,180,70));
    m_highKilledBrush = QBrush(gradientKilledHigh);
}

void GLWaveformRendererFilteredSignal::onResize()
{
    m_polygon[0].reserve(2*m_waveformWidget->getWidth()+3);
    m_polygon[1].reserve(2*m_waveformWidget->getWidth()+3);
    m_polygon[2].reserve(2*m_waveformWidget->getWidth()+3);
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
        m_waveformWidget->regulateVisualSample(currentPosition);
    }

    painter->save();

    painter->setRenderHint( QPainter::Antialiasing);

    painter->translate(0.0,m_waveformWidget->getHeight()/2.0);
    painter->scale(1.0,m_waveformWidget->getGain()*2.0*(double)m_waveformWidget->getHeight()/255.0);

    int sampleOffset = 0;
    //vRince test in multi pass with no max comtupted (good looking, but disactivated need more work on perfs)
    //for( int sampleOffset = 0; sampleOffset < samplesPerPixel; ++sampleOffset)
    //{
    m_polygon[0].clear();
    m_polygon[1].clear();
    m_polygon[2].clear();

    m_polygon[0].push_back(QPointF(0.0,0.0));
    m_polygon[1].push_back(QPointF(0.0,0.0));
    m_polygon[2].push_back(QPointF(0.0,0.0));

    for( int i = sampleOffset; i < numberOfSamples; i += int(samplesPerPixel))
    {
        float xPos = (float)i/samplesPerPixel;
        int thisIndex = currentPosition + 2*i - numberOfSamples;
        if(thisIndex >= 0 && (thisIndex+samplesPerPixel+1) < waveformData.size())
            //vRince we could lost some data at the end but never more than samplesPerPixel+1 ...
        {
            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for( int sampleIndex = 0; sampleIndex < (samplesPerPixel); ++sampleIndex)
            {
                maxLow = std::max( maxLow, waveform->getConstLowData()[thisIndex+sampleIndex]);
                maxBand = std::max( maxBand, waveform->getConstMidData()[thisIndex+sampleIndex]);
                maxHigh = std::max( maxHigh, waveform->getConstHighData()[thisIndex+sampleIndex]);
            }

            if( m_lowFilterControlObject && m_midFilterControlObject && m_highFilterControlObject)
            {
                m_polygon[0].push_back(QPointF(xPos,(float)maxLow*m_lowFilterControlObject->get()));
                m_polygon[1].push_back(QPointF(xPos,(float)maxBand*m_midFilterControlObject->get()));
                m_polygon[2].push_back(QPointF(xPos,(float)maxHigh*m_highFilterControlObject->get()));
            }
            else
            {
                //if for some reason we don't have controls
                m_polygon[0].push_back(QPointF(xPos,(float)maxLow));
                m_polygon[1].push_back(QPointF(xPos,(float)maxBand));
                m_polygon[2].push_back(QPointF(xPos,(float)maxHigh));
            }
        }
        else
        {
            m_polygon[0].push_back(QPointF(xPos,0.0));
            m_polygon[1].push_back(QPointF(xPos,0.0));
            m_polygon[2].push_back(QPointF(xPos,0.0));
        }
    }

    m_polygon[0].push_back(QPointF(m_waveformWidget->getWidth(),0.0));
    m_polygon[1].push_back(QPointF(m_waveformWidget->getWidth(),0.0));
    m_polygon[2].push_back(QPointF(m_waveformWidget->getWidth(),0.0));

    for( int i = numberOfSamples - 1; i >= 0; i -= int(samplesPerPixel))
    {
        float xPos = (float)i/samplesPerPixel;
        int thisIndex = currentPosition + 2*i - numberOfSamples + 1; //take left channel
        if(thisIndex >= 1 && (thisIndex+samplesPerPixel+1) < waveformData.size())
        {
            unsigned char maxLow = 0;
            unsigned char maxBand = 0;
            unsigned char maxHigh = 0;

            for( int sampleIndex = 0; sampleIndex < (samplesPerPixel); ++sampleIndex)
            {
                maxLow = std::max( maxLow, waveform->getConstLowData()[thisIndex+sampleIndex]);
                maxBand = std::max( maxBand, waveform->getConstMidData()[thisIndex+sampleIndex]);
                maxHigh = std::max( maxHigh, waveform->getConstHighData()[thisIndex+sampleIndex]);
            }

            if( m_lowFilterControlObject && m_midFilterControlObject && m_highFilterControlObject)
            {
                m_polygon[0].push_back(QPointF(xPos,-(float)maxLow*m_lowFilterControlObject->get()));
                m_polygon[1].push_back(QPointF(xPos,-(float)maxBand*m_midFilterControlObject->get()));
                m_polygon[2].push_back(QPointF(xPos,-(float)maxHigh*m_highFilterControlObject->get()));
            }
            else
            {
                //if for some reason we don't have controls
                m_polygon[0].push_back(QPointF(xPos,-(float)maxLow));
                m_polygon[1].push_back(QPointF(xPos,-(float)maxBand));
                m_polygon[2].push_back(QPointF(xPos,-(float)maxHigh));
            }
        }
        else
        {
            m_polygon[0].push_back(QPointF(xPos,0.0));
            m_polygon[1].push_back(QPointF(xPos,0.0));
            m_polygon[2].push_back(QPointF(xPos,0.0));
        }
    }

    m_polygon[0].push_back(QPointF(0.0,0.0));
    m_polygon[1].push_back(QPointF(0.0,0.0));
    m_polygon[2].push_back(QPointF(0.0,0.0));


    if( m_lowKillControlObject && m_lowKillControlObject->get() > 0.1)
    {
        painter->setPen( QPen( m_lowKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    }
    else
    {
        painter->setPen( QPen( m_lowBrush, 0.0));
        painter->setBrush( m_lowBrush);
    }
    painter->drawPolygon(m_polygon[0].data(),m_polygon[0].size());


    if( m_midKillControlObject && m_midKillControlObject->get() > 0.1)
    {
        painter->setPen( QPen( m_midKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    }
    else
    {
        painter->setPen( QPen( m_midBrush, 0.0));
        painter->setBrush( m_midBrush);
    }
    painter->drawPolygon(m_polygon[1].data(),m_polygon[1].size());

    if( m_highKillControlObject && m_highKillControlObject->get() > 0.1)
    {
        painter->setPen( QPen( m_highKilledBrush, 0.0));
        painter->setBrush(QColor(150,150,150,20));
    }
    else
    {
        painter->setPen( QPen( m_highBrush, 0.0));
        painter->setBrush( m_highBrush);
    }

    painter->drawPolygon(m_polygon[2].data(),m_polygon[2].size());

    //multipasstest
    //}

    painter->restore();
}
