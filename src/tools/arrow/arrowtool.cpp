// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "arrowtool.h"
#include <cmath>

namespace {

#define PADDING_VALUE 2

const int ArrowWidth = 10;
const int ArrowHeight = 18;

QPainterPath getArrowHead(QPoint p1, QPoint p2, const int thickness)
{
    QLineF base(p1, p2);
    // Create the vector for the position of the base  of the arrowhead
    QLineF temp(QPoint(0, 0), p2 - p1);
    int val = ArrowHeight + thickness * 4;
    if (base.length() < val) {
        val = static_cast<int>(base.length() + thickness * 2);
    }
    temp.setLength(base.length() + thickness * 2 - val);
    // Move across the line up to the head
    QPointF bottomTranslation(temp.p2());

    // Rotate base of the arrowhead
    base.setLength(ArrowWidth + thickness * 2);
    base.setAngle(base.angle() + 90);
    // Move to the correct point
    QPointF temp2 = p1 - base.p2();
    // Center it
    QPointF centerTranslation((temp2.x() / 2), (temp2.y() / 2));

    base.translate(bottomTranslation);
    base.translate(centerTranslation);

    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(base.p1());
    path.lineTo(base.p2());
    path.lineTo(p2);
    return path;
}

// gets a shorter line to prevent overlap in the point of the arrow
QLine getShorterLine(QPoint p1, QPoint p2, const int thickness)
{
    QLineF l(p1, p2);
    int val = ArrowHeight + thickness * 4;
    if (l.length() < val) {
        val = static_cast<int>(l.length() + thickness * 2);
    }
    l.setLength(l.length() + thickness * 2 - val);
    return l.toLine();
}

} // unnamed namespace

ArrowTool::ArrowTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_padding = ArrowWidth / 2;
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon ArrowTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "arrow-bottom-left.svg");
}
QString ArrowTool::name() const
{
    return tr("Arrow");
}

ToolType ArrowTool::nameID() const
{
    return ToolType::ARROW;
}

QString ArrowTool::description() const
{
    return tr("Set the Arrow as the paint tool");
}

CaptureTool* ArrowTool::copy(QObject* parent)
{
    return new ArrowTool(parent);
}

void ArrowTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawLine(
      getShorterLine(m_points.first, m_points.second, m_thickness));
    m_arrowPath = getArrowHead(m_points.first, m_points.second, m_thickness);
    painter.fillPath(m_arrowPath, QBrush(m_color));
}

void ArrowTool::paintMousePreview(QPainter& painter,
                                  const CaptureContext& context)
{
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void ArrowTool::drawStart(const CaptureContext& context)
{
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void ArrowTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}

void ArrowTool::drawObjectSelection(QPainter& painter)
{
    int offset =
      m_thickness <= 1 ? 1 : static_cast<int>(round(m_thickness / 2 + 0.5));

    // get min and max arrow pos
    int min_x = m_points.first.x();
    int min_y = m_points.first.y();
    int max_x = m_points.first.x();
    int max_y = m_points.first.y();
    for (int i = 0; i < m_arrowPath.elementCount(); i++) {
        QPointF pt = m_arrowPath.elementAt(i);
        if (static_cast<int>(pt.x()) < min_x) {
            min_x = static_cast<int>(pt.x());
        }
        if (static_cast<int>(pt.y()) < min_y) {
            min_y = static_cast<int>(pt.y());
        }
        if (static_cast<int>(pt.x()) > max_x) {
            max_x = static_cast<int>(pt.x());
        }
        if (static_cast<int>(pt.y()) > max_y) {
            max_y = static_cast<int>(pt.y());
        }
    }

    // get min and max line pos
    int line_pos_min_x =
      std::min(std::min(m_points.first.x(), m_points.second.x()), min_x);
    int line_pos_min_y =
      std::min(std::min(m_points.first.y(), m_points.second.y()), min_y);
    int line_pos_max_x =
      std::max(std::max(m_points.first.x(), m_points.second.x()), max_x);
    int line_pos_max_y =
      std::max(std::max(m_points.first.y(), m_points.second.y()), max_y);

    QRect rect = QRect(line_pos_min_x - offset,
                       line_pos_min_y - offset,
                       line_pos_max_x - line_pos_min_x + offset * 2,
                       line_pos_max_y - line_pos_min_y + offset * 2);
    drawObjectSelectionRect(painter, rect);
}
