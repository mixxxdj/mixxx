// WTime is a widget showing the current time
// In skins it is represented by a <Time> node.

#pragma once

#include "widget/wlabel.h"

class WTime: public WLabel {
    Q_OBJECT
  public:
    explicit WTime(QWidget *parent=nullptr);
    ~WTime() override;

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void refreshTime();

  private:
    void setTimeFormat(const QDomNode& node, const SkinContext& context);

    QTimer* m_pTimer;
    QString m_sTimeFormat;
    // m_interval defines how often the time will be updated
    short m_interval;
    // m_interval is set to s_iSecondInterval if seconds are shown
    // otherwise, m_interval = s_iMinuteInterval
    static constexpr short s_iSecondInterval = 100;
    static constexpr short s_iMinuteInterval = 1000;
};
