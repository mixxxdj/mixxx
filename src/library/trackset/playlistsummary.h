#pragma once

#include <QObject>

#include "util/duration.h"

class PlaylistSummary {
  public:
    explicit PlaylistSummary(int id = -1, const QString& label = nullptr)
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
    bool isValid() const {
        return m_id != -1;
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
    void setName(const QString& name) {
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

class PlaylistSummaryWrapper : public QObject {
    Q_OBJECT
  public:
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(int id READ getId)

    Q_PROPERTY(uint trackCount READ getTrackCount WRITE setTrackCount)
    Q_PROPERTY(int trackDuration READ getTrackDuration WRITE setTrackDuration)
    Q_PROPERTY(int matches READ getMatches WRITE setMatches)

    PlaylistSummaryWrapper(PlaylistSummary& summary);
    ~PlaylistSummaryWrapper();

    int getId() const {
        return m_summary.id();
    }

    QString getName() const {
        return m_summary.name();
    };
    void setName(const QString& name) {
        m_summary.setName(name);
    };

    // The number of all tracks in this crate
    uint getTrackCount() const {
        return m_summary.count();
    }
    void setTrackCount(uint trackCount) {
        m_summary.setCount(trackCount);
    }

    // The total duration (in seconds) of all tracks in this crate
    int getTrackDuration() const {
        return m_summary.duration();
    }
    void setTrackDuration(int trackDuration) {
        m_summary.setDuration(trackDuration);
    }

    // The total duration (in seconds) of all tracks in this crate
    int16_t getMatches() const {
        return m_summary.matches();
    }
    void setMatches(int matches) {
        m_summary.setMatches(matches);
    }

  private:
    PlaylistSummary m_summary;
};
