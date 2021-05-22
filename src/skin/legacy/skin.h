#pragma once

#include <QFileInfo>
#include <QList>
#include <QScreen>
#include <QString>

namespace mixxx {
namespace skin {
namespace legacy {

class Skin {
  public:
    Skin() = default;
    Skin(const QFileInfo& path);

    bool isValid() const;
    QFileInfo path() const;

    QString name() const;
    QString description() const;
    QList<QString> colorschemes() const;

    bool fitsScreenSize(const QScreen& screen) const;

  private:
    QFileInfo skinFile() const;

    QFileInfo m_path;
};

} // namespace legacy
} // namespace skin
} // namespace mixxx
