#pragma once

#include <grantlee/engine.h>
#include <grantlee/template.h>

#include <QObject>
#include <QSharedPointer>
#include <QString>

class NoEscapeStream : public Grantlee::OutputStream {
  public:
    NoEscapeStream();
    NoEscapeStream(QTextStream* stream);
    ~NoEscapeStream() override;

    QString escape(const QString& input) const override {
        return input;
    };
    QSharedPointer<Grantlee::OutputStream> clone(QTextStream* stream) const override;
};

class FileEscapeStream : public Grantlee::OutputStream {
  public:
    FileEscapeStream();
    FileEscapeStream(QTextStream* stream);
    ~FileEscapeStream() override;

    QString escape(const QString& input) const override;
    QSharedPointer<Grantlee::OutputStream> clone(QTextStream* stream) const override;
};

class Formatter {
  public:
    static Grantlee::Engine* getEngine(QObject* parent);
    // render template without escaping html characters
    static QString renderNoEscape(
            Grantlee::Template& templ,
            Grantlee::Context& context);
    static QString renderFilenameEscape(
            Grantlee::Template& templ,
            Grantlee::Context& context);
};
