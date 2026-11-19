#include "visualizer/visualizerlayout.h"

#include <QVarLengthArray>
#include <algorithm>
#include <cmath>

#include "util/assert.h"

namespace mixxx {

namespace {

using namespace std::chrono_literals;

// A deck joins the layout when its share exceeds the first value and leaves it
// when its share falls below the second one.
constexpr float kJoinShare = 0.02f;
constexpr float kLeaveShare = 0.01f;

// Between kJoinShare and this share the cell of a deck is smaller than its
// share, so that the cell of a deck that joins grows out of nothing.
constexpr float kFullCellShare = 0.08f;

// The summed loudness below which the mix counts as silent.
constexpr float kSilentLoudness = 0.002f;

// How fast the sites move to their places after a deck has joined or left.
constexpr std::chrono::duration<double> kSiteMoveTime = 350ms;

// The bonuses are adjusted in steps until the area of every cell is this close
// to its target. The step size is small enough for the adjustment to converge
// for up to four decks.
constexpr float kAreaTolerance = 2e-4f;
constexpr float kBonusStep = 0.2f;
constexpr int kMaxBonusSteps = 200;
constexpr float kJoinBonusOffset = 0.5f;

constexpr float kMinCellArea = 1e-5f;
constexpr float kNoBorderDistance = 1e6f;

using PerDeck = VisualizerLayout::PerDeck;
using Sites = std::array<QVector2D, kMaxNumberOfDecks>;
using Active = std::array<bool, kMaxNumberOfDecks>;

// The places of the sites for one to four audible decks, in the order of the
// decks. Three decks form a triangle around the center, so that their cells
// meet in the middle of the picture.
static_assert(kMaxNumberOfDecks == 4);
const std::array<Sites, kMaxNumberOfDecks> kSitePlaces = {
        Sites{QVector2D(0.0f, 0.0f)},
        Sites{QVector2D(-0.25f, 0.0f), QVector2D(0.25f, 0.0f)},
        Sites{QVector2D(-0.25f, -0.1443f),
                QVector2D(0.25f, -0.1443f),
                QVector2D(0.0f, 0.2887f)},
        Sites{QVector2D(-0.25f, -0.25f),
                QVector2D(0.25f, -0.25f),
                QVector2D(-0.25f, 0.25f),
                QVector2D(0.25f, 0.25f)},
};

// The corners of a cell. A cell is the picture cut by up to three borders, so
// it has seven corners at most.
using Polygon = QVarLengthArray<QVector2D, 8>;

// Returns the border between two decks seen from the first
// one: dot(xy, position) + z is positive where the first deck wins. A
// position belongs to the deck with the smallest squared distance to its
// site minus the bonus of the deck.
QVector3D borderBetween(const QVector2D& site,
        float bonus,
        const QVector2D& otherSite,
        float otherBonus) {
    const QVector2D direction = (site - otherSite) * 2.0f;
    const float offset = otherSite.lengthSquared() - site.lengthSquared() + bonus - otherBonus;
    return QVector3D(direction.x(), direction.y(), offset);
}

// Returns the part of the polygon in which dot(border.xy, position) + border.z
// is not negative.
Polygon clipped(const Polygon& polygon, const QVector3D& border) {
    Polygon result;
    const int count = static_cast<int>(polygon.size());
    for (int index = 0; index < count; ++index) {
        const QVector2D& current = polygon[index];
        const QVector2D& next = polygon[(index + 1) % count];
        const float currentDistance =
                QVector2D::dotProduct(border.toVector2D(), current) + border.z();
        const float nextDistance =
                QVector2D::dotProduct(border.toVector2D(), next) + border.z();
        if (currentDistance >= 0.0f) {
            result.append(current);
        }
        if ((currentDistance >= 0.0f) != (nextDistance >= 0.0f)) {
            const float t = currentDistance / (currentDistance - nextDistance);
            result.append(current + (next - current) * t);
        }
    }
    return result;
}

Polygon cellPolygon(int deckIndex,
        const Sites& sites,
        const PerDeck& bonuses,
        const Active& active) {
    Polygon polygon;
    polygon.append(QVector2D(-0.5f, -0.5f));
    polygon.append(QVector2D(0.5f, -0.5f));
    polygon.append(QVector2D(0.5f, 0.5f));
    polygon.append(QVector2D(-0.5f, 0.5f));
    for (int otherIndex = 0; otherIndex < kMaxNumberOfDecks; ++otherIndex) {
        if (otherIndex == deckIndex || !active[otherIndex]) {
            continue;
        }
        polygon = clipped(polygon,
                borderBetween(sites[deckIndex],
                        bonuses[deckIndex],
                        sites[otherIndex],
                        bonuses[otherIndex]));
    }
    return polygon;
}

float cross(const QVector2D& a, const QVector2D& b) {
    return a.x() * b.y() - a.y() * b.x();
}

// Returns twice the area of the polygon. The sign tells the orientation of its
// corners.
float doubleArea(const Polygon& polygon) {
    float sum = 0.0f;
    const int count = static_cast<int>(polygon.size());
    for (int index = 0; index < count; ++index) {
        sum += cross(polygon[index], polygon[(index + 1) % count]);
    }
    return sum;
}

// Returns the area that the cell of each deck shall have, as a fraction of the
// picture. It is the share of the deck, except for decks that have just joined.
PerDeck targetAreas(const PerDeck& shares, const Active& active) {
    PerDeck areas{};
    float sum = 0.0f;
    for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
        if (!active[deckIndex]) {
            continue;
        }
        const float t = std::clamp(
                (shares[deckIndex] - kJoinShare) / (kFullCellShare - kJoinShare),
                0.0f,
                1.0f);
        areas[deckIndex] = shares[deckIndex] * t * t * (3.0f - 2.0f * t);
        sum += areas[deckIndex];
    }
    if (sum > 0.0f) {
        for (float& area : areas) {
            area /= sum;
        }
    }
    return areas;
}

} // namespace

VisualizerLayout::VisualizerLayout()
        : m_cells(),
          m_sites(),
          m_bonuses(),
          m_active() {
}

// static
VisualizerLayout::PerDeck VisualizerLayout::updatedShares(
        const PerDeck& loudness, const PerDeck& previousShares) {
    float totalLoudness = 0.0f;
    for (float deckLoudness : loudness) {
        totalLoudness += std::max(deckLoudness, 0.0f);
    }
    if (totalLoudness < kSilentLoudness) {
        return previousShares;
    }
    PerDeck shares;
    for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
        shares[deckIndex] = std::max(loudness[deckIndex], 0.0f) / totalLoudness;
    }
    return shares;
}

void VisualizerLayout::update(
        const PerDeck& shares,
        std::chrono::duration<double> elapsed,
        const QSizeF& pictureSize) {
    VERIFY_OR_DEBUG_ASSERT(pictureSize.width() > 0 && pictureSize.height() > 0) {
        return;
    }
    const float pictureWidth = static_cast<float>(pictureSize.width());
    const float pictureHeight = static_cast<float>(pictureSize.height());

    // Give every deck in the layout its place. The site of a deck that joins is
    // put there at once and its bonus is set too low for a cell, so that
    // nothing changes visibly. The other sites move to their places gradually.
    float lowestBonus = 0.0f;
    for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
        if (m_active[deckIndex]) {
            lowestBonus = std::min(lowestBonus, m_bonuses[deckIndex]);
        }
    }
    Active joined{};
    int activeCount = 0;
    for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
        const bool wasActive = m_active[deckIndex];
        m_active[deckIndex] = shares[deckIndex] > (wasActive ? kLeaveShare : kJoinShare);
        joined[deckIndex] = m_active[deckIndex] && !wasActive;
        if (m_active[deckIndex]) {
            ++activeCount;
        }
    }
    const float moved = static_cast<float>(1.0 - std::exp(-(elapsed / kSiteMoveTime)));
    int place = 0;
    for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
        if (!m_active[deckIndex]) {
            continue;
        }
        const QVector2D& target = kSitePlaces[activeCount - 1][place];
        ++place;
        if (joined[deckIndex]) {
            m_sites[deckIndex] = target;
            m_bonuses[deckIndex] = lowestBonus - kJoinBonusOffset;
        } else {
            m_sites[deckIndex] += (target - m_sites[deckIndex]) * moved;
        }
    }

    // Raise the bonus of every deck whose cell is too small and lower it where
    // the cell is too large, until all cells have their target area. Starting
    // from the bonuses of the previous update, this takes only a few steps.
    const PerDeck targets = targetAreas(shares, m_active);
    for (int step = 0; step < kMaxBonusSteps; ++step) {
        float largestError = 0.0f;
        float bonusSum = 0.0f;
        for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
            if (!m_active[deckIndex]) {
                continue;
            }
            const float area = std::abs(doubleArea(
                                       cellPolygon(deckIndex, m_sites, m_bonuses, m_active))) /
                    2.0f;
            const float error = targets[deckIndex] - area;
            largestError = std::max(largestError, std::abs(error));
            m_bonuses[deckIndex] += kBonusStep * error;
            bonusSum += m_bonuses[deckIndex];
        }
        // Only the differences of the bonuses matter. Keeping their mean at
        // zero prevents them from drifting.
        for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
            if (m_active[deckIndex]) {
                m_bonuses[deckIndex] -= bonusSum / static_cast<float>(activeCount);
            }
        }
        if (largestError < kAreaTolerance) {
            break;
        }
    }

    for (int deckIndex = 0; deckIndex < kMaxNumberOfDecks; ++deckIndex) {
        Cell& cell = m_cells[deckIndex];
        cell.m_borders.fill(QVector3D(0.0f, 0.0f, kNoBorderDistance));
        cell.m_area = 0.0f;
        if (!m_active[deckIndex]) {
            continue;
        }

        // Scale the borders, so that they measure the distance in pixels.
        int borderIndex = 0;
        for (int otherIndex = 0; otherIndex < kMaxNumberOfDecks; ++otherIndex) {
            if (otherIndex == deckIndex || !m_active[otherIndex]) {
                continue;
            }
            const QVector3D border = borderBetween(m_sites[deckIndex],
                    m_bonuses[deckIndex],
                    m_sites[otherIndex],
                    m_bonuses[otherIndex]);
            const float slopePerPixel =
                    QVector2D(border.x() / pictureWidth, border.y() / pictureHeight).length();
            if (slopePerPixel > 0.0f) {
                cell.m_borders[borderIndex] = border / slopePerPixel;
            } else if (border.z() < 0.0f) {
                cell.m_borders[borderIndex] = QVector3D(0.0f, 0.0f, -kNoBorderDistance);
            }
            ++borderIndex;
        }

        const float area = std::abs(doubleArea(
                                   cellPolygon(deckIndex, m_sites, m_bonuses, m_active))) /
                2.0f;
        if (area >= kMinCellArea) {
            cell.m_area = area;
        }
    }
}

} // namespace mixxx
