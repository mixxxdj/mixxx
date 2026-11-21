#pragma once

#include <QSizeF>
#include <QVector2D>
#include <QVector3D>
#include <array>
#include <chrono>

#include "util/defs.h"

namespace mixxx {

/// Divides a picture between the decks, for combining their covers into one.
/// Every audible deck owns one cell of the picture, and the area of the cell is
/// the share of the deck in the mix. One deck fills the picture, two decks
/// split it into a left and a right part, three decks into three wedges that
/// meet in the middle and four decks into four quarters. The cells have
/// straight borders that move continuously when the loudness of a deck changes
/// or when a deck joins or leaves the mix.
///
/// All positions are picture coordinates: the origin is the center of the
/// picture, x grows to the right, y grows downwards and the picture spans
/// -0.5 to 0.5 on both axes.
class VisualizerLayout {
  public:
    /// One value per deck.
    using PerDeck = std::array<float, kMaxNumberOfDecks>;

    /// The part of the picture that belongs to one deck.
    struct Cell {
        /// The borders to the cells of the other decks. For a position p,
        /// dot(xy, p) + z is the distance from the border in pixels, which is
        /// positive on the side of this cell. Unused entries are positive
        /// everywhere.
        std::array<QVector3D, kMaxNumberOfDecks - 1> m_borders;
        /// The area of the cell as a fraction of the picture. It is 0 if the
        /// deck has no cell.
        float m_area;
    };

    VisualizerLayout();

    /// Returns the share of each deck in the mix, 0..1, from the loudness of
    /// the decks. While all decks are silent the previous shares are returned,
    /// so that the picture stays as it was.
    static PerDeck updatedShares(const PerDeck& loudness, const PerDeck& previousShares);

    /// Recalculates the cells for the given shares and for a
    /// picture of the given size in pixels. The cells do not jump when a
    /// deck joins or leaves: the elapsed time limits how far they move
    /// towards their new places.
    void update(const PerDeck& shares,
            std::chrono::duration<double> elapsed,
            const QSizeF& pictureSize);

    const Cell& cell(int deckIndex) const {
        return m_cells[deckIndex];
    }

  private:
    std::array<Cell, kMaxNumberOfDecks> m_cells;
    std::array<QVector2D, kMaxNumberOfDecks> m_sites;
    PerDeck m_bonuses;
    std::array<bool, kMaxNumberOfDecks> m_active;
};

} // namespace mixxx
