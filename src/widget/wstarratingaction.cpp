#include "widget/wstarratingaction.h"

#include <QHBoxLayout>

#include "moc_wstarratingaction.cpp"
#include "widget/wstarrating.h"

WStarRatingAction::WStarRatingAction(QWidget* parent)
        : QWidgetAction(parent),
          m_pStarRating(make_parented<WStarRating>(parent)) {
    m_pStarRating->setMouseTracking(true);
    // forward the signal
    connect(m_pStarRating,
            &WStarRating::ratingChangeRequest,
            this,
            &WStarRatingAction::ratingSet);

    QHBoxLayout* pLayout = new QHBoxLayout();
    pLayout->addWidget(m_pStarRating);
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* pWidget = new QWidget();
    pWidget->setLayout(pLayout);
    pWidget->setSizePolicy(QSizePolicy());
    setDefaultWidget(pWidget);
}

void WStarRatingAction::setRating(const int rating) {
    m_pStarRating->slotSetRating(rating);
}

QSize WStarRatingAction::sizeHint() {
    return m_pStarRating->sizeHint();
}
