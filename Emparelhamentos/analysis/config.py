# analysis/config.py
import matplotlib.pyplot as plt

# Global publication-quality figure settings
FIG_SIZE = (8, 6)
DPI = 300
GRID_ALPHA = 0.3
FIG_EXT = "png"

# Algorithm-specific styling dictionaries
STYLES = {
    "SimpleAugmentingMatcher": {
        "color": "#D85A30",  # Coral
        "linestyle": "-",    # Solid
        "marker": "o",       # Circle
        "label": "Simple Augmenting"
    },
    "HopcroftKarpMatcher": {
        "color": "#185FA5",  # Blue
        "linestyle": "-",    # Solid
        "marker": "s",       # Square
        "label": "Hopcroft-Karp"
    },
    "HungarianMatcher+BellmanFordStrategy": {
        "color": "#854F0B",  # Amber
        "linestyle": "--",   # Dashed
        "marker": "^",       # Triangle
        "label": "Hungarian (Bellman-Ford)"
    },
    "HungarianMatcher+JohnsonDijkstraStrategy": {
        "color": "#0F6E56",  # Teal
        "linestyle": "-",    # Solid
        "marker": "D",       # Diamond
        "label": "Hungarian (Johnson-Dijkstra)"
    }
}

def apply_base_style(ax, xlabel, ylabel, title=None):
    """Applies the universal grid, label, and legend conventions."""
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    if title:
        ax.set_title(title)
    ax.grid(True, alpha=GRID_ALPHA)
    handles, labels = ax.get_legend_handles_labels()
    if handles:
        ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')