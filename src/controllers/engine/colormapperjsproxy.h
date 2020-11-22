#pragma once
#include <QJSValue>
#include <memory>

#include "controllers/engine/colormapper.h"

/// ColorMapperJSProxy is a wrapper class that exposes ColorMapper via the
/// QJSEngine and makes it possible to create and use ColorMapper object from
/// JavaScript controller mappings.
class ColorMapperJSProxy final : public QObject {
    Q_OBJECT
  public:
    // Passing a QMap<QRgb, QVariant> argument to the constructor as needed by
    // the ColorMapper constructor segfaults. QJSEngine converts a JS object to
    // a QVariantMap, so this constructor converts the QVariantMap to a
    // QMap<QRgb, QVariant>.
    Q_INVOKABLE ColorMapperJSProxy(const QVariantMap& availableColors);

    // Mixxx would segfault without this if a script calls "new ColorMapper()"
    // This should not actually be used.
    Q_INVOKABLE ColorMapperJSProxy();

    /// For a given RGB color code (e.g. 0xFF0000), this finds the nearest
    /// available color and returns a JS object with properties "red", "green",
    /// "blue" (each with value range 0-255).
    Q_INVOKABLE QVariantMap getNearestColor(uint ColorCode);

    /// For a given RGB color code (e.g. 0xFF0000), this finds the nearest
    /// available color, then returns the value associated with that color
    /// (which could be a MIDI byte value for example).
    Q_INVOKABLE QVariant getValueForNearestColor(uint ColorCode);

  private:
    std::unique_ptr<ColorMapper> m_pColorMapper;
};
