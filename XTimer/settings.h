#pragma once
#include <qdialog.h>
#include <QFrame>
#include "config.h"

class XTimerSettings : public QDialog {
    Q_OBJECT
public:
	explicit XTimerSettings(QWidget* parent = nullptr);
private:
	XTimerConfigBase settingsConfig;
	XTimerRunConfigBase settingsRunConfig;
};