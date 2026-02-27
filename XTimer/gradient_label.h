#pragma once
#include <QPainter>
#include <QColor>
#include <QLabel>

class GradientLabel : public QLabel {
public:
	static QColor colorShift(const QColor& col) {
		float h, s, v;
		col.getHsvF(&h, &s, &v);

		h += 0.1f;
		if (h > 1.0f) { h -= 1.0f; }

		QColor out;
		out.setHsvF(h, s, v);
		return out;
	}
public:
	using QLabel::QLabel;

	void SetColor(const QColor& col) {
		colorDown = col;
		colorUp = colorShift(col);
	}
	void SetVGradient(const QColor& down, const QColor& up) {
		colorDown = down;
		colorUp = up;
	}
	QColor GetColor() {
		return colorDown;
	}

protected:
	QColor colorUp;
	QColor colorDown;
	void paintEvent(QPaintEvent* event) override {
		QPainter painter(this);
		QLinearGradient gradient(0, 0, 0, height());
		gradient.setColorAt(0.0, colorUp);
		gradient.setColorAt(1.0, colorDown);

		QPen pen;
		pen.setBrush(QBrush(gradient));
		painter.setPen(pen);

		painter.setFont(font());
		painter.drawText(rect(), alignment(), text());
	}
};