#include "waveformsignalcolors.h"

#include "widget/wskincolor.h"
#include "widget/wwidget.h"

#include <QtXml/QDomNode>

WaveformSignalColors::WaveformSignalColors()
{
}

bool WaveformSignalColors::setup(const QDomNode &node)
{
/*
    QString string;
    QTextStream textStr(&string);
    node.save(textStr,4);
    qDebug() << string;
*/

    m_signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    m_lowColor.setNamedColor(WWidget::selectNodeQString(node, "SignalLowColor"));
    m_lowColor = WSkinColor::getCorrectColor(m_lowColor);

    m_midColor.setNamedColor(WWidget::selectNodeQString(node, "SignalMidColor"));
    m_midColor = WSkinColor::getCorrectColor(m_midColor);

    m_highColor.setNamedColor(WWidget::selectNodeQString(node, "SignalHighColor"));
    m_highColor = WSkinColor::getCorrectColor(m_highColor);

    bool filteredColorValid = m_lowColor.isValid() && m_midColor.isValid() && m_highColor.isValid();

    if( m_signalColor.isValid() && filteredColorValid)
        return true; //default

    if( m_signalColor.isValid() && !filteredColorValid)
    {
        fallBackFromSignalColor(); //pre waveform-2.0 skins
        return false;
    }

    fallBackDefaultColor();
    return false;
}

void WaveformSignalColors::fallBackFromSignalColor()
{
    qWarning() << "WaveformSignalColors::fallBackFromSignalColor - " \
                  "skin do not provide low/mid/high signal colors";

    double h,s,l,a;
    m_signalColor.getHslF(&h,&s,&l,&a);

    const double analogousAngle = 1.0/12.0;

    if( s < 0.1) // gray
    {
        const double sMax = 1.0 - h;
        m_lowColor.setHslF(h,s,l);
        m_midColor.setHslF(h,s+sMax*0.2,l);
        m_highColor.setHslF(h,s+sMax*0.4,l);
    }
    else
    {
        if( l < 0.1) // ~white
        {
            const double lMax = 1.0 - l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(h,s,l+lMax*0.2);
            m_highColor.setHslF(h,s,l+lMax*0.4);
        }
        else if( l < 0.5)
        {
            const double lMax = 1.0 - l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(stableHue(h-analogousAngle*0.3),s,l+lMax*0.1);
            m_highColor.setHslF(stableHue(h+analogousAngle*0.3),s,l+lMax*0.4);
        }
        else if ( l < 0.9)
        {
            const double lMin = l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(stableHue(h-analogousAngle*0.3),s,l-lMin*0.1);
            m_highColor.setHslF(stableHue(h+analogousAngle*0.3),s,l-lMin*0.4);
        }
        else // ~black
        {
            const double lMin = l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(h,s,l-lMin*0.2);
            m_highColor.setHslF(h,s,l-lMin*0.4);
        }
    }
}

void WaveformSignalColors::fallBackDefaultColor()
{
    qWarning() << "WaveformSignalColors::fallBackDefaultColor - " \
                  "skin do not provide valid signal colors ! Default colors is use ...";

    m_signalColor = Qt::green;
    fallBackFromSignalColor();
}

//NOTE(vRince) this sabilise hue between -1.0 and 2.0 but not more !
float WaveformSignalColors::stableHue( float hue) const
{
    return hue < 0.0 ? hue + 1.0 : hue > 1.0 ? hue - 1.0 : hue;
}
