#pragma once

#include "util/fileinfo.h"
#include "util/sandbox.h"

namespace mixxx {

/// Bundles `FileInfo` with the corresponding security token (if available).
/// On systems with sandboxing the latter is required to actually access the
/// corresponding file system object.
class FileAccess final {
  public:
    explicit FileAccess(
            FileInfo fileInfo,
            SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    FileAccess() = default;
    FileAccess(FileAccess&&) = default;
    FileAccess(const FileAccess&) = default;
    FileAccess& operator=(FileAccess&&) = default;
    FileAccess& operator=(const FileAccess&) = default;

    const FileInfo& info() const {
        return m_fileInfo;
    }

    const SecurityTokenPointer& token() const {
        return m_pSecurityToken;
    }

    bool isReadable() const {
        return m_pSecurityToken && m_fileInfo.isReadable();
    }
    bool isWritable() const {
        return m_pSecurityToken && m_fileInfo.isWritable();
    }

  private:
    FileInfo m_fileInfo;
    SecurityTokenPointer m_pSecurityToken;
};

} // namespace mixxx
