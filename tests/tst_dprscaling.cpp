// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/dprscaling.h"

#include <QTest>

using DprScaling::Fit;

class TestDprScaling : public QObject
{
    Q_OBJECT

private slots:
    void fitCrop_data();
    void fitCrop();
    void fitCrop_neverEnlarges();
    void fitCrop_degenerateLogicalSize();
};

void TestDprScaling::fitCrop_data()
{
    QTest::addColumn<QSize>("cropped");
    QTest::addColumn<QSize>("logical");
    QTest::addColumn<qreal>("reportedDpr");
    QTest::addColumn<QSize>("expectedSize");
    QTest::addColumn<qreal>("expectedDpr");

    // Measured on a real mixed-scale setup. The portal composites the whole
    // desktop at one uniform scale (1.5), so each monitor's crop arrives at
    // that scale regardless of its own.

    // DP-1: a 3840x2160 panel driven at 150%, which Qt reports as DPR 2. The
    // crop is already the panel's native pixels; enlarging it to 5120x2880 to
    // match the reported ratio would be pure invention.
    QTest::newRow("fractional scale keeps native pixels")
      << QSize(3840, 2160) << QSize(2560, 1440) << qreal(2.0)
      << QSize(3840, 2160) << qreal(1.5);

    // eDP-1: a 1920x1080 panel at 100%. The portal upscaled it into the
    // composite, so the crop holds more pixels than the panel has; dropping
    // back discards only what the portal invented.
    QTest::newRow("portal upscale is undone")
      << QSize(2880, 1620) << QSize(1920, 1080) << qreal(1.0)
      << QSize(1920, 1080) << qreal(1.0);

    // Composite scale and reported ratio agree: nothing to do.
    QTest::newRow("matching scale is untouched")
      << QSize(3840, 2160) << QSize(1920, 1080) << qreal(2.0)
      << QSize(3840, 2160) << qreal(2.0);

    QTest::newRow("unscaled display is untouched")
      << QSize(1920, 1080) << QSize(1920, 1080) << qreal(1.0)
      << QSize(1920, 1080) << qreal(1.0);

    // Over budget on one axis only still clamps both, so the crop keeps the
    // monitor's logical size rather than acquiring a new aspect ratio.
    QTest::newRow("single axis over budget clamps both")
      << QSize(2880, 1080) << QSize(1920, 1080) << qreal(1.0)
      << QSize(1920, 1080) << qreal(1.0);
}

void TestDprScaling::fitCrop()
{
    QFETCH(QSize, cropped);
    QFETCH(QSize, logical);
    QFETCH(qreal, reportedDpr);
    QFETCH(QSize, expectedSize);
    QFETCH(qreal, expectedDpr);

    const Fit fit = DprScaling::fitCrop(cropped, logical, reportedDpr);

    QCOMPARE(fit.size, expectedSize);
    QCOMPARE(fit.dpr, expectedDpr);
}

/// The invariant behind every row above: a crop is never scaled up, whatever
/// the reported ratio claims.
void TestDprScaling::fitCrop_neverEnlarges()
{
    for (int width = 800; width <= 4000; width += 400) {
        const QSize cropped(width, width / 2);
        const Fit fit =
          DprScaling::fitCrop(cropped, QSize(1920, 960), qreal(2.0));
        QVERIFY(fit.size.width() <= cropped.width());
        QVERIFY(fit.size.height() <= cropped.height());
    }
}

void TestDprScaling::fitCrop_degenerateLogicalSize()
{
    // A zero logical width would divide by zero when deriving the ratio.
    const Fit fit = DprScaling::fitCrop(QSize(0, 0), QSize(0, 0), qreal(2.0));
    QCOMPARE(fit.dpr, qreal(2.0));
}

QTEST_APPLESS_MAIN(TestDprScaling)
#include "tst_dprscaling.moc"
