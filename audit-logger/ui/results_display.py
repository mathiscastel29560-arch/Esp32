"""
Results Display Components - Clear & Understandable

Composants d'Affichage des Résultats - Clairs et Compréhensibles
"""

from PyQt6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel,
                             QTableWidget, QTableWidgetItem, QProgressBar,
                             QScrollArea, QFrame, QGroupBox)
from PyQt6.QtCore import Qt, QSize
from PyQt6.QtGui import QFont, QColor, QBrush, QIcon


class ResultCard(QFrame):
    """Card for displaying a single result"""

    def __init__(self, icon: str, title: str, status: str = "info"):
        super().__init__()
        self.status = status
        self.setup_ui(icon, title)

    def setup_ui(self, icon: str, title: str):
        """Setup card"""
        self.setFrameStyle(QFrame.Shape.StyledPanel | QFrame.Shadow.Raised)

        layout = QVBoxLayout()
        layout.setContentsMargins(15, 10, 15, 10)
        layout.setSpacing(8)

        # Header
        header_layout = QHBoxLayout()

        icon_label = QLabel(icon)
        icon_label.setFont(QFont("Arial", 24))
        header_layout.addWidget(icon_label)

        title_label = QLabel(title)
        title_label.setFont(QFont("Segoe UI", 12, QFont.Weight.Bold))
        title_label.setStyleSheet(f"color: {self._get_status_color()};")
        header_layout.addWidget(title_label)

        header_layout.addStretch()
        layout.addLayout(header_layout)

        # Content area
        self.content_layout = QVBoxLayout()
        self.content_layout.setSpacing(5)
        layout.addLayout(self.content_layout)

        self.setLayout(layout)
        self.setStyleSheet(self._get_stylesheet())

    def add_detail(self, label: str, value: str):
        """Add detail row"""
        row_layout = QHBoxLayout()
        row_layout.setSpacing(10)

        label_widget = QLabel(f"• {label}:")
        label_widget.setFont(QFont("Segoe UI", 10, QFont.Weight.Bold))
        label_widget.setStyleSheet("color: #f39c12;")
        label_widget.setMinimumWidth(150)
        row_layout.addWidget(label_widget)

        value_widget = QLabel(str(value))
        value_widget.setFont(QFont("Courier", 10))
        value_widget.setStyleSheet("color: #ecf0f1;")
        value_widget.setWordWrap(True)
        row_layout.addWidget(value_widget)

        self.content_layout.addLayout(row_layout)

    def add_separator(self):
        """Add separator"""
        sep = QFrame()
        sep.setFrameShape(QFrame.Shape.HLine)
        sep.setStyleSheet("color: #34495e;")
        self.content_layout.addWidget(sep)

    def _get_status_color(self) -> str:
        """Get color for status"""
        colors = {
            "success": "#2ecc71",
            "warning": "#f39c12",
            "error": "#e74c3c",
            "info": "#3498db",
        }
        return colors.get(self.status, "#ecf0f1")

    def _get_stylesheet(self) -> str:
        """Get card stylesheet"""
        color = self._get_status_color()
        return f"""
            ResultCard {{
                background-color: #1a1a2e;
                border: 2px solid {color};
                border-radius: 8px;
            }}
        """


class AttackResultDisplay(QWidget):
    """Display results from attack execution"""

    def __init__(self):
        super().__init__()
        self.setup_ui()

    def setup_ui(self):
        """Setup display"""
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(10)

        self.scroll = QScrollArea()
        self.scroll.setWidgetResizable(True)
        self.scroll.setStyleSheet("""
            QScrollArea {
                background-color: #0f3460;
                border: none;
            }
            QScrollBar:vertical {
                width: 10px;
                background-color: #1a1a2e;
            }
            QScrollBar::handle:vertical {
                background-color: #34495e;
                border-radius: 5px;
            }
        """)

        self.container = QWidget()
        self.container_layout = QVBoxLayout()
        self.container_layout.setSpacing(10)
        self.container.setLayout(self.container_layout)

        self.scroll.setWidget(self.container)
        layout.addWidget(self.scroll)

        self.setLayout(layout)

    def show_attack_result(self, attack_data: dict):
        """Display attack result"""
        # Clear previous
        while self.container_layout.count():
            self.container_layout.takeAt(0).widget().deleteLater()

        # Determine status icon and color
        success = attack_data.get("success", False)
        status_icon = "✅" if success else "❌"
        status_color = "success" if success else "error"

        # Main result card
        result_card = ResultCard(
            status_icon,
            attack_data.get("attack_type", "Attaque"),
            status_color
        )

        # Add details
        result_card.add_detail("Statut", "Réussi ✓" if success else "Échoué ✗")
        result_card.add_detail("ID Attaque", attack_data.get("attack_id", "N/A"))

        if "target" in attack_data:
            result_card.add_detail("Cible", attack_data["target"])

        if "duration" in attack_data:
            result_card.add_detail("Durée", f"{attack_data['duration']}s")

        result_card.add_separator()

        if "message" in attack_data:
            result_card.add_detail("Résultat", attack_data["message"])

        if "details" in attack_data:
            details = attack_data["details"]
            for key, value in details.items():
                result_card.add_detail(key, str(value))

        self.container_layout.addWidget(result_card)
        self.container_layout.addStretch()

    def show_workflow_result(self, workflow_data: dict):
        """Display workflow result"""
        # Clear previous
        while self.container_layout.count():
            self.container_layout.takeAt(0).widget().deleteLater()

        # Main workflow card
        workflow_card = ResultCard(
            "⚙️",
            workflow_data.get("name", "Flux de Travail"),
            "info"
        )

        # Progress
        progress = workflow_data.get("progress", {})
        workflow_card.add_detail("Progression", f"{progress.get('completed', 0)}/{progress.get('total_steps', 0)} étapes")
        workflow_card.add_detail("État", workflow_data.get("status", "En cours"))

        # Duration
        if "duration" in workflow_data:
            workflow_card.add_detail("Durée Totale", f"{workflow_data['duration']}s")

        workflow_card.add_separator()

        # Steps
        if "steps" in workflow_data:
            for step in workflow_data["steps"]:
                step_status = step.get("status", "pending")
                status_icon = {
                    "completed": "✅",
                    "running": "🔄",
                    "failed": "❌",
                    "pending": "⏳",
                }.get(step_status, "❓")

                workflow_card.add_detail(
                    f"{status_icon} {step.get('step_id', 'Step')}",
                    step.get("type", "")
                )

        self.container_layout.addWidget(workflow_card)
        self.container_layout.addStretch()


class ProgressDisplay(QWidget):
    """Display progress with clear indicators"""

    def __init__(self, title: str = "Progression"):
        super().__init__()
        self.title = title
        self.setup_ui()

    def setup_ui(self):
        """Setup progress display"""
        layout = QVBoxLayout()
        layout.setContentsMargins(15, 15, 15, 15)
        layout.setSpacing(10)

        # Title
        title_label = QLabel(self.title)
        title_label.setFont(QFont("Segoe UI", 11, QFont.Weight.Bold))
        title_label.setStyleSheet("color: #f39c12;")
        layout.addWidget(title_label)

        # Progress bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setMinimum(0)
        self.progress_bar.setMaximum(100)
        self.progress_bar.setValue(0)
        self.progress_bar.setStyleSheet("""
            QProgressBar {
                border: 1px solid #34495e;
                border-radius: 5px;
                background-color: #1a1a2e;
                text-align: center;
                color: #ecf0f1;
                height: 25px;
            }
            QProgressBar::chunk {
                background-color: #f39c12;
                border-radius: 4px;
            }
        """)
        layout.addWidget(self.progress_bar)

        # Status text
        self.status_label = QLabel("En attente...")
        self.status_label.setFont(QFont("Segoe UI", 10))
        self.status_label.setStyleSheet("color: #95a5a6;")
        layout.addWidget(self.status_label)

        self.setLayout(layout)

    def set_progress(self, percent: int, status: str = ""):
        """Set progress"""
        self.progress_bar.setValue(percent)
        if status:
            self.status_label.setText(status)


class ComparisonTable(QTableWidget):
    """Table for displaying before/after comparisons"""

    def __init__(self, headers: list):
        super().__init__()
        self.setColumnCount(len(headers))
        self.setHorizontalHeaderLabels(headers)
        self.setup_style()

    def setup_style(self):
        """Setup table style"""
        self.setStyleSheet("""
            QTableWidget {
                background-color: #1a1a2e;
                gridline-color: #34495e;
                border: 1px solid #34495e;
            }
            QTableWidget::item {
                padding: 5px;
                border-bottom: 1px solid #34495e;
            }
            QTableWidget::item:selected {
                background-color: #f39c12;
                color: #1a1a2e;
            }
            QHeaderView::section {
                background-color: #2c3e50;
                color: #ecf0f1;
                padding: 5px;
                border: 1px solid #34495e;
                font-weight: bold;
            }
        """)

        # Font
        self.setFont(QFont("Segoe UI", 10))
        self.horizontalHeader().setFont(QFont("Segoe UI", 10, QFont.Weight.Bold))

    def add_row(self, data: list, status: str = "info"):
        """Add row with status coloring"""
        row = self.rowCount()
        self.insertRow(row)

        color_map = {
            "success": QColor("#2ecc71"),
            "warning": QColor("#f39c12"),
            "error": QColor("#e74c3c"),
            "info": QColor("#3498db"),
        }

        bg_color = color_map.get(status, QColor("#34495e"))

        for col, value in enumerate(data):
            item = QTableWidgetItem(str(value))
            item.setForeground(QBrush(QColor("#ecf0f1")))
            # Light background for row
            if status != "info":
                item.setBackground(QBrush(QColor(bg_color.getRgb()[0], bg_color.getRgb()[1], bg_color.getRgb()[2], 30)))
            self.setItem(row, col, item)


class StatisticsDisplay(QWidget):
    """Display statistics in clear format"""

    def __init__(self):
        super().__init__()
        self.setup_ui()

    def setup_ui(self):
        """Setup statistics display"""
        layout = QVBoxLayout()
        layout.setContentsMargins(15, 15, 15, 15)
        layout.setSpacing(15)

        # Title
        title = QLabel("📊 Statistiques")
        title.setFont(QFont("Segoe UI", 14, QFont.Weight.Bold))
        title.setStyleSheet("color: #f39c12;")
        layout.addWidget(title)

        # Stats grid
        self.stats_layout = QVBoxLayout()
        self.stats_layout.setSpacing(10)
        layout.addLayout(self.stats_layout)

        layout.addStretch()
        self.setLayout(layout)

    def add_stat(self, label: str, value: str, icon: str = "📌"):
        """Add statistic"""
        stat_layout = QHBoxLayout()
        stat_layout.setSpacing(10)

        icon_label = QLabel(icon)
        icon_label.setFont(QFont("Arial", 18))
        stat_layout.addWidget(icon_label)

        label_widget = QLabel(label)
        label_widget.setFont(QFont("Segoe UI", 11))
        label_widget.setStyleSheet("color: #95a5a6;")
        stat_layout.addWidget(label_widget)

        value_widget = QLabel(str(value))
        value_widget.setFont(QFont("Segoe UI", 14, QFont.Weight.Bold))
        value_widget.setStyleSheet("color: #2ecc71;")
        stat_layout.addWidget(value_widget)

        stat_layout.addStretch()

        self.stats_layout.addLayout(stat_layout)

    def clear_stats(self):
        """Clear all statistics"""
        while self.stats_layout.count():
            self.stats_layout.takeAt(0).widget().deleteLater()
