#pragma once

#include <QList>
#include <QString>
#include <QByteArray>

#include "library/parser.h"
#include "library/basesqltablemodel.h"

class ParserCsv : public Parser {
    Q_OBJECT
  public:
    ~ParserCsv() override = default;

    QList<QString> parse(const QString&) override;
    static bool writeCSVFile(
            const QString& file,
            BaseSqlTableModel* pPlaylistTableModel,
            bool useRelativePath);
    static bool writeReadableTextFile(
            const QString& file,
            BaseSqlTableModel* pPlaylistTableModel,
            bool writeTimestamp);

  private:
    /// Reads a line from the file and returns filepath if a valid file
    QList<QList<QString>> tokenize(const QByteArray& str, char delimiter);
};
