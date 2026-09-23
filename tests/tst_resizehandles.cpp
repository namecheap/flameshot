// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/resizehandles.h"

#include <QTest>

using ResizeHandles::Handle;

Q_DECLARE_METATYPE(ResizeHandles::Handle)

namespace {
// left 100, top 100, right 300, bottom 200
const QRect BOX(QPoint(100, 100), QPoint(300, 200));
const int TOLERANCE = 8;
}

class TestResizeHandles : public QObject
{
    Q_OBJECT

private slots:
    void handleAt_data();
    void handleAt();
    void handleAt_tinyBoxPrefersCorner();
    void handleAt_emptyBoxHasNoHandles();

    void resized_data();
    void resized();
};

void TestResizeHandles::handleAt_data()
{
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<Handle>("expected");

    QTest::newRow("top-left") << QPoint(100, 100) << ResizeHandles::TopLeft;
    QTest::newRow("top") << QPoint(200, 100) << ResizeHandles::Top;
    QTest::newRow("top-right") << QPoint(300, 100) << ResizeHandles::TopRight;
    QTest::newRow("right") << QPoint(300, 150) << ResizeHandles::Right;
    QTest::newRow("bottom-right")
      << QPoint(300, 200) << ResizeHandles::BottomRight;
    QTest::newRow("bottom") << QPoint(200, 200) << ResizeHandles::Bottom;
    QTest::newRow("bottom-left")
      << QPoint(100, 200) << ResizeHandles::BottomLeft;
    QTest::newRow("left") << QPoint(100, 150) << ResizeHandles::Left;

    QTest::newRow("within tolerance")
      << QPoint(105, 94) << ResizeHandles::TopLeft;
    QTest::newRow("just outside tolerance")
      << QPoint(100, 91) << ResizeHandles::None;
    QTest::newRow("interior") << QPoint(150, 130) << ResizeHandles::None;
    QTest::newRow("on an edge between handles")
      << QPoint(150, 100) << ResizeHandles::None;
}

void TestResizeHandles::handleAt()
{
    QFETCH(QPoint, pos);
    QFETCH(Handle, expected);

    QCOMPARE(ResizeHandles::handleAt(BOX, pos, TOLERANCE), expected);
}

void TestResizeHandles::handleAt_tinyBoxPrefersCorner()
{
    // Every handle is within reach; a corner resizes both axes, so it wins.
    const QRect tiny(QPoint(100, 100), QPoint(102, 102));

    QCOMPARE(ResizeHandles::handleAt(tiny, QPoint(101, 101), TOLERANCE),
             ResizeHandles::TopLeft);
}

void TestResizeHandles::handleAt_emptyBoxHasNoHandles()
{
    QCOMPARE(ResizeHandles::handleAt(QRect(), QPoint(0, 0), TOLERANCE),
             ResizeHandles::None);
}

void TestResizeHandles::resized_data()
{
    QTest::addColumn<Handle>("handle");
    QTest::addColumn<QPoint>("pos");
    QTest::addColumn<QRect>("expected");

    QTest::newRow("top-left moves its corner")
      << ResizeHandles::TopLeft << QPoint(50, 40)
      << QRect(QPoint(50, 40), QPoint(300, 200));
    QTest::newRow("top-right moves its corner")
      << ResizeHandles::TopRight << QPoint(320, 90)
      << QRect(QPoint(100, 90), QPoint(320, 200));
    QTest::newRow("bottom-right moves its corner")
      << ResizeHandles::BottomRight << QPoint(350, 260)
      << QRect(QPoint(100, 100), QPoint(350, 260));
    QTest::newRow("bottom-left moves its corner")
      << ResizeHandles::BottomLeft << QPoint(90, 210)
      << QRect(QPoint(90, 100), QPoint(300, 210));

    QTest::newRow("top ignores horizontal motion")
      << ResizeHandles::Top << QPoint(999, 80)
      << QRect(QPoint(100, 80), QPoint(300, 200));
    QTest::newRow("right ignores vertical motion")
      << ResizeHandles::Right << QPoint(350, 999)
      << QRect(QPoint(100, 100), QPoint(350, 200));
    QTest::newRow("bottom ignores horizontal motion")
      << ResizeHandles::Bottom << QPoint(0, 250)
      << QRect(QPoint(100, 100), QPoint(300, 250));
    QTest::newRow("left ignores vertical motion")
      << ResizeHandles::Left << QPoint(120, 0)
      << QRect(QPoint(120, 100), QPoint(300, 200));

    QTest::newRow("edge dragged past the opposite side flips")
      << ResizeHandles::Right << QPoint(40, 150)
      << QRect(QPoint(40, 100), QPoint(100, 200));
    QTest::newRow("corner dragged past the opposite corner flips")
      << ResizeHandles::BottomRight << QPoint(50, 50)
      << QRect(QPoint(50, 50), QPoint(100, 100));

    // Coinciding edges would leave the object's two points equal on that
    // axis; both equal makes it invalid and impossible to select again.
    QTest::newRow("edge onto the opposite side keeps one pixel")
      << ResizeHandles::Right << QPoint(100, 150)
      << QRect(QPoint(100, 100), QPoint(101, 200));
    QTest::newRow("corner onto the opposite corner keeps one pixel")
      << ResizeHandles::BottomRight << QPoint(100, 100)
      << QRect(QPoint(100, 100), QPoint(101, 101));
    QTest::newRow("top onto the bottom keeps one pixel")
      << ResizeHandles::Top << QPoint(0, 200)
      << QRect(QPoint(100, 199), QPoint(300, 200));

    QTest::newRow("no handle leaves the box alone")
      << ResizeHandles::None << QPoint(0, 0) << BOX;
}

void TestResizeHandles::resized()
{
    QFETCH(Handle, handle);
    QFETCH(QPoint, pos);
    QFETCH(QRect, expected);

    QCOMPARE(ResizeHandles::resized(BOX, handle, pos), expected);
}

QTEST_APPLESS_MAIN(TestResizeHandles)
#include "tst_resizehandles.moc"
