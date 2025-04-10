#pragma once

#include <QString>
#include <QVariant>

#include "effects/backends/effectmanifestparameter.h"
#include "util/class.h"

class EngineEffectParameter {
  public:
    EngineEffectParameter(EffectManifestParameterPointer pParameterManifest)
            : m_pParameterManifest(pParameterManifest) {
        m_value = m_pParameterManifest->getDefault();
    }
    virtual ~EngineEffectParameter() {
    }

    const QString& id() const {
        return m_pParameterManifest->id();
    }

    inline double value() const {
        return m_value;
    }
    inline void setValue(const double value) {
        // Values should be clamped by EffectParameter before sending to the engine.
        VERIFY_OR_DEBUG_ASSERT(
                value >= m_pParameterManifest->getMinimum() &&
                value <= m_pParameterManifest->getMaximum()) {
            return;
        }
        m_value = value;
    }
    inline int toInt() const {
        return static_cast<int>(m_value);
    }
    inline bool toBool() const {
        return m_value > 0.0;
    }

  private:
    EffectManifestParameterPointer m_pParameterManifest;
    double m_value;

    DISALLOW_COPY_AND_ASSIGN(EngineEffectParameter);
};
