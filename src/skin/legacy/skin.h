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
    explicit Skin(const QFileInfo& path)
            : m_path(path) {
    }

    QFileInfo path() const {
        return m_path;
    }

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
