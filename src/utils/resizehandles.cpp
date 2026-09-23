// SPDX-License-Identifier: GPL-3.0-or-later

#include "resizehandles.h"

#include <utility>

namespace ResizeHandles {

namespace {

// Moves @p moving to @p to; when it lands on @p fixed, keeps it one pixel on
// the side it started from.
int placeEdge(int moving, int fixed, int to)
{
    if (to != fixed) {
        return to;
    }
    return moving < fixed ? fixed - 1 : fixed + 1;
}

}

QPoint handleCenter(const QRect& box, Handle handle)
{
    const QPoint c = box.center();
    switch (handle) {
        case TopLeft:
            return box.topLeft();
        case Top:
            return { c.x(), box.top() };
        case TopRight:
            return box.topRight();
        case Right:
            return { box.right(), c.y() };
        case BottomRight:
            return box.bottomRight();
        case Bottom:
            return { c.x(), box.bottom() };
        case BottomLeft:
            return box.bottomLeft();
        case Left:
            return { box.left(), c.y() };
        case None:
            break;
    }
    return c;
}

Handle handleAt(const QRect& box, const QPoint& pos, int tolerance)
{
    if (box.isEmpty()) {
        return None;
    }
    // Corners first: on a small box they overlap the edge handles.
    for (Handle h : { TopLeft,
                      TopRight,
                      BottomRight,
                      BottomLeft,
                      Top,
                      Right,
                      Bottom,
                      Left }) {
        const QPoint d = pos - handleCenter(box, h);
        if (QPoint::dotProduct(d, d) <= tolerance * tolerance) {
            return h;
        }
    }
    return None;
}

QRect resized(const QRect& start, Handle handle, const QPoint& pos)
{
    int left = start.left();
    int top = start.top();
    int right = start.right();
    int bottom = start.bottom();

    if (handle == TopLeft || handle == Left || handle == BottomLeft) {
        left = placeEdge(left, right, pos.x());
    }
    if (handle == TopRight || handle == Right || handle == BottomRight) {
        right = placeEdge(right, left, pos.x());
    }
    if (handle == TopLeft || handle == Top || handle == TopRight) {
        top = placeEdge(top, bottom, pos.y());
    }
    if (handle == BottomLeft || handle == Bottom || handle == BottomRight) {
        bottom = placeEdge(bottom, top, pos.y());
    }

    if (left > right) {
        std::swap(left, right);
    }
    if (top > bottom) {
        std::swap(top, bottom);
    }
    return { QPoint(left, top), QPoint(right, bottom) };
}

}
