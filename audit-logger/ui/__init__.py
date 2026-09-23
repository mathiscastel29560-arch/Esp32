"""
UI Components for Audit Logger

Composants UI pour Audit Logger
"""

from ui.translations_fr import translate as tr
from ui.navigation import (
    NavigationBar,
    BreadcrumbNavigation,
    TabIndicator,
    ResultsPanel,
    SimpleStatusDisplay,
)
from ui.results_display import (
    ResultCard,
    AttackResultDisplay,
    ProgressDisplay,
    ComparisonTable,
    StatisticsDisplay,
)

__all__ = [
    'tr',
    'NavigationBar',
    'BreadcrumbNavigation',
    'TabIndicator',
    'ResultsPanel',
    'SimpleStatusDisplay',
    'ResultCard',
    'AttackResultDisplay',
    'ProgressDisplay',
    'ComparisonTable',
    'StatisticsDisplay',
]
