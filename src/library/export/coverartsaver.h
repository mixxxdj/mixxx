#pragma once

#include <QImage>
#include <QString>

class CoverArtSaver {
  public:
    CoverArtSaver(const QString& fileName, const QImage& origCoverArt);
    ~CoverArtSaver() = default;

    bool saveCoverArt();

  private:
    QString m_fileName;
    QImage m_origCoverArt;
};
