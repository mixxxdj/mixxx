#pragma once

#include "util/types.h"

class SideChainWorker {
  public:
    SideChainWorker() { }
    virtual ~SideChainWorker() = default;
    virtual void process(const CSAMPLE* pBuffer, const std::size_t bufferSize) = 0;
    virtual void shutdown() = 0;
};
