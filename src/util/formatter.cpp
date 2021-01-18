#include "util/formatter.h"

#include <grantlee/engine.h>

Grantlee::Engine* Formatter::getEngine(QObject* parent) {
    auto engine = new Grantlee::Engine(parent);
    // register custom
    return engine;
}
