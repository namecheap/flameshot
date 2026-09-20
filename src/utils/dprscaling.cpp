// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprscaling.h"

namespace DprScaling {

Fit fitCrop(const QSize& cropped, const QSize& logical, qreal reportedDpr)
{
    if (logical.width() <= 0 || logical.height() <= 0) {
        return { cropped, reportedDpr };
    }

    // Qt rounds a compositor's scale up, never down, so this over-estimates the
    // monitor's real pixels rather than under-estimating them. Clamping to it
    // therefore cannot throw away pixels the panel could have shown.
    const int budgetWidth = qRound(logical.width() * reportedDpr);
    const int budgetHeight = qRound(logical.height() * reportedDpr);

    QSize size = cropped;
    if (cropped.width() > budgetWidth || cropped.height() > budgetHeight) {
        size = QSize(budgetWidth, budgetHeight);
    }

    return { size, qreal(size.width()) / logical.width() };
}

} // namespace DprScaling
