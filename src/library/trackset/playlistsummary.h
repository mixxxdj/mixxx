#pragma once

#include "util/duration.h"

class PlaylistSummary {
  public:
    explicit PlaylistSummary(int id = -1, QString label = nullptr)
            : m_id(id),
              m_name(label),
              m_count(0),
              m_duration(0),
              m_matches(0) {
    }
    ~PlaylistSummary() = default;

    int id() const {
        return m_id;
    }

    void setCount(int count) {
        m_count = count;
    }
    int count() const {
        return m_count;
    }

    void setDuration(int duration) {
        m_duration = duration;
    }
    int duration() const {
        return m_duration;
    }

    QString name() const {
        return m_name;
    }
    void setName(QString name) {
        m_name = name;
    }

    int matches() const {
        return m_matches;
    }
    void setMatches(int matches) {
        m_matches = matches;
    }

    QString getLabel() const {
        return createPlaylistLabel(
                m_name,
                m_count,
                m_duration);
    }

    static QString createPlaylistLabel(
            const QString& name,
            int count,
            int duration) {
        if (!count && !duration) {
            return QString(name);
        } else {
            return QStringLiteral("%1 (%2) %3")
                    .arg(name,
                            QString::number(count),
                            mixxx::Duration::formatTime(
                                    duration, mixxx::Duration::Precision::SECONDS));
        }
    }

  private:
    int m_id;
    QString m_name;
    int m_count;
    int m_duration;
    /// m_matches is used when querying track playlists for
    int m_matches;
};

Q_DECLARE_METATYPE(PlaylistSummary);
