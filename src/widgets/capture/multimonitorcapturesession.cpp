// SPDX-License-Identifier: GPL-3.0-or-later

#include "multimonitorcapturesession.h"

#include "utils/desktopinfo.h"
#include "utils/screengrabber.h"
#include "widgets/capture/capturewidget.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

namespace {
// Fast enough to feel immediate, cheap enough to be irrelevant. Only used
// where a global cursor position exists at all.
constexpr int POINTER_POLL_MS = 50;
}

MultiMonitorCaptureSession::MultiMonitorCaptureSession(QObject* parent)
  : QObject(parent)
{}

bool MultiMonitorCaptureSession::start(const CaptureRequest& request)
{
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return false;
    }

    bool ok = true;
    ScreenGrabber grabber;
    const QPixmap fullDesktop = grabber.grabFullDesktop(ok);
    if (!ok || fullDesktop.isNull()) {
        return false;
    }

    m_geometries.clear();
    for (QScreen* screen : screens) {
        m_geometries.append(screen->geometry());
    }
    m_tracker = MonitorFocus::ActiveMonitorTracker(screens.size());

    {
        // Held only while the widgets are built, so each one crops from this
        // grab instead of calling the screenshot portal again.
        ScreenGrabber::SessionCache cache(fullDesktop);

        for (int i = 0; i < screens.size(); ++i) {
            CaptureRequest perScreen = request;
            perScreen.setSelectedMonitor(i);

            auto* widget = new CaptureWidget(perScreen);
            m_widgets.append(widget);

            connect(widget,
                    &CaptureWidget::pointerEnteredMonitor,
                    this,
                    &MultiMonitorCaptureSession::handlePointerEntered);
            connect(widget,
                    &CaptureWidget::editingStarted,
                    this,
                    &MultiMonitorCaptureSession::handleEditingStarted);
            connect(widget,
                    &QObject::destroyed,
                    this,
                    &MultiMonitorCaptureSession::handleWidgetDestroyed);

#if defined(Q_OS_WIN)
            widget->show();
#else
            widget->showFullScreen();
#endif
        }
    }

    // Nothing is armed until the pointer announces itself, so start every
    // display dimmed rather than showing N help overlays at once.
    for (auto& widget : m_widgets) {
        if (widget) {
            widget->setArmed(false);
        }
    }

    // Wayland has no global cursor position; there the per-widget Enter events
    // are the only signal, and polling would fight them.
    if (!DesktopInfo().waylandDetected()) {
        m_pollTimer = new QTimer(this);
        m_pollTimer->setInterval(POINTER_POLL_MS);
        connect(m_pollTimer, &QTimer::timeout, this, [this]() {
            handlePointerEntered(
              MonitorFocus::monitorIndexAt(m_geometries, QCursor::pos()));
        });
        m_pollTimer->start();
    }

    return true;
}

CaptureWidget* MultiMonitorCaptureSession::armedWidget() const
{
    const int active = m_tracker.activeMonitor();
    if (active < 0 || active >= m_widgets.size()) {
        return nullptr;
    }
    return m_widgets.at(active);
}

void MultiMonitorCaptureSession::handlePointerEntered(int monitorIndex)
{
    if (!m_tracker.pointerAt(monitorIndex)) {
        return;
    }
    applyArmedState();
    emit armedChanged(armedWidget());
}

void MultiMonitorCaptureSession::handleEditingStarted(int monitorIndex)
{
    if (m_tracker.isLatched()) {
        return;
    }
    m_tracker.beginEditing(monitorIndex);

    if (m_pollTimer) {
        m_pollTimer->stop();
    }
    applyArmedState();
    discardAllExcept(m_tracker.activeMonitor());
    emit latched(armedWidget());
}

void MultiMonitorCaptureSession::applyArmedState()
{
    const int active = m_tracker.activeMonitor();
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets.at(i)) {
            m_widgets.at(i)->setArmed(i == active);
        }
    }
}

void MultiMonitorCaptureSession::discardAllExcept(int keepIndex)
{
    m_tearingDown = true;
    for (int i = 0; i < m_widgets.size(); ++i) {
        CaptureWidget* widget = m_widgets.at(i);
        if (!widget || i == keepIndex) {
            continue;
        }
        // Without this each discarded widget would report a failed capture,
        // which main.cpp turns into an application exit.
        widget->discardSilently();
        widget->close();
    }
    m_tearingDown = false;
}

void MultiMonitorCaptureSession::handleWidgetDestroyed()
{
    if (m_tearingDown) {
        return;
    }

    // A widget went away on its own: the user pressed Esc, or the capture
    // finished. Either way the session is over, so take the rest down quietly
    // -- the widget that closed has already reported the outcome.
    if (!m_tracker.isLatched()) {
        discardAllExcept(-1);
    }

    bool anyLeft = false;
    for (const auto& widget : m_widgets) {
        if (widget) {
            anyLeft = true;
            break;
        }
    }
    if (!anyLeft) {
        deleteLater();
    }
}
