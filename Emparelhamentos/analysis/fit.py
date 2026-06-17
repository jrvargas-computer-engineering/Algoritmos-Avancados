# analysis/fit.py
import numpy as np

def fit_power_law(x_data, y_data):
    """
    Fits log(y) = k * log(n) + c via ordinary least squares.
    Returns the empirical exponent (k), the constant (c), and the R^2 correlation coefficient.
    """
    # Filter out invalid domains for log transformations
    mask = (x_data > 0) & (y_data > 0)
    x = np.array(x_data)[mask]
    y = np.array(y_data)[mask]
    
    if len(x) < 2:
        return np.nan, np.nan, np.nan

    log_x = np.log10(x)
    log_y = np.log10(y)
    
    # Linear fit: log(y) = k*log(x) + c
    k, c_log = np.polyfit(log_x, log_y, 1)
    
    # Calculate R-squared value
    y_pred = k * log_x + c_log
    ss_res = np.sum((log_y - y_pred)**2)
    ss_tot = np.sum((log_y - np.mean(log_y))**2)
    r_squared = 1 - (ss_res / ss_tot) if ss_tot > 0 else 1.0
    
    c = 10**c_log
    return k, c, r_squared