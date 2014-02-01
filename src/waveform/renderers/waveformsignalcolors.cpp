#include "waveformsignalcolors.h"

#include <QDomNode>

#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WaveformSignalColors::WaveformSignalColors()
{
}

bool WaveformSignalColors::setup(const QDomNode &node, const SkinContext& context)
{
/*
    QString string;
    QTextStream textStr(&string);
    node.save(textStr,4);
    qDebug() << string;
*/

    m_signalColor.setNamedColor(context.selectString(node, "SignalColor"));
    m_signalColor = WSkinColor::getCorrectColor(m_signalColor);

    m_lowColor.setNamedColor(context.selectString(node, "SignalLowColor"));
    m_lowColor = WSkinColor::getCorrectColor(m_lowColor);

    m_midColor.setNamedColor(context.selectString(node, "SignalMidColor"));
    m_midColor = WSkinColor::getCorrectColor(m_midColor);

    m_highColor.setNamedColor(context.selectString(node, "SignalHighColor"));
    m_highColor = WSkinColor::getCorrectColor(m_highColor);

    m_axesColor.setNamedColor(context.selectString(node, "AxesColor"));
    if (!m_axesColor.isValid()) {
        m_axesColor = QColor(245,245,245);
    }
    m_axesColor = WSkinColor::getCorrectColor(m_axesColor);

    m_playPosColor.setNamedColor(context.selectString(node, "PlayPosColor"));
    m_playPosColor = WSkinColor::getCorrectColor(m_playPosColor);
    if (!m_playPosColor.isValid()) {
        m_playPosColor = m_axesColor;
    }

    m_bgColor.setNamedColor(context.selectString(node, "BgColor"));
    if (!m_bgColor.isValid()) {
        m_bgColor = QColor(0, 0, 0);
    }
    m_bgColor = WSkinColor::getCorrectColor(m_bgColor);

    bool filteredColorValid = m_lowColor.isValid() && m_midColor.isValid() && m_highColor.isValid();

    if (m_signalColor.isValid() && filteredColorValid) {
        return true; //default
    }

    if (m_signalColor.isValid() && !filteredColorValid) {
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

    // NOTE(rryan): On ARM, qreal is float so it's important we use qreal here
    // and not double or float or else we will get build failures on ARM.
    qreal h, s, l, a;
    m_signalColor.getHslF(&h,&s,&l,&a);

    const double analogousAngle = 1.0/12.0;

    if( s < 0.1) // gray
    {
        const qreal sMax = 1.0 - h;
        m_lowColor.setHslF(h,s,l);
        m_midColor.setHslF(h,s+sMax*0.2,l);
        m_highColor.setHslF(h,s+sMax*0.4,l);
    }
    else
    {
        if( l < 0.1) // ~white
        {
            const qreal lMax = 1.0 - l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(h,s,l+lMax*0.2);
            m_highColor.setHslF(h,s,l+lMax*0.4);
        }
        else if( l < 0.5)
        {
            const qreal lMax = 1.0 - l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(stableHue(h-analogousAngle*0.3),s,l+lMax*0.1);
            m_highColor.setHslF(stableHue(h+analogousAngle*0.3),s,l+lMax*0.4);
        }
        else if ( l < 0.9)
        {
            const qreal lMin = l;
            m_lowColor.setHslF(h,s,l);
            m_midColor.setHslF(stableHue(h-analogousAngle*0.3),s,l-lMin*0.1);
            m_highColor.setHslF(stableHue(h+analogousAngle*0.3),s,l-lMin*0.4);
        }
        else // ~black
        {
            const qreal lMin = l;
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
