#include <QString>
#include <QVector>
#include <cstring>

// Returns the longest common substring shared between a and b. Does not support
// finding multiple longest common substrings.
inline QString LCS(const QString& a, const QString& b) {
    const int m = a.size();
    const int n = b.size();
    const int rows = m + 1;
    const int cols = n + 1;

    QVector<QVector<int> > M(rows);
    int longest = 0;
    int longest_loc = 0;

    for (int i = 0; i < rows; ++i) {
        M[i] = QVector<int>(cols, 0);
    }
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
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
