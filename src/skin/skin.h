#pragma once

#include <QFileInfo>
#include <QList>
#include <QPixmap>
#include <QScreen>
#include <QString>
#include <memory>

namespace mixxx {
namespace skin {

enum class SkinType {
    Legacy,
};

class Skin {
  public:
    virtual ~Skin() = default;

    virtual SkinType type() const = 0;

    virtual bool isValid() const = 0;
    virtual QFileInfo path() const = 0;
    virtual QPixmap preview(const QString& schemeName) const = 0;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QList<QString> colorschemes() const = 0;

    virtual bool fitsScreenSize(const QScreen& screen) const = 0;
};

typedef std::shared_ptr<Skin> SkinPointer;

} // namespace skin
} // namespace mixxx
