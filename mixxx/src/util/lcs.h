#include <cstring>

#include <QString>

// Returns the longest common substring shared between a and b. Does not support
// finding multiple longest common substrings.
inline QString LCS(const QString& a, const QString& b) {
    const size_t m = a.size();
    const size_t n = b.size();
    const size_t rows = m + 1;
    const size_t cols = n + 1;

    QVector<QVector<size_t> > M(rows);
    size_t longest = 0;
    size_t longest_loc = 0;

    for (size_t i = 0; i < rows; ++i) {
        M[i] = QVector<size_t>(cols, 0);
    }
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (a.at(i-1) == b.at(j-1)) {
                M[i][j] = M[i-1][j-1] + 1;
                if (M[i][j] > longest) {
                    longest = M[i][j];
                    longest_loc = i;
                }
            } else {
                M[i][j] = 0;
            }
        }
    }
    return a.mid(longest_loc - longest, longest);
}
