#include "mixxxformatter.h"

#include <grantlee/util.h>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QtPlugin>
#include <cmath>

#include "moc_mixxxformatter.cpp"

using namespace Grantlee;

namespace {
double kDefaultGroupSize = 10.0;
}

/// Groups numeric values into ranges of values (parameter is group size, default 10)

QVariant RangeGroup::doFilter(const QVariant& input,
        const QVariant& argument,
        bool autoescape) const {
    bool ok = false;
    SafeString safeInput = qvariant_cast<SafeString>(input);
    double finput = 0.0;
    if (!safeInput.get().isNull()) {
        finput = safeInput.get().toDouble(&ok);
        if (!ok) {
            qWarning() << input << "rangegroup filter input is not a number";
            return input;
        }
    } else {
        finput = input.toDouble(&ok);
        if (!ok) {
            qWarning() << input << "rangegroup filter input is not a number";
            return input;
        }
    }
    double modulus = kDefaultGroupSize;
    // FIXME(XXX) why is the argument always a QVariant(Invalid) ?
    //qDebug() << "arg" << argument << getSafeString(argument).get();
    if (!argument.isNull()) {
        SafeString safeModulus = qvariant_cast<SafeString>(argument);
        double modulusInput = kDefaultGroupSize;
        if (!safeModulus.get().isNull()) {
            modulusInput = safeModulus.get().toDouble(&ok);
            if (!ok) {
                qWarning() << argument << "rangegroup filter group size is not a number";
            }
        } else {
            modulusInput = argument.toDouble(&ok);
            if (!ok) {
                qWarning() << argument << "rangegroup filter group size is not a number";
            }
        }
        if (modulus <= 0.0) {
            qWarning() << argument << "rangegroup filter group size is not a positive number";
            modulus = kDefaultGroupSize;
        }
    }

    double rest = std::fmod(finput, modulus);
    double groupStart = finput - rest;
    double groupEnd = groupStart + modulus;
    QString startString;
    QString endString;
    if (abs(groupStart - (qRound(groupStart))) < 0.01) {
        startString = QString::number(static_cast<int>(groupStart));
    } else {
        startString = QString("%1").arg(groupStart, 0, 'f', 2);
    }
    if (abs(groupEnd - (qRound(groupEnd))) < 0.01) {
        endString = QString::number(static_cast<int>(groupEnd));
    } else {
        endString = QString("%1").arg(groupEnd, 0, 'f', 2);
    }

    return QVariant(startString + QStringLiteral("-") + endString);
}

QHash<QString, AbstractNodeFactory*> FormatterPlugin::nodeFactories(const QString& name) {
    Q_UNUSED(name);
    QHash<QString, AbstractNodeFactory*> nodes;
    return nodes;
}

QHash<QString, Filter*> FormatterPlugin::filters(const QString& name) {
    Q_UNUSED(name);
    QHash<QString, Filter*> filters;

    filters.insert("rangegroup", new RangeGroup());

    return filters;
}
