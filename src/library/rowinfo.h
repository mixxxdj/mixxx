// rowinfo.h
#pragma once
#include <QColor>
#include <QSqlRecord>
#include <QString>

class RowInfo {
  public:
    RowInfo()
            : m_trackId(-1) {
    }

    const QString& getLabel() const {
        return m_label;
    }

    void setLabel(const QString& label) {
        m_label = label;
    }

    const QColor& getColor() const {
        return m_color;
    }

    void setColor(const QColor& color) {
        m_color = color;
    }

    int getTrackId() const {
        return m_trackId;
    }

    void setTrackId(int trackId) {
        m_trackId = trackId;
    }

    const QSqlRecord& getSqlRecord() const {
        return m_sqlRecord;
    }

    void setSqlRecord(const QSqlRecord& sqlRecord) {
        m_sqlRecord = sqlRecord;
    }

  private:
    QString m_label;
    QColor m_color;
    int m_trackId;
    QSqlRecord m_sqlRecord;
};
