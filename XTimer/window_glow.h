#pragma once
#include <QWidget>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>

class WindowGlow : public QWidget
{
public:
    // target is the window that gradient has to be applied on
    explicit WindowGlow(QWidget* target)
        : QWidget(nullptr), m_target(target)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint);

        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);

        resizeToTarget();
        show();

        m_target->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override
    {
        if (obj == m_target)
        {
            switch (ev->type())
            {
            case QEvent::Move:
            case QEvent::Resize:
                resizeToTarget();
                break;
            case QEvent::Show:
                show();
                break;
            case QEvent::Hide:
                hide();
                break;
            case QEvent::WindowActivate:
                lower();
                break;
            default:
                break;
            }
        }
        return QWidget::eventFilter(obj, ev);
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int glow = 40;

        QRect inner = rect().adjusted(glow, glow, -glow, -glow);
        QPainterPath innerPath;
        innerPath.addRoundedRect(inner, 12, 12);

        QPainterPath outerPath;
        outerPath.addRoundedRect(rect(), 12 + glow, 12 + glow);
        outerPath = outerPath.subtracted(innerPath);

        QImage img(size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);

        QPainter imgPainter(&img);
        imgPainter.setRenderHint(QPainter::Antialiasing);

        for (int y = 0; y < height(); ++y)
        {
            float alphaY = 1.0f - float(abs(y - height() / 2)) / (height() / 2);
            for (int x = 0; x < width(); ++x)
            {
                float alphaX = 1.0f - float(abs(x - width() / 2)) / (width() / 2);
                float alpha = alphaX * alphaY;
                QColor color(0, 255, 64, int(alpha * 160));
                img.setPixelColor(x, y, color);
            }
        }

        p.fillPath(outerPath, QBrush(img));
    }

private:
    void resizeToTarget()
    {
        const int glow = 40;

        QRect g = m_target->frameGeometry();
        setGeometry(g.adjusted(-glow, -glow, glow, glow));
        lower();
    }

    QWidget* m_target;
};