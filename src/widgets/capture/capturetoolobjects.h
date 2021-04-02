// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#ifndef FLAMESHOT_CAPTURETOOLOBJECTS_H
#define FLAMESHOT_CAPTURETOOLOBJECTS_H

#include "src/tools/capturetool.h"
#include <QList>
#include <QPointer>

class CaptureToolObjects : public QObject
{
public:
    explicit CaptureToolObjects(QObject* parent = nullptr){};
    QList<QPointer<CaptureTool>> captureToolObjects();
    void append(QPointer<CaptureTool> captureTool);
    QPointer<CaptureTool> at(int index);
    void removeAt(int index);
    void clear();
    int size();
    int find(const QPoint& pos, const QSize& captureSize);
    QPointer<CaptureTool> toolAt(int index);
    CaptureToolObjects& operator=(const CaptureToolObjects& other);

private:
    int findWithRadius(QPainter& painter,
                       QPixmap& pixmap,
                       const QPoint& pos,
                       int radius = 0);

    // class members
    QList<QPointer<CaptureTool>> m_captureToolObjects;
    QVector<QImage> m_imageCache;
};

#endif // FLAMESHOT_CAPTURETOOLOBJECTS_H
