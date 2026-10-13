#include "app/Application.h"
#include "ui/Window.h"

#include <QStyle>

Application::Application(int& argc, char** argv) : QApplication(argc, argv) {
    setApplicationName("BattleShips");
    setApplicationDisplayName("Морской бой");
    setOrganizationName("Robshak");
    setApplicationVersion("1.1.2");
    setStyle("Fusion");
    setPalette(style()->standardPalette());
    setStyleSheet(
        "QWidget { font-size: 13px; } QGroupBox { font-weight: bold; } "
        "QPushButton { padding: 5px 10px; } QComboBox { min-height: 26px; } "
        "QProgressBar { border: 1px solid #b8c9d8; border-radius: 3px; background: #f4f8fc; } "
        "QProgressBar::chunk { background: #8eafc9; }");
}

int Application::run(int argc, char** argv) {
    Application application(argc, argv);
    Window window;
    window.show();
    return application.exec();
}
