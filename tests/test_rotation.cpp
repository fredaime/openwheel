// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 OpenWheel Contributors

#include <QtTest/QtTest>
#include "core/rotationhandler.h"

class TestRotation : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInitialState();
    void testAccelerationCurve();
    void testReset();
    void testZeroDeltaIgnored();
    void testActionTriggered();
    void testAccelerationToggle();
    void testBaseThreshold();
};

void TestRotation::testInitialState()
{
    RotationHandler handler;
    QCOMPARE(handler.currentValue(), 0.0);
    QCOMPARE(handler.isAdjusting(), 0);
}

void TestRotation::testAccelerationCurve()
{
    // Test via the public interface: slow rotation should not accelerate
    RotationHandler handler;
    handler.setAccelerationEnabled(1);
    handler.setBaseThreshold(15);

    // A single small rotation shouldn't trigger (below threshold)
    QSignalSpy spy(&handler, &RotationHandler::actionTriggered);
    handler.onRotation(10);
    QCOMPARE(spy.count(), 0);

    // Enough rotation should trigger
    handler.onRotation(10);
    QVERIFY(spy.count() >= 1);
}

void TestRotation::testReset()
{
    RotationHandler handler;
    handler.onRotation(5);
    QVERIFY(handler.currentValue() != 0.0);

    handler.reset();
    QCOMPARE(handler.currentValue(), 0.0);
    QCOMPARE(handler.isAdjusting(), 0);
}

void TestRotation::testZeroDeltaIgnored()
{
    RotationHandler handler;
    QSignalSpy valueSpy(&handler, &RotationHandler::valueChanged);
    handler.onRotation(0);
    QCOMPARE(valueSpy.count(), 0);
}

void TestRotation::testActionTriggered()
{
    RotationHandler handler;
    handler.setBaseThreshold(10);
    handler.setAccelerationEnabled(0);

    ActionConfig cw;
    cw.type = ActionConfig::Type::Keyboard;
    cw.keys = QStringLiteral("Right");
    ActionConfig ccw;
    ccw.type = ActionConfig::Type::Keyboard;
    ccw.keys = QStringLiteral("Left");
    handler.setCurrentActions(cw, ccw);

    QSignalSpy spy(&handler, &RotationHandler::actionTriggered);

    // Clockwise: 15 degrees with threshold 10 should trigger once
    handler.onRotation(15);
    QVERIFY(spy.count() >= 1);

    spy.clear();
    handler.reset();

    // Counter-clockwise
    handler.onRotation(-15);
    QVERIFY(spy.count() >= 1);
}

void TestRotation::testAccelerationToggle()
{
    RotationHandler handler;
    handler.setAccelerationEnabled(0);
    // No crash, just verifies toggle works
    handler.setAccelerationEnabled(1);
}

void TestRotation::testBaseThreshold()
{
    RotationHandler handler;
    handler.setBaseThreshold(30);
    handler.setAccelerationEnabled(0);

    QSignalSpy spy(&handler, &RotationHandler::actionTriggered);
    handler.onRotation(20);
    QCOMPARE(spy.count(), 0);  // Below threshold of 30

    handler.onRotation(15);
    QVERIFY(spy.count() >= 1); // Now accumulated 35 > 30
}

QTEST_GUILESS_MAIN(TestRotation)
#include "test_rotation.moc"
