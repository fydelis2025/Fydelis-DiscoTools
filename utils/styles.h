#ifndef STYLES_H
#define STYLES_H

#include <QString>
#include <QColor>

namespace FydelisTheme {

inline const QColor SIDEBAR_BG       = QColor(26, 26, 46);
inline const QColor SIDEBAR_HOVER    = QColor(22, 33, 62);
inline const QColor SIDEBAR_SELECTED = QColor(15, 52, 96);
inline const QColor ACCENT           = QColor(0, 180, 216);
inline const QColor ACCENT_LIGHT   = QColor(72, 202, 228);  // #48cae4
inline const QColor SUCCESS          = QColor(6, 214, 160);
inline const QColor WARNING          = QColor(255, 209, 102);
inline const QColor DANGER           = QColor(239, 71, 111);
inline const QColor CARD_BG          = QColor(30, 30, 47);
inline const QColor BG_PRIMARY       = QColor(22, 22, 35);
inline const QColor TEXT_PRIMARY     = QColor(224, 224, 224);
inline const QColor TEXT_SECONDARY   = QColor(160, 160, 176);

inline QString globalStyleSheet() {
    return R"(
        QWidget { font-family: "Segoe UI","Noto Sans",sans-serif; font-size: 13px; color: #e0e0e0; }
        QMainWindow { background-color: #161623; }
        QScrollBar:vertical { background:#161623; width:10px; border:none; }
        QScrollBar::handle:vertical { background:#0f3460; border-radius:5px; min-height:30px; }
        QScrollBar::handle:vertical:hover { background:#00b4d8; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }
    )";
}

inline QString sidebarButtonStyle(bool selected = false) {
    return QString(R"(
        QPushButton {
            background-color: %1; color: #e0e0e0; border: none;
            border-radius: 8px; padding: 12px 20px; text-align: left;
            font-size: 14px; font-weight: %2;
        }
        QPushButton:hover { background-color: #16213e; border-left: 3px solid #00b4d8; }
        QPushButton:pressed { background-color: #0f3460; }
    )").arg(selected ? "#0f3460" : "transparent").arg(selected ? "bold" : "normal");
}

inline QString cardStyle() {
    return R"(
        QFrame#Card { background-color: #1e1e2f; border-radius: 12px;
                      border: 1px solid #2a2a3f; padding: 16px; }
        QFrame#Card:hover { border: 1px solid #00b4d8; }
    )";
}

inline QString primaryButtonStyle() {
    return R"(
        QPushButton { background-color: #00b4d8; color: #161623; border: none;
                      border-radius: 8px; padding: 10px 24px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background-color: #48cae4; }
        QPushButton:disabled { background-color: #2a2a3f; color: #5a5a6a; }
    )";
}

inline QString dangerButtonStyle() {
    return R"(
        QPushButton { background-color: #ef476f; color: white; border: none;
                      border-radius: 8px; padding: 10px 24px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background-color: #ff5a7a; }
    )";
}

inline QString comboBoxStyle() {
    return R"(
        QComboBox { background-color: #1a1a2e; color: #e0e0e0;
                    border: 1px solid #2a2a3f; border-radius: 6px; padding: 8px 12px; font-size: 13px; }
        QComboBox:hover { border: 1px solid #00b4d8; }
        QComboBox QAbstractItemView { background-color: #1a1a2e; color: #e0e0e0;
                                      border: 1px solid #00b4d8; selection-background-color: #0f3460; }
    )";
}

inline QString progressBarStyle() {
    return R"(
        QProgressBar { background-color: #1e1e2f; border: none; border-radius: 6px;
                       height: 12px; text-align: center; font-size: 10px; color: #e0e0e0; }
        QProgressBar::chunk { background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #00b4d8, stop:1 #48cae4); border-radius: 6px; }
    )";
}

inline QString textEditStyle() {
    return R"(
        QTextEdit, QPlainTextEdit { background-color: #1a1a2e; color: #e0e0e0;
            border: 1px solid #2a2a3f; border-radius: 6px; padding: 8px;
            font-family: "Cascadia Code","Fira Code","JetBrains Mono",monospace; font-size: 12px; }
    )";
}

} // namespace

#endif
