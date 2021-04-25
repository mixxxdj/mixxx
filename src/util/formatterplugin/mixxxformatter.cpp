#include "mixxxformatter.h"

#include <grantlee/util.h>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QtDebug>
#include <QtMath>
#include <QtPlugin>
#include <cmath>

#include "moc_mixxxformatter.cpp"
#include "util/fileutils.h"

using namespace Grantlee;

namespace {
double kDefaultGroupSize = 10.0;
}

/// Groups numeric values into ranges of values (parameter is group size, default 10)
QVariant RangeGroup::doFilter(const QVariant& input,
        const QVariant& argument,
        bool autoescape) const {
    Q_UNUSED(autoescape);
    auto safeInput = getSafeString(input);

    bool ok = false;
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

    double modulus = getSafeString(argument).get().toDouble(&ok);

    if (ok) {
        if (modulus <= 0.0) {
            qWarning() << argument << "rangegroup filter group size is not a positive number";
            modulus = kDefaultGroupSize;
        }
    } else {
        modulus = kDefaultGroupSize;
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

QVariant ZeroPad::doFilter(const QVariant& input,
        const QVariant& argument,
        bool autoescape) const {
    Q_UNUSED(autoescape)
    auto value = getSafeString(input);

    bool ok;
    int iValue = value.get().toInt(&ok);
    if (!ok) {
        return QString();
    }
    int arg = getSafeString(argument).get().toInt(&ok);
    if (!ok) {
        arg = 2;
    }

    return SafeString(QString("%1").arg(iValue, arg, 10, QChar('0')));
}

QVariant Rounder::doFilter(const QVariant& input,
        const QVariant& argument,
        bool autoescape) const {
    Q_UNUSED(autoescape)
    auto value = getSafeString(input);

    bool ok;
    double dValue = value.get().toDouble(&ok);
    if (!ok) {
        return QString();
    }
    int arg = getSafeString(argument).get().toInt(&ok);
    if (!ok) {
        arg = 0;
    }

    double rValue = qRound(dValue * qPow(10, arg)) / qPow(10, arg);

    return SafeString(QString("%1").arg(rValue, 0, 'f', arg));
}

QVariant NoDir::doFilter(const QVariant& input,
        const QVariant& argument,
        bool autoescape) const {
    Q_UNUSED(autoescape)
    auto value = getSafeString(input);

    return SafeString(FileUtils::replaceDirChars(value.get()));
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
    filters.insert("zeropad", new ZeroPad());
    filters.insert("round", new Rounder());
    filters.insert("nodir", new NoDir());

    return filters;
}
