// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
#include <QRect>

/// Geometry of the eight handles shown on a selected box-shaped object: four
/// corners and four edge midpoints. QtCore only, so it is unit-testable
/// headless.
namespace ResizeHandles {

enum Handle
{
    None,
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left
};

QPoint handleCenter(const QRect& box, Handle handle);

/// The handle within @p tolerance of @p pos, or None. Corners win over edges
/// when both are in reach, as on a box too small to tell them apart.
Handle handleAt(const QRect& box, const QPoint& pos, int tolerance);

/// @p start with the edges that @p handle controls moved to @p pos, the
/// opposite ones staying put. Dragging past the opposite side flips the box.
/// Opposite edges never coincide, so the result always spans two points.
QRect resized(const QRect& start, Handle handle, const QPoint& pos);

}
