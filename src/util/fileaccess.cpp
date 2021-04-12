#include "util/fileaccess.h"

namespace mixxx {

namespace {

inline SecurityTokenPointer acquireSecurityToken(
        SecurityTokenPointer pSecurityToken,
        const FileInfo& fileInfo) {
    if (pSecurityToken) {
        return pSecurityToken;
    } else {
        return Sandbox::openSecurityToken(fileInfo.asQFileInfo(), true);
    }
}

} // anonymous namespace

FileAccess::FileAccess(
        FileInfo fileInfo,
        SecurityTokenPointer pSecurityToken)
        : m_fileInfo(std::move(fileInfo)),
          m_pSecurityToken(
                  acquireSecurityToken(
                          std::move(pSecurityToken),
                          m_fileInfo)) {
}

} // namespace mixxx
