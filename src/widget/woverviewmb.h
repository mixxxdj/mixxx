#ifndef WOVERVIEWMB_H
#define WOVERVIEWMB_H

#include <QColor>
#include "widget/woverview.h"

//MoodBar waveform overview for Mixxx

//Mood Bar is an alternative waveform overview representation in which
//Low,Mid and High bands are represented by RGB values of the color respectively.
//The alpha value of the color will indicate the RMS value of the band (can be used to guess Soundlevel)

class WOverviewMB : public WOverview {
  public:
    WOverviewMB(const char *pGroup, ConfigObject<ConfigValue>* pConfig, QWidget* parent);

  private:
    enum Range{
        High = 0,
        Mid,
        Low,
        All,
    };
    QColor detectColor(unsigned char high,unsigned char mid,unsigned char low);
    virtual bool drawNextPixmapPart();

};

#endif // WOVERVIEWMB_H
