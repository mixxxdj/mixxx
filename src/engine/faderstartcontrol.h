#pragma once

#include <QObject>
#include <QString>

#include "control/controlobject.h"
#include "util/configkey.h"

class FaderStartControl : public QObject {
    Q_OBJECT

public:
    explicit FaderStartControl(const QString& group);

private slots:
    void slotVolumeChanged(double value);

private:
    ControlObject* m_pPlay{nullptr};
    ControlObject* m_pVolume{nullptr};
};