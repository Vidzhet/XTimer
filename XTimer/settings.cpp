#include "settings.h"
#include <qlayout.h>
#include <QLabel.h>
#include <qdialogbuttonbox.h>
#include <qframe.h>
#include <qtabwidget.h>
#include <qlistwidget.h>
#include <qscrollarea.h>
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QColorDialog>
#include <QTableWidget>
#include <QHeaderView>
#include "config.h"
#include "timer.h"
#include "window.h"

XTimerSettings::XTimerSettings(QWidget* parent) : QDialog(parent)
{
	settingsConfig = *config;
	settingsRunConfig = *config;

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	QTableWidget* table = new QTableWidget(this); // used for segments editing in "Run Configuration" page
	auto parseAndSaveSegments = [this, table]() {
		settingsRunConfig.clearSegments();
		for (int i = 0; i < table->rowCount(); i++) {
			settingsRunConfig.segments.append({ table->item(i, 0)->text(), Timer::formatTimeToMs(table->item(i, 1)->text()) });
		}
		};

	connect(buttons, &QDialogButtonBox::accepted, this, [this, parseAndSaveSegments]() {
		parseAndSaveSegments();
		config->CopyRunBase(settingsRunConfig);
		config->CopyBase(settingsConfig);
		config->ApplyUpdates();
		QDialog::reject(); // exec() returns rejected = proper exit, if accepted - window wants to be recreated. if you can refactor it in a way that its just rebuilding the content, please do
		});
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QPushButton* loadconfig_button = new QPushButton("Load Config", this);
	QPushButton* restoredefaults_button = new QPushButton("Restore Defaults", this);

	connect(loadconfig_button, &QPushButton::clicked, this, [&]() {
		config->LoadConfig(QFileDialog::getOpenFileName(this, "Open Config", QCoreApplication::applicationDirPath(), "MGE File (*.mge);;All Files (*)"));
		//settingsConfig = *config;
		accept();
		});
	connect(restoredefaults_button, &QPushButton::clicked, this, [&]() {
		config->RestoreDefaults();
		//settingsConfig = *config;
		accept();
		});

	QListWidget* tabs = new QListWidget(this);
	tabs->addItem("General");
	tabs->addItem("Run Configuration");
	tabs->addItem("Custom Theming");
	tabs->addItem("Colors");
	tabs->setFixedWidth(120);

	QStackedWidget* stack = new QStackedWidget(this);
	connect(tabs, &QListWidget::currentRowChanged, stack, &QStackedWidget::setCurrentIndex);
	tabs->setCurrentRow(0);

	QWidget* pageGeneral = new QWidget();
	QVBoxLayout* generalVL = new QVBoxLayout(pageGeneral);

	QGroupBox* configGroup = new QGroupBox("Config");
	QVBoxLayout* configGroupVL = new QVBoxLayout(configGroup);

	QCheckBox* window_stays_on_top_checkBox = new QCheckBox("Window stays on top");
	window_stays_on_top_checkBox->setCheckState(settingsConfig.window_stays_on_top ? Qt::Checked : Qt::Unchecked);
	connect(window_stays_on_top_checkBox, &QCheckBox::toggled, this, [this](bool checked) {
		settingsConfig.window_stays_on_top = checked;
		});
	configGroupVL->addWidget(window_stays_on_top_checkBox);

	QLineEdit* run_file_input = new QLineEdit(settingsConfig.runFilePath, this);
	connect(run_file_input, &QLineEdit::textChanged, this, [this](const QString& text) {
		settingsConfig.runFilePath = text;
		});

	QPushButton* run_file_browseButton = new QPushButton("Browse", this);
	connect(run_file_browseButton, &QPushButton::clicked, this, [this, run_file_input]() {
		settingsConfig.runFilePath = QFileDialog::getOpenFileName(this, "Open Run Config", QCoreApplication::applicationDirPath(), "MGE File (*.mge);;All Files (*)");
		run_file_input->setText(settingsConfig.runFilePath);
		});

	QHBoxLayout* run_fileHL = new QHBoxLayout();
	run_fileHL->addWidget(new QLabel("Run Config"));
	run_fileHL->addWidget(run_file_input);
	run_fileHL->addWidget(run_file_browseButton);
	configGroupVL->addLayout(run_fileHL);

	QGroupBox* windowGroup = new QGroupBox("Window");
	QVBoxLayout* windowGroupVL = new QVBoxLayout(windowGroup);

	QSpinBox* window_rounding_spinBox = new QSpinBox();
	window_rounding_spinBox->setRange(0, 50);
	window_rounding_spinBox->setValue(settingsConfig.window_rounding);
	connect(window_rounding_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsConfig.window_rounding = value;
		});

	QHBoxLayout* window_roundingHL = new QHBoxLayout();
	window_roundingHL->addWidget(new QLabel("Window rounding"));
	window_roundingHL->addWidget(window_rounding_spinBox);
	windowGroupVL->addLayout(window_roundingHL);

	QDoubleSpinBox* window_opacity_spinBox = new QDoubleSpinBox();
	window_opacity_spinBox->setRange(0, 1);
	window_opacity_spinBox->setSingleStep(0.1);
	window_opacity_spinBox->setDecimals(2);
	window_opacity_spinBox->setValue(settingsConfig.window_opacity);
	connect(window_opacity_spinBox, &QDoubleSpinBox::valueChanged, this, [this](double value) {
		settingsConfig.window_opacity = value;
		});

	QHBoxLayout* window_opacityHL = new QHBoxLayout();
	window_opacityHL->addWidget(new QLabel("Window opacity"));
	window_opacityHL->addWidget(window_opacity_spinBox);
	windowGroupVL->addLayout(window_opacityHL);

	QSpinBox* window_width_spinBox = new QSpinBox();
	window_width_spinBox->setRange(0, 10000);
	window_width_spinBox->setValue(settingsConfig.window_width);
	connect(window_width_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsConfig.window_width = value;
		});

	QHBoxLayout* window_widthHL = new QHBoxLayout();
	window_widthHL->addWidget(new QLabel("Window width"));
	window_widthHL->addWidget(window_width_spinBox);
	windowGroupVL->addLayout(window_widthHL);

	QGroupBox* segmentsGroup = new QGroupBox("Segments");
	QVBoxLayout* segmentsGroupVL = new QVBoxLayout(segmentsGroup);

	QSpinBox* max_segments_spinBox = new QSpinBox();
	max_segments_spinBox->setRange(0, 1000);
	max_segments_spinBox->setValue(settingsConfig.max_frame_segments);
	connect(max_segments_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsConfig.max_frame_segments = value;
		});

	QHBoxLayout* max_segmentsHL = new QHBoxLayout();
	max_segmentsHL->addWidget(new QLabel("Maximum segments in frame"));
	max_segmentsHL->addWidget(max_segments_spinBox);
	segmentsGroupVL->addLayout(max_segmentsHL);

	QSpinBox* segment_height_spinBox = new QSpinBox();
	segment_height_spinBox->setRange(0, 100);
	segment_height_spinBox->setValue(settingsConfig.row_height);
	connect(segment_height_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsConfig.row_height = value;
		});

	QHBoxLayout* segment_heightHL = new QHBoxLayout();
	segment_heightHL->addWidget(new QLabel("Segment row height"));
	segment_heightHL->addWidget(segment_height_spinBox);
	segmentsGroupVL->addLayout(segment_heightHL);

	QSpinBox* ms_precision_spinBox = new QSpinBox();
	ms_precision_spinBox->setRange(0, 3);
	ms_precision_spinBox->setValue(settingsConfig.ms_precision);
	connect(ms_precision_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsConfig.ms_precision = value;
		});

	QHBoxLayout* ms_precisionHL = new QHBoxLayout();
	ms_precisionHL->addWidget(new QLabel("Milliseconds precision (.0f - .3f)"));
	ms_precisionHL->addWidget(ms_precision_spinBox);
	segmentsGroupVL->addLayout(ms_precisionHL);

	generalVL->addWidget(configGroup);
	generalVL->addWidget(windowGroup);
	generalVL->addWidget(segmentsGroup);
	generalVL->addStretch();


	QWidget* pageRunConfig = new QWidget();
	QVBoxLayout* runConfigVL = new QVBoxLayout(pageRunConfig);

	QHBoxLayout* saveButtonsHL = new QHBoxLayout();
	QPushButton* runConfigSaveButton = new QPushButton("Save", this);
	connect(runConfigSaveButton, &QPushButton::clicked, this, [this, parseAndSaveSegments]() {
		parseAndSaveSegments();
		config->CopyRunBase(settingsRunConfig);
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
	QPushButton* runConfigSaveAsButton = new QPushButton("Save As", this);
	connect(runConfigSaveAsButton, &QPushButton::clicked, this, [this, parseAndSaveSegments]() {
		parseAndSaveSegments();
		config->CopyRunBase(settingsRunConfig);
		QString file_path = QFileDialog::getSaveFileName(this, "Save Run Config", QCoreApplication::applicationDirPath(), "MGE File (*.mge);;All Files (*)");
		if (!file_path.isEmpty()) {
			config->SaveRunConfig(file_path);
			config->runFilePath = file_path;
		}
		});

	saveButtonsHL->addWidget(new QLabel("Timer Layout"));
	saveButtonsHL->addStretch();
	saveButtonsHL->addWidget(runConfigSaveButton);
	saveButtonsHL->addWidget(runConfigSaveAsButton);

	runConfigVL->addLayout(saveButtonsHL);

	QGroupBox* runConfigGroup = new QGroupBox(); 
	runConfigGroup->setStyleSheet("QGroupBox { padding-top: 0px; margin-top: 0px; }"); // very dirty and bad, i did this to put save buttons on the top
	QVBoxLayout* runConfigGroupVL = new QVBoxLayout(runConfigGroup);

	QHBoxLayout* titleHL = new QHBoxLayout();
	titleHL->addWidget(new QLabel("Title"));
	QLineEdit* title_input = new QLineEdit(settingsRunConfig.title_name, this);
	connect(title_input, &QLineEdit::textChanged, this, [this](const QString& text) {
		settingsRunConfig.title_name = text;
		});
	titleHL->addWidget(title_input);
	titleHL->addWidget(new QLabel("Attempts"));
	QSpinBox* attempts_spinBox = new QSpinBox();
	attempts_spinBox->setRange(0, 999999);
	attempts_spinBox->setValue(settingsRunConfig.attempts);
	connect(attempts_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsRunConfig.attempts = value;
		});
	titleHL->addWidget(attempts_spinBox);

	runConfigGroupVL->addLayout(titleHL);

	//QTableWidget* table = new QTableWidget(this);
	table->setColumnCount(5);

	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Custom);
	table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Custom);
	table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Custom);
	table->horizontalHeader()->setMinimumSectionSize(20);
	table->horizontalHeader()->resizeSection(2, 20);
	table->horizontalHeader()->resizeSection(3, 20);
	table->horizontalHeader()->resizeSection(4, 30);

	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setVisible(false);
	table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	//table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setDragDropOverwriteMode(false);
	table->setDropIndicatorShown(false);
	table->setDragEnabled(false);
	table->setAcceptDrops(false);
	table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
	table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto addRow = [this, table](const QString& name, const QString& time) {
		int row = table->rowCount();
		table->insertRow(row);

		table->setItem(row, 0, new QTableWidgetItem(name));
		table->setItem(row, 1, new QTableWidgetItem(time));

		QPushButton* upBtn = new QPushButton("↑");
		upBtn->setFixedWidth(20);
		table->setCellWidget(row, 2, upBtn);

		QObject::connect(upBtn, &QPushButton::clicked, [table, upBtn]() mutable {
			int row = -1;
			for (int r = 0; r < table->rowCount(); ++r) {
				if (table->cellWidget(r, 2) == upBtn) { row = r; break; }
			}
			if (row <= 0) { return; }

			table->insertRow(row - 1);
			for (int c = 0; c < table->columnCount(); ++c) {
				table->setItem(row - 1, c, table->takeItem(row + 1, c));
				table->setCellWidget(row - 1, c, table->cellWidget(row + 1, c));
			}
			table->removeRow(row + 1);
			});

		QPushButton* downBtn = new QPushButton("↓");
		downBtn->setFixedWidth(20);
		table->setCellWidget(row, 3, downBtn);

		QObject::connect(downBtn, &QPushButton::clicked, [table, downBtn]() mutable {
			int row = -1;
			for (int r = 0; r < table->rowCount(); ++r) {
				if (table->cellWidget(r, 3) == downBtn) { row = r; break; }
			}
			if (row < 0 || row >= table->rowCount() - 1) { return; }

			table->insertRow(row + 2);
			for (int c = 0; c < table->columnCount(); ++c) {
				table->setItem(row + 2, c, table->takeItem(row, c));
				table->setCellWidget(row + 2, c, table->cellWidget(row, c));
			}
			table->removeRow(row);
			});

		QPushButton* removeBtn = new QPushButton("×");
		removeBtn->setFixedWidth(30);
		table->setCellWidget(row, 4, removeBtn);

		QObject::connect(removeBtn, &QPushButton::clicked, [table, removeBtn]() {
			for (int i = 0; i < table->rowCount(); ++i) {
				if (table->cellWidget(i, 4) == removeBtn) {
					table->removeRow(i);
				}
			}
			});
		};

	for (auto& segment : settingsRunConfig.segments) {
		addRow(segment.name, Timer::formatTimeToStr(segment.segment_time_ms, 2));
	}

	runConfigGroupVL->addWidget(table, 1);

	QHBoxLayout* segmentButtonsHL = new QHBoxLayout();

	QPushButton* addSegmentButton = new QPushButton("Add Segment", this);
	connect(addSegmentButton, &QPushButton::clicked, this, [this, addRow, table]() {
		addRow("New segment", Timer::formatTimeToStr(0, 2));
		table->scrollToBottom();
		});
	QPushButton* clearSegmentsButton = new QPushButton("Clear", this);
	connect(clearSegmentsButton, &QPushButton::clicked, this, [table]() {
		for (int r = 0; r < table->rowCount(); ++r) {
			for (int c = 0; c < table->columnCount(); ++c) {
				QWidget* w = table->cellWidget(r, c);
				if (w) {
					table->removeCellWidget(r, c);
					delete w;
				}
			}
		}
		table->setRowCount(0);
		});
	segmentButtonsHL->addWidget(addSegmentButton);
	segmentButtonsHL->addWidget(clearSegmentsButton);
	runConfigGroupVL->addLayout(segmentButtonsHL);

	runConfigVL->addWidget(runConfigGroup, 1);
	runConfigVL->addStretch();

	QWidget* pageTheming = new QWidget();
	QVBoxLayout* themingVL = new QVBoxLayout(pageTheming);

	QGroupBox* fontGroup = new QGroupBox("Custom Font");
	fontGroup->setCheckable(true);
	fontGroup->setChecked(settingsConfig.enable_font);
	connect(fontGroup, &QGroupBox::toggled, this, [this](bool checked) {
		settingsConfig.enable_font = checked;
		});
	QVBoxLayout* fontGroupVL = new QVBoxLayout(fontGroup);

	QLineEdit* font_file_input = new QLineEdit(settingsConfig.font, this);
	connect(font_file_input, &QLineEdit::textChanged, this, [this](const QString& text) {
		settingsConfig.font = text;
		});

	QPushButton* font_file_browseButton = new QPushButton("Browse", this);
	connect(font_file_browseButton, &QPushButton::clicked, this, [this, font_file_input]() {
		settingsConfig.font = QFileDialog::getOpenFileName(this, "Open Font", QCoreApplication::applicationDirPath(), "TTF File (*.ttf);;All Files (*)");
		font_file_input->setText(settingsConfig.font);
		});

	QHBoxLayout* font_fileHL = new QHBoxLayout();
	QLabel* font_file_label = new QLabel("Font file");
	font_file_label->setFixedWidth(100);
	font_fileHL->addWidget(font_file_label);
	font_fileHL->addWidget(font_file_input);
	font_fileHL->addWidget(font_file_browseButton);
	fontGroupVL->addLayout(font_fileHL);

	QLineEdit* timer_font_file_input = new QLineEdit(settingsConfig.timer_font, this);
	connect(timer_font_file_input, &QLineEdit::textChanged, this, [this](const QString& text) {
		settingsConfig.timer_font = text;
		});

	QPushButton* timer_font_file_browseButton = new QPushButton("Browse", this);
	connect(timer_font_file_browseButton, &QPushButton::clicked, this, [this, timer_font_file_input]() {
		settingsConfig.timer_font = QFileDialog::getOpenFileName(this, "Open Font", QCoreApplication::applicationDirPath(), "TTF File (*.ttf);;All Files (*)");
		timer_font_file_input->setText(settingsConfig.timer_font);
		});

	QHBoxLayout* timer_font_fileHL = new QHBoxLayout();
	QLabel* timer_font_file_label = new QLabel("Timer font file");
	timer_font_file_label->setFixedWidth(100);
	timer_font_fileHL->addWidget(timer_font_file_label);
	timer_font_fileHL->addWidget(timer_font_file_input);
	timer_font_fileHL->addWidget(timer_font_file_browseButton);
	fontGroupVL->addLayout(timer_font_fileHL);

	QSpinBox* timer_font_size_spinBox = new QSpinBox();
	timer_font_size_spinBox->setRange(0, 100);
	timer_font_size_spinBox->setValue(settingsConfig.timer_font_size);
	connect(timer_font_size_spinBox, &QSpinBox::valueChanged, this, [this](int value) {
		settingsConfig.timer_font_size = value;
		});

	QHBoxLayout* timer_font_sizeHL = new QHBoxLayout();
	timer_font_sizeHL->addWidget(new QLabel("Timer font size"));
	timer_font_sizeHL->addWidget(timer_font_size_spinBox);
	fontGroupVL->addLayout(timer_font_sizeHL);

	QGroupBox* bgGroup = new QGroupBox("Custom Background");
	bgGroup->setCheckable(true);
	bgGroup->setChecked(settingsConfig.enable_bg_img);
	connect(bgGroup, &QGroupBox::toggled, this, [this](bool checked) {
		settingsConfig.enable_bg_img = checked;
		});
	QVBoxLayout* bgGroupVL = new QVBoxLayout(bgGroup);

	QLineEdit* bg_file_input = new QLineEdit(settingsConfig.bg_img_file, this);
	connect(bg_file_input, &QLineEdit::textChanged, this, [this](const QString& text) {
		settingsConfig.bg_img_file = text;
		});

	QPushButton* bg_file_browseButton = new QPushButton("Browse", this);
	connect(bg_file_browseButton, &QPushButton::clicked, this, [this, bg_file_input]() { // probably not the best practice to specify image file types manually, need to find a way to filter all supported image types
		settingsConfig.bg_img_file = QFileDialog::getOpenFileName(this, "Open Image", QCoreApplication::applicationDirPath(), "PNG File (*.png);JPEG File (*.jpg);;All Files (*)");
		bg_file_input->setText(settingsConfig.bg_img_file);
		});

	QHBoxLayout* bg_fileHL = new QHBoxLayout();
	bg_fileHL->addWidget(new QLabel("Image file"));
	bg_fileHL->addWidget(bg_file_input);
	bg_fileHL->addWidget(bg_file_browseButton);
	bgGroupVL->addLayout(bg_fileHL);

	QDoubleSpinBox* bg_brightness_spinBox = new QDoubleSpinBox();
	bg_brightness_spinBox->setRange(0, 1);
	bg_brightness_spinBox->setSingleStep(0.1);
	bg_brightness_spinBox->setDecimals(2);
	bg_brightness_spinBox->setValue(settingsConfig.bg_brightness);
	connect(bg_brightness_spinBox, &QDoubleSpinBox::valueChanged, this, [this](double value) {
		settingsConfig.bg_brightness = value;
		});

	QHBoxLayout* bg_brightnessHL = new QHBoxLayout();
	bg_brightnessHL->addWidget(new QLabel("Background brightness"));
	bg_brightnessHL->addWidget(bg_brightness_spinBox);
	bgGroupVL->addLayout(bg_brightnessHL);

	themingVL->addWidget(fontGroup);
	themingVL->addWidget(bgGroup);
	themingVL->addStretch();

	QWidget* pageColors = new QWidget();
	QVBoxLayout* colorsVL = new QVBoxLayout(pageColors);

	QGroupBox* colorsGroup = new QGroupBox("Colors");
	QVBoxLayout* colorsGroupVL = new QVBoxLayout(colorsGroup);

	auto colorInput = [this](const char* name, QColor* down, QColor* up = nullptr) -> QHBoxLayout* {
		QHBoxLayout* hl = new QHBoxLayout();
		QPushButton* btn_color_down = new QPushButton(this);
		btn_color_down->setStyleSheet(QString("background:%1;").arg(down->name(QColor::HexArgb)));
		connect(btn_color_down, &QPushButton::clicked, this, [this, btn_color_down, down]() {
			QColor col = QColorDialog::getColor(*down, this, "Select color", QColorDialog::ShowAlphaChannel);
			if (col.isValid()) {
				*down = col;
				btn_color_down->setStyleSheet(QString("background:%1;").arg(col.name(QColor::HexArgb)));
			}
			});
		QPushButton* btn_color_up = nullptr;
		if (up) {
			btn_color_up = new QPushButton(this);
			btn_color_up->setStyleSheet(QString("background:%1;").arg(up->name(QColor::HexArgb)));
			connect(btn_color_up, &QPushButton::clicked, this, [this, btn_color_up, up]() {
				QColor col = QColorDialog::getColor(*up, this, "Select color", QColorDialog::ShowAlphaChannel);
				if (col.isValid()) {
					*up = col;
					btn_color_up->setStyleSheet(QString("background:%1;").arg(col.name(QColor::HexArgb)));
				}
			});
		}
		hl->addWidget(new QLabel(name, this));
		hl->addStretch();
		hl->addWidget(new QLabel(up ? "Down:" : "Color:", this));
		hl->addWidget(btn_color_down);
		if (up) {
			hl->addWidget(new QLabel("Up:", this));
			hl->addWidget(btn_color_up);
		}
		return hl;
	};

	colorsGroupVL->addLayout(colorInput("Active segment", &settingsConfig.active_segment_color));
	colorsGroupVL->addLayout(colorInput("Timer active", &settingsConfig.timer_label_activeDown, &settingsConfig.timer_label_activeUp));
	colorsGroupVL->addLayout(colorInput("Timer paused", &settingsConfig.timer_label_pausedDown, &settingsConfig.timer_label_pausedUp));
	colorsGroupVL->addLayout(colorInput("Timer inactive", &settingsConfig.timer_label_inactiveDown, &settingsConfig.timer_label_inactiveUp));
	colorsGroupVL->addLayout(colorInput("Timer worse", &settingsConfig.timer_label_worseDown, &settingsConfig.timer_label_worseUp));
	colorsGroupVL->addLayout(colorInput("Timer finished", &settingsConfig.timer_label_finishedDown, &settingsConfig.timer_label_finishedUp));

	colorsVL->addWidget(colorsGroup);
	colorsVL->addStretch();

	auto pageWithScroll = [](QWidget* page) -> QScrollArea* {
		QScrollArea* pageScroll = new QScrollArea();
		pageScroll->setWidgetResizable(true);
		pageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		pageScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		pageScroll->setWidget(page);
		return pageScroll;
		};

	stack->addWidget(pageWithScroll(pageGeneral));
	stack->addWidget(pageWithScroll(pageRunConfig));
	stack->addWidget(pageWithScroll(pageTheming));
	stack->addWidget(pageWithScroll(pageColors));


	QVBoxLayout* mainVL = new QVBoxLayout(this);
	QHBoxLayout* mainHL = new QHBoxLayout(this);
	mainHL->addWidget(tabs);
	mainHL->addWidget(stack);
	mainVL->addLayout(mainHL);
	QHBoxLayout* buttonsHL = new QHBoxLayout(this);
	buttonsHL->addWidget(loadconfig_button);
	buttonsHL->addWidget(restoredefaults_button);
	buttonsHL->addStretch();
	buttonsHL->addWidget(buttons);
	mainVL->addLayout(buttonsHL);
}
