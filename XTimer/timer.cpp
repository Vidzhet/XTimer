#include "timer.h"
#include <QDateTime>
#include <QPainter>
#include <QColor>
#include <qmessagebox>
#include "listener.h"
#include "config.h"
#include "window.h"
#include "gradient_label.h"

Timer::Timer()
{
	label = new GradientLabel(formatTimeToStr(0, config->ms_precision));
	QFont font = label->font();
	font.setPointSize(config->timer_font_size);
	font.setBold(true);
	label->setFont(font);
	label->SetVGradient(config->timer_label_inactiveDown, config->timer_label_inactiveUp);

	QTimer* loop_timer = new QTimer();
	loop_timer->setInterval(10);
	loop_timer->start();
	QObject::connect(loop_timer, &QTimer::timeout, [&]() {
		if (!running) { 
			return; 
		}
		label->setText(this->getTimeStr());

		const auto timer_time = this->getTimeMs();
		if (timer_time > 0 && !config->segments.isEmpty()) { // should we count it?
			auto& segment = config->segments[active_segment];
			const auto segment_time_ms = segment.segment_time_ms;
			if (segment_time_ms > 0 ) { 
				if (segment.segment_time_diff.text.isEmpty()) { // if not empty, then we know that code below has already worked.
					if (active_segment == 0) {
						if (!(timer_time > segment_time_ms / 3 * 2)) { return; }
					} // if time is within or past of one third of avg segment time
					else {
						auto prev_segment_time_ms = config->segments[active_segment - 1].segment_time_ms;
						if (!(timer_time > prev_segment_time_ms + (segment_time_ms - prev_segment_time_ms) / 3 * 2) && prev_segment_time_ms < segment_time_ms) { return; }
					}
				}
				segment.segment_time_diff.text = getActiveSegmentTimeDiff();
				window->updateSegmentsTimeDiff();
			}
		}
		
		});
}

QLabel* Timer::getLabel()
{
	return label;
}

int Timer::getActiveSegment()
{
	return active_segment;
}

QString Timer::getActiveSegmentTimeDiff()
{
	const auto timer_time = this->getTimeMs();
	const auto segment_time_ms = config->segments[active_segment].segment_time_ms;

	QString time;

	if (segment_time_ms == 0) {
		return time;
	}

	if (timer_time < segment_time_ms) { // -time (better time)
		time = QString("-%1").arg(formatTimeToStr(segment_time_ms - timer_time, 2));
		if (segment_time_ms - timer_time > 60000) {
			time.chop(3); // removing miliseconds from the string so its not too big
		}
	}
	else { // +time (worse time)
		if (label->GetColor() != config->timer_label_worseDown) {
			label->SetVGradient(config->timer_label_worseDown, config->timer_label_worseUp); // set red color for timer
		}
		time = QString("+%1").arg(formatTimeToStr(timer_time - segment_time_ms, 2));
		if (timer_time - segment_time_ms > 60000) {
			time.chop(3); // removing miliseconds from the string so its not too big
		}
	}

	return time;
}

void Timer::start(std::chrono::steady_clock::time_point request_time)
{
	startTime = request_time;
	accumulatedMs = 1;
	pausedDuration = std::chrono::steady_clock::duration::zero();
	running = true;
	active_segment = 0;
	config->attempts++;

	for (auto& segment : config->segments) {
		if (segment.segment_avg_time_ms > 0) {
			segment.segment_time_ms = segment.segment_avg_time_ms;
		}
		segment.segment_time_diff.text.clear();
		segment.segment_time_diff.color.setRgb(255, 255, 255);
	}

	label->SetVGradient(config->timer_label_activeDown, config->timer_label_activeUp);
	window->updateSegmentsLayout();
	window->updateAttempts();
}

void Timer::reset()
{
	accumulatedMs = 0;
	running = false;
	label->setText(formatTimeToStr(0, config->ms_precision));
	active_segment = 0;

	for (auto& segment : config->segments) {
		if (segment.segment_avg_time_ms > 0) {
			segment.segment_time_ms = segment.segment_avg_time_ms;
		}
		segment.segment_time_diff.text.clear();
		segment.segment_time_diff.color.setRgb(255, 255, 255);
	}

	label->SetVGradient(config->timer_label_inactiveDown, config->timer_label_inactiveUp);
	window->updateSegmentsLayout();
}

void Timer::pause(std::chrono::steady_clock::time_point request_time)
{
	if (!running) { return; }
	accumulatedMs += std::chrono::duration_cast<std::chrono::milliseconds>(request_time - startTime).count();
	running = false;

	this->setTime(this->getTimeMs()); // fix the delay of 2ms on display

	label->SetVGradient(config->timer_label_pausedDown, config->timer_label_pausedUp);
}

void Timer::resume(std::chrono::steady_clock::time_point request_time)
{
	if (running) { return; }
	startTime = request_time;
	running = true;

	label->SetVGradient(config->timer_label_activeDown, config->timer_label_activeUp);
}

void Timer::next(std::chrono::steady_clock::time_point request_time)
{
	const auto timer_time = this->getTimeMs(request_time);
	if (timer_time == 0 || config->segments.isEmpty() || active_segment + 1 > config->segments.count()) { return; }

	calculateTimeDiffColor(request_time);

	auto& segment = config->segments[active_segment];
	segment.segment_time_ms = timer_time;
	auto segment_time_ms = segment.segment_time_ms;
	auto segment_avg_time_ms = segment.segment_avg_time_ms;
	segment.segment_avg_time_ms = segment_avg_time_ms > 0 ? (segment_avg_time_ms + segment_time_ms)/2 : segment_time_ms;
	++active_segment;
	if (active_segment + 1 > config->segments.count()) { // run is finished
		this->setTime(getTimeMs(request_time));
		accumulatedMs += std::chrono::duration_cast<std::chrono::milliseconds>(request_time - startTime).count();
		running = false;
		label->SetVGradient(config->timer_label_finishedDown, config->timer_label_finishedUp);
	}
	else {
		config->segments[active_segment].segment_time_diff.color.setRgb(255, 255, 255);
		label->SetVGradient(config->timer_label_activeDown, config->timer_label_activeUp);
	}
	window->updateSegmentsLayout();
}

void Timer::prev(std::chrono::steady_clock::time_point request_time)
{
	if (active_segment == 0 || this->getTimeMs() == 0 || config->segments.isEmpty() || active_segment >= config->segments.count()) { return; }
	calculateTimeDiffColor(request_time);
	--active_segment;
	config->segments[active_segment].segment_time_diff.color.setRgb(255, 255, 255);
	window->updateSegmentsLayout();
}

uint64_t Timer::getTimeMs(std::chrono::steady_clock::time_point request_time)
{
	if (!running) { return accumulatedMs; }

	return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(request_time - startTime).count() + accumulatedMs);
}

QString Timer::getTimeStr(std::chrono::steady_clock::time_point request_time)
{
	return formatTimeToStr(this->getTimeMs(request_time), config->ms_precision);
}

QString Timer::formatTimeToStr(uint64_t time_ms, ushort ms_precision)
{
	ms_precision = std::clamp(ms_precision, (ushort)0, (ushort)3);

	qint64 hours = time_ms / 3600000;
	qint64 minutes = (time_ms % 3600000) / 60000;
	qint64 seconds = (time_ms % 60000) / 1000;
	qint64 ms = time_ms % 1000;

	switch (ms_precision) {
	case 0: ms = 0; break;
	case 1: ms /= 100; break;
	case 2: ms /= 10;  break;
	case 3: break;
	}

	QString text;
	if (hours > 0) {
		text = QString("%1:%2:%3.%4")
			.arg(hours)
			.arg(minutes, 2, 10, QChar('0'))
			.arg(seconds, 2, 10, QChar('0'))
			.arg(ms, ms_precision, 10, QChar('0'));
	}
	else if (minutes > 0) {
		text = QString("%1:%2.%3")
			.arg(minutes)
			.arg(seconds, 2, 10, QChar('0'))
			.arg(ms, ms_precision, 10, QChar('0'));
	}
	else {
		text = QString("%1.%2")
			.arg(seconds)
			.arg(ms, ms_precision, 10, QChar('0'));
	}

	if (ms_precision == 0) { // remove last 2 characters if we dont want milliseconds
		text.chop(2);
	}

	return text;
}

uint64_t Timer::formatTimeToMs(const QString& timeStr)
{
	QString str = timeStr.trimmed();
	int ms = 0, sec = 0, min = 0, hour = 0;

	QStringList parts = str.split('.');
	QString timePart = parts[0];

	if (parts.size() > 1) {
		QString msStr = parts[1];
		if (msStr.length() == 1) ms = msStr.toInt() * 100;
		else if (msStr.length() == 2) ms = msStr.toInt() * 10;
		else ms = msStr.left(3).toInt();
	}

	QStringList timeFields = timePart.split(':');
	if (timeFields.size() == 3) {
		hour = timeFields[0].toInt();
		min = timeFields[1].toInt();
		sec = timeFields[2].toInt();
	}
	else if (timeFields.size() == 2) {
		min = timeFields[0].toInt();
		sec = timeFields[1].toInt();
	}
	else if (timeFields.size() == 1) {
		sec = timeFields[0].toInt();
	}
	else {
		return 0;
	}

	uint64_t totalMs = hour * 3600000ull + min * 60000ull + sec * 1000ull + ms;
	return totalMs;
}

void Timer::setTime(uint64_t time_ms)
{
	accumulatedMs = time_ms;
	label->setText(this->getTimeStr());
}

void Timer::calculateTimeDiffColor(std::chrono::steady_clock::time_point request_time) // moved from Timer::next()
{
	const auto timer_time = this->getTimeMs();
	auto& segment = config->segments[active_segment];
	if (segment.segment_time_diff.text.isEmpty()) {
		segment.segment_time_diff.text = getActiveSegmentTimeDiff(); // segments layout is still gonna be updated, so we dont have to call updateSegmentsTimeDiff here.
		segment.segment_time_diff.color = QColor(0, 128, 255); // if segment_time_diff is empty, its not being counted yet. therefore its a great time.
	} // if segment_time_diff is not empty, then we know that its already within or past one third of the segment
	else if (timer_time < segment.segment_time_ms) { // -time (better time)
		if (active_segment == 0) {
			segment.segment_time_diff.color = timer_time > segment.segment_time_ms * 5 / 6 ? QColor(128, 255, 128) : QColor(0, 255, 0); // todo: add config color variables for that too
		} // if time is within five sixths of avg segment time
		else {
			auto prev_segment_time_ms = config->segments[active_segment - 1].segment_time_ms;
			segment.segment_time_diff.color = timer_time > prev_segment_time_ms + (segment.segment_time_ms - prev_segment_time_ms) * 5 / 6 ? QColor(128, 255, 128) : QColor(0, 255, 0);
		}
	}
	else { // +time (worse time)
		if (active_segment == 0) {
			segment.segment_time_diff.color = timer_time < segment.segment_time_ms * 1 / 6 ? QColor(255, 128, 0) : QColor(255, 0, 0);
		} // if time is within one sixths of avg segment time
		else {
			auto prev_segment_time_ms = config->segments[active_segment - 1].segment_time_ms;
			segment.segment_time_diff.color = timer_time < segment.segment_time_ms + (segment.segment_time_ms > prev_segment_time_ms ? segment.segment_time_ms - prev_segment_time_ms : prev_segment_time_ms) * 1 / 6 ? QColor(255, 128, 0) : QColor(255, 0, 0);
		}
	}
}

Timer* timer = nullptr;
