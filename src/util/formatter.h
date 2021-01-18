#pragma once

#include <grantlee/engine.h>

#include <QObject>

class Formatter {
  public:
    static Grantlee::Engine* getEngine(QObject* parent);
};
