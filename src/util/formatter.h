#pragma once

#include <grantlee/engine.h>
#include <grantlee/template.h>

#include <QObject>
#include <QString>

class Formatter {
  public:
    static Grantlee::Engine* getEngine(QObject* parent);
    // render template without escaping html characters
    static QString renderNoEscape(
            Grantlee::Template& templ,
            Grantlee::Context& context);
};
