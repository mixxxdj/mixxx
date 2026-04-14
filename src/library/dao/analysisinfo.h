#pragma once

#include <QByteArray>
#include <QString>

#include "track/trackid.h"

// Standalone AnalysisType and AnalysisInfo, extracted from AnalysisDao so that
// waveform/ headers can reference them without pulling in the full DAO.

// Kept as an unscoped enum so that existing code using AnalysisDao::TYPE_WAVEFORM
// (integer binding to QSqlQuery, implicit int conversion) continues to work.
enum AnalysisType {
    TYPE_UNKNOWN = 0,
    TYPE_WAVEFORM,
    TYPE_WAVESUMMARY,
};

struct AnalysisInfo {
    AnalysisInfo()
            : analysisId(-1),
              type(TYPE_UNKNOWN) {
    }
    int analysisId;
    TrackId trackId;
    AnalysisType type;
    QString description;
    QString version;
    QByteArray data;
};
