#include <QString>

namespace mixxx {

namespace filename {

/// Sanitize the base name of a file (without the extension).
/// Forbidden characters <>.:"'|?*/\ are replaced with -
/// unsanitizedName should not contain the file extension because
/// . characters are replaced with -
/// Forbidden Windows filenames are appended with -
/// This probably doesn't handle every invalid filename on every OS with every filesystem,
/// but hopefully it is good enough for the most common cases.
QString sanitize(const QString& unsanitizedName);

} // namespace filename

} // namespace mixxx
