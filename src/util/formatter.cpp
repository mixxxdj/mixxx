#include "util/formatter.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/outputstream.h>
#include <grantlee/template.h>

#include <QSharedPointer>
#include <QTextStream>

class NoEscapeStream : public Grantlee::OutputStream {
  public:
    NoEscapeStream()
            : Grantlee::OutputStream() {
    }
    NoEscapeStream(QTextStream* stream)
            : Grantlee::OutputStream(stream) {
    }
    ~NoEscapeStream() override{};

    QString escape(const QString& input) const override {
        return input;
    }
    QSharedPointer<Grantlee::OutputStream> clone(QTextStream* stream) const override {
        return QSharedPointer<NoEscapeStream>::create(stream);
    }
};

Grantlee::Engine* Formatter::getEngine(QObject* parent) {
    auto engine = new Grantlee::Engine(parent);
    // register custom
    return engine;
}

QString Formatter::renderNoEscape(Grantlee::Template& tmpl, Grantlee::Context& context) {
    auto rendered = QString();
    auto stream = QTextStream(&rendered);
    NoEscapeStream os(&stream);
    tmpl->render(&os, &context);
    return rendered;
}
