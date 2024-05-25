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
    /// Same as loadAllDirectories() just with paths as QString.
    /// See DlgPrefLibrary::populateDirList() for info.
    QStringList getRootDirStrings() const;

    enum class AddResult {
        Ok,
        AlreadyWatching,
        InvalidOrMissingDirectory,
        UnreadableDirectory,
        SqlError,
    };
    AddResult addDirectory(const mixxx::FileInfo& newDir) const;

    enum class RemoveResult {
        Ok,
        NotFound,
        SqlError,
    };
    RemoveResult removeDirectory(const mixxx::FileInfo& oldDir) const;

    enum class RelocateResult {
        Ok,
        InvalidOrMissingDirectory,
        UnreadableDirectory,
        SqlError,
    };
    // TODO: Move this function out of the DAO
    std::pair<RelocateResult, QList<RelocatedTrack>> relocateDirectory(
            const QString& oldDirectory,
            const QString& newDirectory) const;
};
