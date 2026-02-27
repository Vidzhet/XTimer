#pragma once
#include "config.h"
#include <QDialog>
#include <QFrame>

class XTimerSettings : public QDialog {
    Q_OBJECT
public:
	explicit XTimerSettings(QWidget* parent = nullptr);
private:
	XTimerConfigBase settingsConfig;
	XTimerRunConfigBase settingsRunConfig;
};
