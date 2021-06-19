#pragma once

#include <QList>

#include "library/dao/dao.h"
#include "library/relocatedtrack.h"
#include "util/fileinfo.h"

class DirectoryDAO : public DAO {
  public:
    ~DirectoryDAO() override = default;

    QList<mixxx::FileInfo> loadAllDirectories(
            bool skipInvalidOrMissing = false) const;

    enum class AddResult {
        Ok,
        AlreadyWatching,
        InvalidOrMissingDirectory,
        SqlError,
    };
    AddResult addDirectory(const mixxx::FileInfo& newDir) const;

    enum class RemoveResult {
        Ok,
        NotFound,
        SqlError,
    };
    RemoveResult removeDirectory(const mixxx::FileInfo& oldDir) const;

    // TODO: Move this function out of the DAO
    QList<RelocatedTrack> relocateDirectory(
            const QString& oldFolder,
            const QString& newFolder) const;
};
