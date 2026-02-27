#include "styles.h"
#include <QApplication>
#include <QMainWindow>

void LoadStylesGlobal(QApplication& app)
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(33, 33, 33));
    palette.setColor(QPalette::Base, QColor(22, 22, 22));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::ToolTipBase, Qt::black);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(33, 33, 33));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Highlight, QColor(0, 160, 255));
    palette.setColor(QPalette::HighlightedText, Qt::black);

    app.setStyle("fusion");
    app.setPalette(palette);

    // Button extra style
    // Menu bar extra style
    app.setStyleSheet(R"(
            QMenuBar::item:selected {
                color: rgb(200, 130, 40); /* hover */
            }

            QMenuBar::item:pressed {
                border-radius: 4px;
                background-color: rgba(111, 111, 111, 50);
            }
            QPushButton:hover {
                background-color: rgba(0, 160, 255, 50);
                border: 1px solid rgb(0, 160, 255);
                border-radius: 4px;
            }
            QPushButton:pressed {
                background-color: rgba(0, 160, 255, 100);
            }
            QPushButton:checked {
                border: 1px solid rgb(0, 160, 255);
                background-color: rgba(0, 160, 255, 50);
                border-radius: 4px;
            }
            QMenu::separator {
                height: 1px;
                background: rgb(44,44,44);
                margin-left: 10px;
                margin-right: 10px;
               }
        )"
    );
}
