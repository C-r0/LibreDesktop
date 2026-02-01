#!/usr/bin/env python3
from PyQt6 import QtWidgets, QtCore
from PyQt6.QtGui import QCursor, QIcon
import sys
import math

apps = [
    ("Firefox", "firefox", "/home/cr/LibreDesktop/Launcher/icons/firefox.png"),
    ("Thunar", "thunar", "/home/cr/LibreDesktop/Launcher/icons/thunar.png"),
    ("Terminal", "alacritty", "/home/cr/LibreDesktop/Launcher/icons/terminal.png"),
    ("Rofi", "/home/cr/.local/bin/menu", "/home/cr/LibreDesktop/Launcher/icons/rofi.png"),
    ("X", "", "/home/cr/LibreDesktop/Launcher/icons/x.png"),
]


class RadialLauncher(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowFlags(
            QtCore.Qt.WindowType.FramelessWindowHint
            | QtCore.Qt.WindowType.WindowStaysOnTopHint
            | QtCore.Qt.WindowType.Tool
        )
        self.setAttribute(QtCore.Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(QtCore.Qt.WidgetAttribute.WA_DeleteOnClose)

        self.window_size = 300
        self.resize(self.window_size, self.window_size)

        pos = QCursor.pos()
        self.center_x = self.window_size // 2
        self.center_y = self.window_size // 2
        self.move(pos.x() - self.center_x, pos.y() - self.center_y)

        self.buttons = []
        self.animations = []
        self.is_closing = False

        self.create_radial_buttons(animated=True)

        self.outside_timer = QtCore.QTimer(self)
        self.outside_timer.timeout.connect(self.check_click_outside)
        self.outside_timer.start(50)

    def create_radial_buttons(self, animated=False):
        N = len(apps)
        radius = 100
        button_size = 60

        for i, (name, cmd, icon_path) in enumerate(apps):
            btn = QtWidgets.QPushButton("", self)
            btn.setGeometry(
                self.center_x - button_size // 2,
                self.center_y - button_size // 2,
                button_size,
                button_size,
            )
            btn.setIcon(QIcon(icon_path))
            btn.setIconSize(QtCore.QSize(button_size - 10, button_size - 10))
            btn.setStyleSheet(
                f"""
                QPushButton {{
                    background-color: #3498db;
                    border-radius: {button_size // 2}px;
                }}
                QPushButton:hover {{
                    background-color: #2980b9;
                }}
                """)
            btn.clicked.connect(lambda checked, c=cmd: (QtCore.QProcess.startDetached(c), self.start_fade_out()))
            self.buttons.append(btn)

            opacity_effect = QtWidgets.QGraphicsOpacityEffect()
            opacity_effect.setOpacity(0)
            btn.setGraphicsEffect(opacity_effect)
            btn.opacity_effect = opacity_effect

            if animated:
                angle = 2 * math.pi * i / N
                final_x = self.center_x + radius * math.cos(angle) - button_size / 2
                final_y = self.center_y + radius * math.sin(angle) - button_size / 2

                anim_geo = QtCore.QPropertyAnimation(btn, b"geometry")
                anim_geo.setDuration(500)
                anim_geo.setStartValue(
                    QtCore.QRect(
                        self.center_x - button_size // 2,
                        self.center_y - button_size // 2,
                        button_size,
                        button_size,
                    )
                )
                anim_geo.setEndValue(
                    QtCore.QRect(int(final_x), int(final_y), button_size, button_size)
                )
                anim_geo.setEasingCurve(QtCore.QEasingCurve.Type.OutBack)
                anim_geo.start()
                self.animations.append(anim_geo)

                anim_opacity = QtCore.QPropertyAnimation(opacity_effect, b"opacity")
                anim_opacity.setDuration(500)
                anim_opacity.setStartValue(0)
                anim_opacity.setEndValue(1)
                anim_opacity.start()
                self.animations.append(anim_opacity)

    def start_fade_out(self):
        if self.is_closing:
            return

        self.is_closing = True
        self.outside_timer.stop()
        duration = 300

        for btn in self.buttons:
            geo_anim = QtCore.QPropertyAnimation(btn, b"geometry")
            geo_anim.setDuration(duration)
            geo_anim.setStartValue(btn.geometry())
            geo_anim.setEndValue(
                QtCore.QRect(
                    self.center_x - btn.width() // 2,
                    self.center_y - btn.height() // 2,
                    btn.width(),
                    btn.height(),
                )
            )
            geo_anim.setEasingCurve(QtCore.QEasingCurve.Type.InBack)
            geo_anim.start()
            self.animations.append(geo_anim)

            opacity_anim = QtCore.QPropertyAnimation(btn.opacity_effect, b"opacity")
            opacity_anim.setDuration(duration)
            opacity_anim.setStartValue(btn.opacity_effect.opacity())
            opacity_anim.setEndValue(0)
            opacity_anim.start()
            self.animations.append(opacity_anim)

        QtCore.QTimer.singleShot(duration, self.close)

    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key.Key_Escape:
            self.start_fade_out()

    def check_click_outside(self):
        if self.is_closing:
            return
        if not self.geometry().contains(QCursor.pos()):
            self.start_fade_out()

    def closeEvent(self, event):
        self.outside_timer.stop()
        self.animations.clear()
        QtWidgets.QApplication.quit()
        event.accept()


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    launcher = RadialLauncher()
    launcher.show()
    sys.exit(app.exec())
