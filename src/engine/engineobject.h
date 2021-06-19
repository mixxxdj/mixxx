#pragma once

#include <QObject>

#include "util/types.h"
#include "engine/effects/groupfeaturestate.h"

class EngineObject : public QObject {
    Q_OBJECT
  public:
    EngineObject();
    virtual ~EngineObject();
    virtual void process(CSAMPLE* pInOut,
                         const int iBufferSize) = 0;

    // Sub-classes re-implement and populate GroupFeatureState with the features
    // they extract.
    virtual void collectFeatures(GroupFeatureState* pGroupFeatures) const {
        Q_UNUSED(pGroupFeatures);
    }
};

class EngineObjectConstIn : public QObject {
    Q_OBJECT
  public:
    EngineObjectConstIn();
    virtual ~EngineObjectConstIn();

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOut,
                         const int iBufferSize) = 0;
};
