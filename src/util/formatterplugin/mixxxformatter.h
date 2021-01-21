#pragma once

#include <grantlee/filter.h>
#include <grantlee/safestring.h>
#include <grantlee/taglibraryinterface.h>

#include <QObject>
#include <QString>
#include <QVariant>

using namespace Grantlee;

/// Groups numeric values into ranges of values (parameter is group size, default 10)
class RangeGroup : public Filter {
  public:
    RangeGroup(){};
    ~RangeGroup(){};

    QVariant doFilter(const QVariant& input,
            const QVariant& arg = {},
            bool autoescape = false) const override;
    bool isSafe() const override {
        return true;
    }; // see the Autoescaping section
};

class FormatterPlugin : public QObject, public TagLibraryInterface {
    Q_OBJECT
    Q_INTERFACES(Grantlee::TagLibraryInterface)
    Q_PLUGIN_METADATA(IID "org.grantlee.TagLibraryInterface")
  public:
    FormatterPlugin(QObject* parent = nullptr)
            : QObject(parent){};

    QHash<QString, AbstractNodeFactory*> nodeFactories(const QString& name = QString());
    QHash<QString, Filter*> filters(const QString& name = QString());
};
