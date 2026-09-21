"""
Improved Navigation System - Simple & Intuitive

Système de Navigation Amélioré - Simple et Intuitif
"""

from PyQt6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QTabWidget,
                             QPushButton, QLabel, QStackedWidget)
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QFont, QColor, QIcon

from ui.translations_fr import translate as tr


class NavTab:
    """Navigation tab definition"""
    def __init__(self, tab_id: str, icon: str, label_en: str, label_fr: str):
        self.tab_id = tab_id
        self.icon = icon
        self.label_en = label_en
        self.label_fr = label_fr

    def get_label(self, language: str = "fr") -> str:
        return self.label_fr if language == "fr" else self.label_en


class NavigationBar(QWidget):
    """Simplified navigation bar with clear indicators"""

    tab_changed = pyqtSignal(str)  # Emits tab_id

    def __init__(self, language: str = "fr"):
        super().__init__()
        self.language = language
        self.current_tab = "dashboard"
        self.setup_ui()

    def setup_ui(self):
        """Setup navigation bar"""
        layout = QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(5)

        # Navigation tabs
        self.tabs = {
            "dashboard": NavTab("dashboard", "📊", "Dashboard", "Tableau de Bord"),
            "audits": NavTab("audits", "📋", "Audits", "Audits"),
            "attacks": NavTab("attacks", "⚡", "Attacks", "Attaques"),
            "results": NavTab("results", "📈", "Results", "Résultats"),
            "settings": NavTab("settings", "⚙️", "Settings", "Paramètres"),
        }

        self.buttons = {}

        for tab_id, tab in self.tabs.items():
            btn = self.create_nav_button(tab)
            self.buttons[tab_id] = btn
            layout.addWidget(btn)

            btn.clicked.connect(lambda checked, tid=tab_id: self.on_tab_click(tid))

        # Spacer
        layout.addStretch()

        self.setLayout(layout)
        self.setStyleSheet(self._get_nav_stylesheet())

        # Set dashboard as active by default
        self.set_active_tab("dashboard")

    def create_nav_button(self, tab: NavTab) -> QPushButton:
        """Create a navigation button"""
        btn = QPushButton(f"{tab.icon} {tab.get_label(self.language)}")
        btn.setMinimumHeight(40)
        btn.setMinimumWidth(120)
        btn.setCursor(Qt.CursorShape.PointingHandCursor)
        btn.setFont(QFont("Segoe UI", 10))
        return btn

    def on_tab_click(self, tab_id: str):
        """Handle tab click"""
        self.set_active_tab(tab_id)
        self.tab_changed.emit(tab_id)

    def set_active_tab(self, tab_id: str):
        """Set active tab"""
        # Reset all buttons
        for bid, btn in self.buttons.items():
            if bid == tab_id:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: #f39c12;
                        color: white;
                        border: 2px solid #e67e22;
                        border-radius: 5px;
                        font-weight: bold;
                    }
                    QPushButton:hover {
                        background-color: #e67e22;
                    }
                """)
            else:
                btn.setStyleSheet("""
                    QPushButton {
                        background-color: #34495e;
                        color: #ecf0f1;
                        border: 1px solid #2c3e50;
                        border-radius: 5px;
                    }
                    QPushButton:hover {
                        background-color: #455a64;
                    }
                """)

        self.current_tab = tab_id

    def _get_nav_stylesheet(self) -> str:
        """Get navigation bar stylesheet"""
        return """
            NavigationBar {
                background-color: #2c3e50;
                border-bottom: 2px solid #1a1a1a;
                padding: 5px;
            }
        """


class BreadcrumbNavigation(QWidget):
    """Breadcrumb navigation for clarity"""

    def __init__(self, language: str = "fr"):
        super().__init__()
        self.language = language
        self.path = ["Audit Logger"]
        self.setup_ui()

    def setup_ui(self):
        """Setup breadcrumb"""
        layout = QHBoxLayout()
        layout.setContentsMargins(10, 5, 10, 5)

        self.label = QLabel()
        self.label.setFont(QFont("Segoe UI", 9))
        self.label.setStyleSheet("color: #7f8c8d; padding: 0px 5px;")

        layout.addWidget(self.label)
        layout.addStretch()

        self.setLayout(layout)
        self.update_path()

    def set_path(self, path: list):
        """Set breadcrumb path"""
        self.path = ["Audit Logger"] + path
        self.update_path()

    def update_path(self):
        """Update breadcrumb display"""
        breadcrumb_text = " > ".join(self.path)
        self.label.setText(f"🏠 {breadcrumb_text}")


class TabIndicator(QWidget):
    """Clear indicator of current section"""

    def __init__(self, icon: str, title: str, subtitle: str = ""):
        super().__init__()
        self.setup_ui(icon, title, subtitle)

    def setup_ui(self, icon: str, title: str, subtitle: str):
        """Setup indicator"""
        layout = QHBoxLayout()
        layout.setContentsMargins(15, 10, 15, 10)
        layout.setSpacing(15)

        # Icon
        icon_label = QLabel(icon)
        icon_label.setFont(QFont("Arial", 28))
        layout.addWidget(icon_label)

        # Text
        text_layout = QVBoxLayout()

        title_label = QLabel(title)
        title_label.setFont(QFont("Segoe UI", 16, QFont.Weight.Bold))
        title_label.setStyleSheet("color: #ecf0f1;")
        text_layout.addWidget(title_label)

        if subtitle:
            subtitle_label = QLabel(subtitle)
            subtitle_label.setFont(QFont("Segoe UI", 10))
            subtitle_label.setStyleSheet("color: #95a5a6;")
            text_layout.addWidget(subtitle_label)

        layout.addLayout(text_layout)
        layout.addStretch()

        self.setLayout(layout)
        self.setStyleSheet("""
            TabIndicator {
                background-color: #1a1a2e;
                border-bottom: 2px solid #f39c12;
            }
        """)


class ResultsPanel(QWidget):
    """Clear, understandable results display"""

    def __init__(self, language: str = "fr"):
        super().__init__()
        self.language = language
        self.setup_ui()

    def setup_ui(self):
        """Setup results panel"""
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)

        # Empty state
        self.empty_label = QLabel("📭 Aucun résultat disponible")
        self.empty_label.setFont(QFont("Segoe UI", 12))
        self.empty_label.setStyleSheet("color: #95a5a6; padding: 40px;")
        self.empty_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        layout.addWidget(self.empty_label)
        self.setLayout(layout)

    def show_result(self, result: dict):
        """Display a result in clear format"""
        # Clear previous content
        while self.layout().count():
            self.layout().takeAt(0).widget().deleteLater()

        layout = QVBoxLayout()
        layout.setContentsMargins(15, 15, 15, 15)
        layout.setSpacing(10)

        # Result type indicator
        result_type = result.get("type", "unknown")
        icon_map = {
            "success": "✅",
            "warning": "⚠️",
            "error": "❌",
            "info": "ℹ️",
        }
        icon = icon_map.get(result.get("status", "info"), "ℹ️")

        # Title
        title_label = QLabel(f"{icon} {result.get('title', 'Résultat')}")
        title_label.setFont(QFont("Segoe UI", 14, QFont.Weight.Bold))
        title_label.setStyleSheet(self._get_status_color(result.get("status")))
        layout.addWidget(title_label)

        # Message
        if "message" in result:
            msg_label = QLabel(result["message"])
            msg_label.setFont(QFont("Segoe UI", 11))
            msg_label.setWordWrap(True)
            msg_label.setStyleSheet("color: #ecf0f1;")
            layout.addWidget(msg_label)

        # Details
        if "details" in result:
            details_label = QLabel("📋 Détails:")
            details_label.setFont(QFont("Segoe UI", 10, QFont.Weight.Bold))
            details_label.setStyleSheet("color: #f39c12; margin-top: 10px;")
            layout.addWidget(details_label)

            for key, value in result["details"].items():
                detail_line = QLabel(f"  • {key}: {value}")
                detail_line.setFont(QFont("Courier", 10))
                detail_line.setStyleSheet("color: #bdc3c7;")
                layout.addWidget(detail_line)

        # Actions
        if "actions" in result:
            actions_layout = QHBoxLayout()
            actions_layout.setSpacing(5)

            for action_name, action_callback in result["actions"].items():
                btn = QPushButton(f"📌 {action_name}")
                btn.setMinimumHeight(35)
                btn.clicked.connect(action_callback)
                btn.setFont(QFont("Segoe UI", 10))
                actions_layout.addWidget(btn)

            layout.addLayout(actions_layout)

        layout.addStretch()

        container = QWidget()
        container.setLayout(layout)
        container.setStyleSheet("""
            background-color: #1a1a2e;
            border: 1px solid #34495e;
            border-radius: 5px;
        """)

        self.layout().addWidget(container)

    def _get_status_color(self, status: str) -> str:
        """Get color for status"""
        colors = {
            "success": "color: #2ecc71;",
            "warning": "color: #f39c12;",
            "error": "color: #e74c3c;",
            "info": "color: #3498db;",
        }
        return colors.get(status, "color: #ecf0f1;")


class SimpleStatusDisplay(QWidget):
    """Simple, clear status display"""

    def __init__(self, language: str = "fr"):
        super().__init__()
        self.language = language
        self.setup_ui()

    def setup_ui(self):
        """Setup status display"""
        layout = QHBoxLayout()
        layout.setContentsMargins(10, 5, 10, 5)
        layout.setSpacing(15)

        # Status indicator
        self.status_label = QLabel("🟢 Prêt")
        self.status_label.setFont(QFont("Segoe UI", 11, QFont.Weight.Bold))
        layout.addWidget(self.status_label)

        # Info
        self.info_label = QLabel("")
        self.info_label.setFont(QFont("Segoe UI", 10))
        self.info_label.setStyleSheet("color: #95a5a6;")
        layout.addWidget(self.info_label)

        layout.addStretch()

        self.setLayout(layout)
        self.setStyleSheet("""
            SimpleStatusDisplay {
                background-color: #2c3e50;
                border: 1px solid #34495e;
                border-radius: 3px;
            }
        """)

    def set_status(self, status: str, message: str = ""):
        """Set status"""
        status_map = {
            "ready": ("🟢", "Prêt", "#2ecc71"),
            "processing": ("🟡", "Traitement en cours", "#f39c12"),
            "error": ("🔴", "Erreur", "#e74c3c"),
            "success": ("✅", "Succès", "#2ecc71"),
            "warning": ("⚠️", "Attention", "#f39c12"),
        }

        icon, label, color = status_map.get(status, ("❓", status, "#95a5a6"))

        self.status_label.setText(f"{icon} {label}")
        self.status_label.setStyleSheet(f"color: {color}; font-weight: bold;")

        if message:
            self.info_label.setText(message)
        else:
            self.info_label.setText("")
