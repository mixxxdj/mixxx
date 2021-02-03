#include "util/formatter.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/outputstream.h>
#include <grantlee/template.h>

#include <QSharedPointer>
#include <QTextStream>

#include "track/keys.h"
#include "util/fileutils.h"

NoEscapeStream::NoEscapeStream()
        : Grantlee::OutputStream() {
}
NoEscapeStream::NoEscapeStream(QTextStream* stream)
        : Grantlee::OutputStream(stream) {
}

NoEscapeStream::~NoEscapeStream(){};

QSharedPointer<Grantlee::OutputStream> NoEscapeStream::clone(QTextStream* stream) const {
    return QSharedPointer<NoEscapeStream>::create(stream);
}

FileEscapeStream::FileEscapeStream()
        : Grantlee::OutputStream() {
}
FileEscapeStream::FileEscapeStream(QTextStream* stream)
        : Grantlee::OutputStream(stream) {
}
FileEscapeStream::~FileEscapeStream(){};

QString FileEscapeStream::escape(const QString& input) const {
    return FileUtils::escapeFileName(input);
}

QSharedPointer<Grantlee::OutputStream> FileEscapeStream::clone(QTextStream* stream) const {
    return QSharedPointer<FileEscapeStream>::create(stream);
}

Grantlee::Engine* Formatter::getEngine(QObject* parent) {
    Grantlee::registerMetaType<Keys>();

    auto engine = new Grantlee::Engine(parent);
    engine->addDefaultLibrary(QStringLiteral("mixxxformatter"));
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

QString Formatter::renderFilenameEscape(Grantlee::Template& tmpl, Grantlee::Context& context) {
    auto rendered = QString();
    auto stream = QTextStream(&rendered);
    FileEscapeStream os(&stream);
    tmpl->render(&os, &context);
    return rendered;
}
