#pragma once

#include <QObject>

#include "util/types.h"

struct GroupFeatureState;

class EngineObject : public QObject {
    Q_OBJECT
  public:
    EngineObject();
    ~EngineObject() override;
    virtual void process(CSAMPLE* pInOut,
            const std::size_t bufferSize) = 0;

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
    ~EngineObjectConstIn() override;

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOut, const std::size_t bufferSize) = 0;
};
