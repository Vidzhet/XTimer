#include "window.h"
#include "qwindow.h"
#include "qmenu.h"
#include <qmouseevent>
#include <qapplication.h>
#include "timer.h"
#include <QtWidgets/QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include "styles.h"
#include "listener.h"
#include "config.h"
#include "settings.h"
#include <qmessagebox.h>
#include <QColorDialog>
#include <QFileDialog>
#include <QScrollBar>
#include <QFontDatabase>
#include "window_glow.h"

XTimerWindow::XTimerWindow(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | (config->window_stays_on_top ? Qt::Window |  Qt::WindowStaysOnTopHint : Qt::Window));
    setWindowOpacity(config->window_opacity);
    //new WindowGlow(this); // window glow is disabled

    //QVBoxLayout* mainVL = new QVBoxLayout(this);
    //wndFrame = new QFrame();
    //wndFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //wndFrame->setFrameStyle(QFrame::StyledPanel);
    //wndFrame->setObjectName("wndFrame");
    //wndFrame->setAttribute(Qt::WA_OpaquePaintEvent);
    //wndFrame->setAutoFillBackground(true);
    //wndFrame->setStyleSheet("background: transparent;");
    //setFrameColor(config->frame_color);

    //QVBoxLayout* layout = new QVBoxLayout(wndFrame);
    //mainVL->addWidget(wndFrame); // i dont like the frame
    //mainVL->setContentsMargins(0, 0, 0, 0);
    //mainVL->setSpacing(0);

    QVBoxLayout* layout = new QVBoxLayout(this); // i removed the frame for now. if you need to return the frame, delete this line and uncomment the block above.
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QHBoxLayout* titleHL = new QHBoxLayout();
    titleEmptyLabel = new QLabel();
    titleEmptyLabel->setContentsMargins(8, 0, 8, 0);
    titleHL->addWidget(titleEmptyLabel); // empty element to center the title
    title = new QLabel(config->title_name);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    title->setAlignment(Qt::AlignCenter);
    title->setFixedHeight(30);
    attempts = new QLabel(QString::number(config->attempts));
    attempts->setContentsMargins(8, 0, 8, 0);
    titleHL->addWidget(title);
    titleHL->addWidget(attempts);
    layout->addLayout(titleHL);

    QFrame* mainFrame = new QFrame();
    mainFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    frameLayout = new QVBoxLayout(mainFrame);
    frameLayout->setSpacing(0);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    mainFrameScroll = new QScrollArea();
    mainFrameScroll->setWidgetResizable(true);
    mainFrameScroll->setFixedWidth(config->window_width);
    mainFrameScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainFrameScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainFrameScroll->setWidget(mainFrame);
    mainFrameScroll->setFrameShape(QFrame::NoFrame);
    mainFrameScroll->setStyleSheet("background: transparent;");

    updateSegmentsLayout(true);
    updateActiveSegmentColor();
    setBackgroundImage(config->bg_img_file);

    layout->addWidget(mainFrameScroll);

    QHBoxLayout* timerHL = new QHBoxLayout();
    auto timerLabel = timer->getLabel();
    timerLabel->setContentsMargins(0, 0, 8, 0);
    timerEmptyLabel = new QLabel();
    timerEmptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    timerHL->addWidget(timerEmptyLabel);
    timerHL->addWidget(timerLabel);
    layout->addLayout(timerHL);

    updateWindowContent();

    layout->addStretch();
}

void XTimerWindow::clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        if (QLayout* l = item->layout()) {
            clearLayout(l);
        }
        delete item;
    }
}

void XTimerWindow::setFrameColor(const QColor& color)
{
    /*if (wndFrame) { 
        wndFrame->setStyleSheet(QString("QFrame#wndFrame{ border: 1px solid %1; }").arg(color.name()));
    }*/
}

void XTimerWindow::setBackgroundImage(const QString& file)
{
    if (config->enable_bg_img && !file.isEmpty()) {
        background_img = QPixmap(file).scaled(this->size(), Qt::KeepAspectRatioByExpanding);
    }
    /*if (background_img.isNull()) {
        QMessageBox::information(this, "", "cant open bg image");
    }*/
}

void XTimerWindow::updateWindowContent()
{
    QString titleBgStyleSheet(config->enable_bg_img ? "background: transparent;" : "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(44, 44, 44, 255), stop:1 rgba(22, 22, 22, 255));");
    QString timerBgStyleSheet(config->enable_bg_img ? "background: transparent;" : "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(33, 33, 33, 255), stop:1 rgba(22, 22, 22, 255));");

    timer->getLabel()->setStyleSheet(timerBgStyleSheet);
    QString timer_font_family;
    int timer_font_id = QFontDatabase::addApplicationFont(config->timer_font);
    if (timer_font_id != -1) {
        timer_font_family = QFontDatabase::applicationFontFamilies(timer_font_id).at(0);
    }
    if (config->enable_font && !config->timer_font.isEmpty()) {
        QFont font(timer_font_id == -1 ? config->timer_font : timer_font_family);
        font.setPointSize(config->timer_font_size);
        font.setBold(true);
        timer->getLabel()->setFont(font);
    }
    else {
        QFont font;
        font.setPointSize(config->timer_font_size);
        font.setBold(true);
        timer->getLabel()->setFont(font);
    }
    if (config->enable_bg_img) {
        if (!timer->getLabel()->graphicsEffect()) {
            QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
            shadow->setBlurRadius(20);
            shadow->setOffset(3, 3);
            shadow->setColor(Qt::black);
            timer->getLabel()->setGraphicsEffect(shadow);
        }
    }
    else {
        timer->getLabel()->setGraphicsEffect(nullptr);
    }
    if (timerEmptyLabel) {
        timerEmptyLabel->setStyleSheet(timerBgStyleSheet);
    }
    if (titleEmptyLabel) {
        titleEmptyLabel->setStyleSheet(titleBgStyleSheet);
    }
    QString font_family;
    int font_id = QFontDatabase::addApplicationFont(config->font);
    if (font_id != -1) {
        font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
    }
    if (title) {
        title->setStyleSheet(titleBgStyleSheet);
        if (config->enable_font && !config->font.isEmpty()) {
            QFont font(font_id == -1 ? config->font : font_family);
            font.setPointSize(10);
            font.setWeight(QFont::DemiBold);
            title->setFont(font);
        }
        else {
            QFont font = title->font();
            font.setPointSize(10);
            font.setWeight(QFont::DemiBold);
            title->setFont(font);
        }
    }
    if (attempts) {
        attempts->setStyleSheet(titleBgStyleSheet);
        if (config->enable_font && !config->font.isEmpty()) {
            QFont font(font_id == -1 ? config->font : font_family);
            attempts->setFont(font);
        }
        else {
            attempts->setFont(QFont());
        }
    }
    updateSegmentsLayout(true);
}

void XTimerWindow::updateSegmentsLayout(bool shouldResize)
{
    clearLayout(frameLayout); 
    segmentsTimeDiffBuffer.clear(); // old pointers are not valid after clearing layout

    int rowCount = 0;
    for (auto& segment : config->segments) {
        QHBoxLayout* segmentLayout = new QHBoxLayout();
        QLabel* label = new QLabel(segment.name);
        QString font_family;
        int font_id = QFontDatabase::addApplicationFont(config->font);
        if (font_id != -1) {
            font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        }
        QFont labelFont = config->enable_font && !config->font.isEmpty() ? (font_id == -1 ? config->font : font_family) : label->font();
        labelFont.setPointSize(10);
        labelFont.setWeight(QFont::DemiBold);
        label->setFont(labelFont);
        
        QString bgColorDarkerStyleSheet = config->enable_bg_img ? QString("background: transparent;") : QString("background: rgba(1,1,1,111);");
        QString bgColorLighterStyleSheet = config->enable_bg_img ? QString("background: transparent;") : QString("background: rgba(1,1,1,60);");
        QString bgColor = rowCount==timer->getActiveSegment() && timer->getTimeMs() > 0 ? activeSegmentStyleSheet : (rowCount % 2 == 0) ? bgColorDarkerStyleSheet : bgColorLighterStyleSheet;

        label->setContentsMargins(8, 2, 0, 2);
        label->setFixedHeight(config->row_height);
        label->setStyleSheet(bgColor);
        QFontMetrics fm(labelFont);
        label->setFixedWidth(config->window_width - fm.horizontalAdvance("+00:00 +0:00:00"));
        label->setText(fm.elidedText(label->text(), Qt::ElideRight, label->width() - fm.horizontalAdvance("..."))); // free up some space for ...

        segmentLayout->addWidget(label);

        QLabel* segmentTimeDiff = new QLabel(segment.segment_time_diff.text);
        segmentTimeDiff->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        QFont timeDiffFont = config->enable_font && !config->font.isEmpty() ? (font_id == -1 ? config->font : font_family) : segmentTimeDiff->font();
        timeDiffFont.setWeight(QFont::Bold);
        segmentTimeDiff->setFont(timeDiffFont);
        segmentsTimeDiffBuffer.append(segmentTimeDiff); // pushing label pointers to the buffer so we can then update them separately. see updateSegmentsTimeDiff()
        segmentTimeDiff->setStyleSheet(bgColor + QString("color: %1;").arg(segment.segment_time_diff.color.name()));
        segmentLayout->addWidget(segmentTimeDiff);

        if (segment.segment_time_ms > 0) {
            QString segmentTimeStr = timer->formatTimeToStr(segment.segment_time_ms, 2);
            if (segment.segment_time_ms < 10000) { // do we want that?
                segmentTimeStr.push_front("0");
            }
            else if (segment.segment_time_ms > 60000) {
                segmentTimeStr = timer->formatTimeToStr(segment.segment_time_ms, 0);
            }
            QLabel* segmentTime = new QLabel(segmentTimeStr);
            segmentTime->setContentsMargins(8, 2, 4, 2);
            segmentTime->setStyleSheet(bgColor);
            QFont font = config->enable_font && !config->font.isEmpty() ? (font_id == -1 ? config->font : font_family) : segmentTime->font();
            font.setWeight(static_cast<QFont::Weight>(1000));
            font.setPointSize(10);
            segmentTime->setFont(font);
            segmentLayout->addWidget(segmentTime);
        }

        frameLayout->addLayout(segmentLayout);

        rowCount++;
    }
    frameLayout->addStretch();
    frameLayout->update();

    auto scroll = mainFrameScroll->verticalScrollBar();
    if (std::max(0, (timer->getActiveSegment() + 1) + 1 - config->max_frame_segments) > scroll->value() / config->row_height) {
        scroll->setValue(scroll->value() + config->row_height);
    }
    else if (timer->getActiveSegment() <= scroll->value() / config->row_height) {
        scroll->setValue(std::max(0, scroll->value() - config->row_height));
    }

    if (shouldResize) {
        rowCount = std::min(rowCount, static_cast<int>(config->max_frame_segments)); // max rows in frame
        mainFrameScroll->setFixedHeight(config->row_height * rowCount/* + 2*/); // +2 was there because of the frame borders
        mainFrameScroll->setFixedWidth(config->window_width);
        QTimer::singleShot(0, this, [this, rowCount]() {
            resize(config->window_width, rowCount * config->row_height + 100);
            resizeEvent(nullptr);
            });
    }
}

void XTimerWindow::updateTitle(const QString& text)
{
    title->setText(text);
}

void XTimerWindow::updateAttempts()
{
    if (attempts) {
        attempts->setText(QString::number(config->attempts));
    }
}

void XTimerWindow::updateActiveSegmentColor()
{
    QColor colorDown = config->active_segment_color;
    float h, s, v;
    colorDown.getHsvF(&h, &s, &v);

    v += 0.5f;
    if (v > 1.0f) { v = 1.0f; }

    QColor colorUp;
    colorUp.setHsvF(h, s, v);

    activeSegmentStyleSheet = QString("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(%1, %2, %3, 150), stop:1 rgba(%5, %6, %7, 150));").arg(colorUp.red()).arg(colorUp.green()).arg(colorUp.blue()).arg(colorDown.red()).arg(colorDown.green()).arg(colorDown.blue());
}

void XTimerWindow::updateSegmentsTimeDiff()
{
    QList<QLabel*>::iterator bufferIt = segmentsTimeDiffBuffer.begin();
    for (QList<RunSegment>::iterator segmentIt = config->segments.begin(); bufferIt != segmentsTimeDiffBuffer.end() && segmentIt != config->segments.end(); ++segmentIt, ++bufferIt) {
        (*bufferIt)->setText(segmentIt->segment_time_diff.text);
    }
}

void XTimerWindow::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		windowHandle()->startSystemMove();
}

void XTimerWindow::mouseMoveEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		windowHandle()->startSystemMove();
}

void XTimerWindow::contextMenuEvent(QContextMenuEvent* e)
{
	QMenu menu(this);
    menu.addAction("Open", [this]() {
        QString file_path = QFileDialog::getOpenFileName(this, "Open Run Config", QCoreApplication::applicationDirPath(), "MGE File (*.mge);;All Files (*)");
        if (!file_path.isEmpty()) {
            config->LoadRunConfig(file_path);
            config->runFilePath = file_path;
        }
        });
    menu.addAction("Save", [this]() {
        if (config->runFilePath.isEmpty()) {
            QString file_path = QFileDialog::getSaveFileName(this, "Save Run Config", QCoreApplication::applicationDirPath(), "MGE File (*.mge);;All Files (*)");
            if (!file_path.isEmpty()) {
                config->SaveRunConfig(file_path);
                config->runFilePath = file_path;
            }
        }
        else {
            config->SaveRunConfig(config->runFilePath);
        }
        });
    menu.addAction("Save As", [this]() {
        QString file_path = QFileDialog::getSaveFileName(this, "Save Run Config", QCoreApplication::applicationDirPath(), "MGE File (*.mge);;All Files (*)");
        if (!file_path.isEmpty()) {
            config->SaveRunConfig(file_path);
            config->runFilePath = file_path;
        }
        });
    menu.addSeparator();
	menu.addAction("Start", []() { 
		timer->reset();
		timer->start();
		});
	menu.addAction("Reset", []() {
		timer->reset();
		});
	menu.addAction("Pause", []() {
		timer->pause();
		});
	menu.addAction("Resume", []() {
		timer->resume();
		});
	menu.addAction("Next Segment", []() {
		timer->next();
		});
	menu.addAction("Previous Segment", []() {
		timer->prev();
		});
    menu.addSeparator();
	menu.addAction("Settings", [this]() { 
        XTimerSettings* settingsWnd = new XTimerSettings(this);
        settingsWnd->resize(600, 400);
        while (settingsWnd->exec() == QDialog::Accepted) { // accepted = it needs to be recreated (i just dont want to recreate the layout manually, i apologize for the bad design.)
            delete settingsWnd;
            settingsWnd = new XTimerSettings(this);
            settingsWnd->resize(600, 400);
        }
        delete settingsWnd;
        /*XTimerSettings settingsWnd;
        settingsWnd.resize(600, 400);
        settingsWnd.exec();*/
		});
    menu.addSeparator();
	menu.addAction("Hide", [this]() { 
        showMinimized();
		});

	menu.addAction("Exit", &QApplication::quit);
	menu.exec(e->globalPos());
}

void XTimerWindow::resizeEvent(QResizeEvent* e) // be careful with event ptr here, i called it explicitly once and passed nullptr in arguments.
{
    QPainterPath path; // rounding
    path.addRoundedRect(this->rect(), config->window_rounding, config->window_rounding);
    this->setMask(QRegion(path.toFillPolygon().toPolygon()));

    if (!background_img.isNull() && config->enable_bg_img) {
        background_img = background_img.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }

    // if (!e) { return; } // use that if you will ever get to need an event
}

void XTimerWindow::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    if (!background_img.isNull() && config->enable_bg_img) {
        p.drawPixmap(rect(), background_img);
        p.fillRect(rect(), QColor(1, 1, 1, (1.0f - config->bg_brightness) * 255));
    }

    QWidget::paintEvent(e);
}

XTimerWindow* window = nullptr;
