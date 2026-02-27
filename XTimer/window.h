#pragma once
#include <qwidget.h>
#include <QFrame>
#include <QBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QList>
#include <QPainter>

class XTimerWindow : public QWidget {
    Q_OBJECT
public:
    explicit XTimerWindow(QWidget* parent = nullptr);
    static void clearLayout(QLayout* layout);
    void setFrameColor(const QColor& color); // frame is not used anymore. remove?
    void setBackgroundImage(const QString& file);
    void updateWindowContent(); // sets style sheet for title and timer elements, also calls updateSegmentsLayout
    void updateSegmentsLayout(bool shouldResize = false);
    void updateTitle(const QString& text);
    void updateAttempts();
    void updateActiveSegmentColor();
    void updateSegmentsTimeDiff();
private:
    QFrame* wndFrame = nullptr;
    QVBoxLayout* frameLayout = nullptr;
    QScrollArea* mainFrameScroll = nullptr;
    QVBoxLayout* layout = nullptr;
    QLabel* title = nullptr;
    QLabel* attempts = nullptr;
    QLabel* titleEmptyLabel = nullptr;
    QLabel* timerEmptyLabel = nullptr;
    
    QString activeSegmentStyleSheet;
    QList<QLabel*> segmentsTimeDiffBuffer;
    QPixmap background_img;
protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
};

extern XTimerWindow* window;
