#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QVariant>

#include "library/columncache.h"
#include "qml/qml_owned_ptr.h"

namespace mixxx {
namespace qml {

class QmlLibraryTrackListColumn : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString label MEMBER m_label NOTIFY labelChanged)
    Q_PROPERTY(int fillSpan MEMBER m_fillSpan FINAL)
    Q_PROPERTY(int columnIdx MEMBER m_columnIdx FINAL)
    Q_PROPERTY(double preferredWidth MEMBER m_preferredWidth FINAL)
    Q_PROPERTY(double autoHideWidth MEMBER m_autoHideWidth NOTIFY autoHideWidthChanged)
    Q_PROPERTY(Display display MEMBER m_display NOTIFY displayChanged)
    Q_PROPERTY(Role role MEMBER m_role FINAL)
    Q_PROPERTY(ColumnType columnType MEMBER m_columnType FINAL)
    QML_NAMED_ELEMENT(TrackListColumn)
  public:
    enum class SQLColumns {
        Album = ColumnCache::COLUMN_LIBRARYTABLE_ALBUM,
        Artist = ColumnCache::COLUMN_LIBRARYTABLE_ARTIST,
        Title = ColumnCache::COLUMN_LIBRARYTABLE_TITLE,
        Year = ColumnCache::COLUMN_LIBRARYTABLE_YEAR,
        Bpm = ColumnCache::COLUMN_LIBRARYTABLE_BPM,
        Key = ColumnCache::COLUMN_LIBRARYTABLE_KEY,
        FileType = ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE,
        Bitrate = ColumnCache::COLUMN_LIBRARYTABLE_BITRATE,
    };
    Q_ENUM(SQLColumns)
    enum class Display {
        Auto,
        Show,
        Hide
    };
    Q_ENUM(Display)
    enum class Role {
        Location,
        Artist,
        Title,
        Cover,
    };
    Q_ENUM(Role)
    enum class ColumnType {
        Default,
        Overview,
        AlbumArt,
    };
    Q_ENUM(ColumnType)
    explicit QmlLibraryTrackListColumn(QObject* parent = nullptr)
            : QObject(parent) {
    }
    explicit QmlLibraryTrackListColumn(QObject* parent,
            const QString& label,
            int fillSpan,
            int columnIdx,
            double preferredWidth,
            double autoHideWidth,
            Role role,
            ColumnType columnType = ColumnType::Default,
            Display display = Display::Auto);
    const QString& label() const {
        return m_label;
    }
    Role role() const {
        return m_role;
    }
    ColumnType columnType() const {
        return m_columnType;
    }
    Display display() const {
        return m_display;
    }
    int fillSpan() const {
        return m_fillSpan;
    }
    int columnIdx() const {
        return m_columnIdx;
    }
    double preferredWidth() const {
        return m_preferredWidth;
    }
    double autoHideWidth() const {
        return m_autoHideWidth;
    }

  private:
    QString m_label;
    Role m_role;
    ColumnType m_columnType{ColumnType::Default};
    int m_fillSpan{0};
    int m_columnIdx{-1};
    Display m_display;
    double m_preferredWidth{-1};
    double m_autoHideWidth{-1};

  signals:
    void displayChanged();
    void labelChanged();
    void autoHideWidthChanged();
};
} // namespace qml
} // namespace mixxx
