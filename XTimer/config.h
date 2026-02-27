#pragma once
#include <QColor>
#include <QList>
#include <Qstring>
#include <QApplication>
#include <QLabel>

class XTimerConfigBase {
public:
	void CopyBase(const XTimerConfigBase& other) {
		operator=(other);
	}
public:
	// general
	bool window_stays_on_top;
	QString runFilePath;
	ushort ms_precision;
	ushort window_rounding;
	ushort max_frame_segments;
	ushort row_height;
	float window_opacity;
	int window_width;

	// theming
	QString font;
	QString timer_font;
	ushort timer_font_size;
	bool enable_font;
	bool enable_bg_img;
	float bg_brightness;
	QString bg_img_file;

	// colors
	QColor frame_color;
	QColor timer_label_activeDown;
	QColor timer_label_activeUp;
	QColor timer_label_pausedDown;
	QColor timer_label_pausedUp;
	QColor timer_label_inactiveDown;
	QColor timer_label_inactiveUp;
	QColor timer_label_worseDown;
	QColor timer_label_worseUp;
	QColor timer_label_finishedDown;
	QColor timer_label_finishedUp;
	QColor active_segment_color;
};

struct RunSegment {
	QString name;
	uint64_t segment_time_ms;
	uint64_t segment_avg_time_ms = 0;
	struct {
		QString text;
		QColor color = QColor(255, 255, 255);
	} segment_time_diff;
	
};

class XTimerRunConfigBase {
public:
	void CopyRunBase(const XTimerRunConfigBase& other) {
		operator=(other);
	}
	void clearSegments();
public:
	QString title_name;
	uint attempts;
	QList<RunSegment> segments;
};

class XTimerConfig : public XTimerConfigBase, public XTimerRunConfigBase {
public:
	XTimerConfig();
	~XTimerConfig();

	void LoadConfig(const QString& file);
	void LoadRunConfig(const QString& file);
	void SaveConfig(const QString& file);
	void SaveRunConfig(const QString& file);
	void RestoreDefaults();
	void ApplyUpdates();
};

extern XTimerConfig* config;
