#pragma once
#include <QElapsedTimer>
#include <QLabel>
#include <QTimer>

class GradientLabel;

class Timer {
public:
	Timer();
	QLabel* getLabel();
	int getActiveSegment(); // returns an index of active segment (starts with 0)
	QString getActiveSegmentTimeDiff();
	static QString formatTimeToStr(uint64_t time_ms, ushort ms_precision);
	static uint64_t formatTimeToMs(const QString& timeStr);
public: // exported methods
	void start(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	void reset();
	void pause(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	void resume(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	void next(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	void prev(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	uint64_t getTimeMs(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	QString getTimeStr(std::chrono::steady_clock::time_point request_time = std::chrono::steady_clock::now());
	void setTime(uint64_t time_ms);
	bool isRunning();
private:
	void calculateTimeDiffColor(std::chrono::steady_clock::time_point request_time);
private:
	GradientLabel* label;
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::duration pausedDuration{};
	bool running = false;
	qint64 accumulatedMs = 0;
	qsizetype active_segment = 0;
};

extern Timer* timer;
