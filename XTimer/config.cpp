#include "config.h"
#include <qdatetime.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qfileinfo.h>
#include <QFontDatabase.h>
#include "ConfigCreator.h"
#include "timer.h"
#include "window.h"
#include "gradient_label.h"

#define APP_CONFIG_NAME QCoreApplication::applicationDirPath() + "/config.mge"

XTimerConfig::XTimerConfig()
{
	attempts = 0;
	title_name = "XTimer";

	RestoreDefaults();

	try {
		LoadConfig(APP_CONFIG_NAME);
	}
	catch (std::runtime_error& ex) {
		qWarning() << ex.what();
	}

	QObject::connect(qApp, &QCoreApplication::aboutToQuit, []() { // autosave on exit
		config->SaveConfig(APP_CONFIG_NAME);
		});
	
	if (!runFilePath.isEmpty()) {
		try {
			LoadRunConfig(runFilePath);
		}
		catch (std::runtime_error& ex) {
			qWarning() << ex.what();
		}
	}
}

XTimerConfig::~XTimerConfig()
{
}

// ensure that app will not die and continue when we cant read or something
#define IGNORE_EXCEPT(expr) try {expr} catch (...){}

void XTimerConfig::LoadConfig(const QString& file)
{
	if (file.isEmpty()) {
		return;
	}
	vidzhet::ConfigCreator file_(file.toStdString(), Read);

	IGNORE_EXCEPT( window_stays_on_top = file_.read_static<bool>("window_stays_on_top"); )
	IGNORE_EXCEPT( runFilePath = file_.read_static<std::string>("runConfig_file_path").c_str(); )
	IGNORE_EXCEPT( ms_precision = file_.read_static<ushort>("ms_precision"); )
	IGNORE_EXCEPT( window_rounding = file_.read_static<ushort>("window_rounding"); )
	IGNORE_EXCEPT( max_frame_segments = file_.read_static<ushort>("max_frame_segments"); )
	IGNORE_EXCEPT( row_height = file_.read_static<ushort>("row_height"); )
	IGNORE_EXCEPT( window_opacity = file_.read_static<float>("window_opacity"); )
	IGNORE_EXCEPT( window_width = file_.read_static<int>("window_width"); )

	IGNORE_EXCEPT( font = file_.read_static<std::string>("font_file_path").c_str(); )
	IGNORE_EXCEPT( timer_font = file_.read_static<std::string>("timer_font_file_path").c_str(); )
	IGNORE_EXCEPT( timer_font_size = file_.read_static<ushort>("timer_font_size"); )
	IGNORE_EXCEPT( enable_font = file_.read_static<bool>("enable_font"); )
	IGNORE_EXCEPT( enable_bg_img = file_.read_static<bool>("enable_bg_img"); )
	IGNORE_EXCEPT( bg_img_file = file_.read_static<std::string>("bg_img_file").c_str(); )
	IGNORE_EXCEPT( bg_brightness = file_.read_static<float>("bg_brightness"); )

	IGNORE_EXCEPT( timer_label_activeDown = file_.read_static<QRgb>("timer_label_activeDown"); )
	IGNORE_EXCEPT( timer_label_activeUp = file_.read_static<QRgb>("timer_label_activeUp"); )
	IGNORE_EXCEPT( timer_label_pausedDown = file_.read_static<QRgb>("timer_label_pausedDown"); )
	IGNORE_EXCEPT( timer_label_pausedUp = file_.read_static<QRgb>("timer_label_pausedUp"); )
	IGNORE_EXCEPT( timer_label_inactiveDown = file_.read_static<QRgb>("timer_label_inactiveDown"); )
	IGNORE_EXCEPT( timer_label_inactiveUp = file_.read_static<QRgb>("timer_label_inactiveUp"); )
	IGNORE_EXCEPT( timer_label_worseDown = file_.read_static<QRgb>("timer_label_worseDown"); )
	IGNORE_EXCEPT( timer_label_worseUp = file_.read_static<QRgb>("timer_label_worseUp"); )
	IGNORE_EXCEPT( timer_label_finishedDown = file_.read_static<QRgb>("timer_label_finishedDown"); )
	IGNORE_EXCEPT( timer_label_finishedUp = file_.read_static<QRgb>("timer_label_finishedUp"); )
	IGNORE_EXCEPT( active_segment_color = file_.read_static<QRgb>("active_segment_color"); )

	ApplyUpdates();

	file_.close();
}

void XTimerConfig::SaveConfig(const QString& file) {
	if (file.isEmpty()) {
		return;
	}
	vidzhet::ConfigCreator file_(file.toStdString(), Write);

	file_.additem<bool>("window_stays_on_top", window_stays_on_top);
	file_.additem<std::string>("runConfig_file_path", runFilePath.toStdString());
	file_.additem<ushort>("ms_precision", ms_precision);
	file_.additem<ushort>("window_rounding", window_rounding);
	file_.additem<ushort>("max_frame_segments", max_frame_segments);
	file_.additem<ushort>("row_height", row_height);
	file_.additem<float>("window_opacity", window_opacity);
	file_.additem<int>("window_width", window_width);

	file_.additem<std::string>("font_file_path", font.toStdString());
	file_.additem<std::string>("timer_font_file_path", timer_font.toStdString());
	file_.additem<ushort>("timer_font_size", timer_font_size);
	file_.additem<bool>("enable_font", enable_font);
	file_.additem<bool>("enable_bg_img", enable_bg_img);
	file_.additem<std::string>("bg_img_file", bg_img_file.toStdString());
	file_.additem<float>("bg_brightness", bg_brightness);

	file_.additem<QRgb>("timer_label_activeDown", timer_label_activeDown.rgba());
	file_.additem<QRgb>("timer_label_activeUp", timer_label_activeUp.rgba());
	file_.additem<QRgb>("timer_label_pausedDown", timer_label_pausedDown.rgba());
	file_.additem<QRgb>("timer_label_pausedUp", timer_label_pausedUp.rgba());
	file_.additem<QRgb>("timer_label_inactiveDown", timer_label_inactiveDown.rgba());
	file_.additem<QRgb>("timer_label_inactiveUp", timer_label_inactiveUp.rgba());
	file_.additem<QRgb>("timer_label_worseDown", timer_label_worseDown.rgba());
	file_.additem<QRgb>("timer_label_worseUp", timer_label_worseUp.rgba());
	file_.additem<QRgb>("timer_label_finishedDown", timer_label_finishedDown.rgba());
	file_.additem<QRgb>("timer_label_finishedUp", timer_label_finishedUp.rgba());
	file_.additem<QRgb>("active_segment_color", active_segment_color.rgba());

	file_.close();
}

void XTimerConfig::LoadRunConfig(const QString& file)
{
	if (file.isEmpty()) {
		return;
	}
	vidzhet::ConfigCreator file_(file.toStdString(), Read);

	title_name = file_.read_static<std::string>("title_name").c_str();
	attempts = file_.read_static<uint>("attempts");

	auto segment_names = file_.header<std::string>("segment_names");
	auto segment_avg_times = file_.header<uint64_t>("segment_avg_times");

	clearSegments();
	while (segment_names.next() && segment_avg_times.next()) {
		uint64_t segment_avg_time = segment_avg_times.read();
		segments.append({ segment_names.read().c_str(), segment_avg_time});
	}

	if (window) {
		window->updateAttempts();
		window->updateSegmentsLayout(true);
		window->updateTitle(title_name);
	}

	file_.close();
}

void XTimerConfig::SaveRunConfig(const QString& file)
{
	if (file.isEmpty()) {
		return;
	}
	vidzhet::ConfigCreator file_(file.toStdString(), Write);

	file_.additem<std::string>("title_name", title_name.toStdString());
	file_.additem<uint>("attempts", attempts);

	auto segment_names = file_.addheader<std::string>("segment_names");
	for (auto& segment : segments) {
		segment_names.write(segment.name.toStdString());
	}
	segment_names.finalize();
	auto segment_avg_times = file_.addheader<uint64_t>("segment_avg_times");
	for (auto& segment : segments) {
		segment_avg_times.write(segment.segment_time_ms);
	}
	segment_avg_times.finalize();

	file_.close();
}

void XTimerConfig::RestoreDefaults()
{
	window_stays_on_top = true;
	ms_precision = 3;
	window_rounding = 0;
	window_opacity = 1;
	max_frame_segments = 14;
	row_height = 30;
	window_width = 300;

	enable_font = false;
	font.clear();
	timer_font.clear();
	timer_font_size = 40;
	enable_bg_img = false;
	bg_img_file.clear();
	bg_brightness = 0.6f;

	frame_color.setRgb(128, 128, 128);
	timer_label_activeDown.setRgb(0, 255, 64);
	timer_label_activeUp = GradientLabel::colorShift(timer_label_activeDown);
	timer_label_pausedDown.setRgb(128, 255, 128);
	timer_label_pausedUp = GradientLabel::colorShift(timer_label_pausedDown);
	timer_label_inactiveDown.setRgb(200, 200, 200);
	timer_label_inactiveUp = GradientLabel::colorShift(timer_label_inactiveDown);
	timer_label_worseDown.setRgb(255, 0, 0);
	timer_label_worseUp.setRgb(255, 100, 100);
	timer_label_finishedDown.setRgb(0, 255, 255);
	timer_label_finishedUp = GradientLabel::colorShift(timer_label_finishedDown);
	active_segment_color.setRgb(50, 100, 50);

	ApplyUpdates();
}

void XTimerConfig::ApplyUpdates()
{
	if (window) {
		window->setBackgroundImage(bg_img_file);
		window->updateWindowContent();
		window->setWindowFlags(window_stays_on_top ? window->windowFlags() | Qt::WindowStaysOnTopHint : window->windowFlags() & ~Qt::WindowStaysOnTopHint);
		window->setWindowOpacity(window_opacity);
		window->show();
	}
}

void XTimerRunConfigBase::clearSegments()
{
	segments.clear();
}

XTimerConfig* config = nullptr;
