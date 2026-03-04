#include <chrono>
#include <QBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QtWidgets/QApplication>
#include "styles.h"
#include "listener.h"
#include "window.h"
#include "config.h"
#include "timer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    api = new Listener;
    config = new XTimerConfig; // init config
    timer = new Timer;

    window = new XTimerWindow;

    LoadStylesGlobal(app);

    window->show();
    return app.exec();
}

COMMAND(test) {
    QMessageBox::information(nullptr, "Test", "Called from IPC");
    NORETURN;
}

COMMAND(reset) {
    timer->reset();
    NORETURN;
}

COMMAND(start) {
    timer->start(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong())));
    NORETURN;
}

COMMAND(pause) {
    timer->pause(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong())));
    NORETURN;
}

COMMAND(resume) {
    timer->resume(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong())));
    NORETURN;
}

COMMAND(next) {
    timer->next(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong())));
    NORETURN;
}

COMMAND(prev) {
    timer->prev(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong())));
    NORETURN;
}

COMMAND(get_time_ms) {
    RETURN_UINT64(timer->getTimeMs(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong()))));
}

COMMAND(get_time_str) {
    RETURN_STR(timer->getTimeStr(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(args[0].toULongLong()))));
}

COMMAND(set_time_ms) {
    timer->setTime(args[0].toULongLong());
    NORETURN;
}

COMMAND(is_running) {
    RETURN_BOOL(timer->isRunning());
}
