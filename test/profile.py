import os
import glob
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from datetime import datetime

def timing_deviation():
    log_path = "logs/timing.csv"
    if not os.path.exists(log_path):
        print("Timing data not found!")
        return

    df = pd.read_csv(log_path, names=["scheduled", "start", "end"])
    df["scheduled_dt"] = df["scheduled"].apply(lambda x: datetime.fromtimestamp(x))
    df["start_diff"]   = (df["start"] - df["scheduled"]) * 1000
    df["end_diff"]     = (df["end"]   - df["scheduled"]) * 1000

    # thresholds (ms)
    reconnect_threshold = 50
    outlier_threshold   = 50

    # masks
    reconnect_mask = (abs(df["start_diff"]) > reconnect_threshold) | (abs(df["end_diff"]) > reconnect_threshold)
    outlier_mask   = (abs(df["start_diff"]) > outlier_threshold)   | (abs(df["end_diff"]) > outlier_threshold)

    # separate normal and reconnection data
    normal_df    = df[~outlier_mask]
    reconnect_df = df[reconnect_mask]

    # compute averages
    # Average Delay (excluding outliers)
    norm_vals = np.concatenate([normal_df["start_diff"].values, normal_df["end_diff"].values])
    avg_normal = np.mean(norm_vals) if len(norm_vals) > 0 else np.nan
    # Average Reconnection Delay (only reconnect events)
    reconn_vals = np.concatenate([reconnect_df["start_diff"].values, reconnect_df["end_diff"].values])
    avg_reconn = np.mean(reconn_vals) if len(reconn_vals) > 0 else np.nan

    fig = plt.figure(figsize=(14, 8))
    ax_main = fig.add_subplot(111)

    # Main Plot: only normal data
    ax_main.plot(normal_df["scheduled_dt"], normal_df["start_diff"],
                 label="Start Delay (normal)", linestyle="--", marker="o", markersize=4, alpha=0.7)
    ax_main.plot(normal_df["scheduled_dt"], normal_df["end_diff"],
                 label="End Delay (normal)",   linestyle="-",  marker="s", markersize=4, alpha=0.7)

    typical_max = 75
    ax_main.set_ylim(-typical_max, typical_max)

    # Inset: full range including outliers
    ax_inset = fig.add_axes([0.65, 0.15, 0.25, 0.25])
    ax_inset.plot(df["scheduled_dt"], df["start_diff"], ".-", markersize=2, alpha=0.5)
    ax_inset.plot(df["scheduled_dt"], df["end_diff"],   ".-", markersize=2, alpha=0.5)
    ax_inset.set_title("Full Range View", fontsize=9)
    ax_inset.tick_params(labelsize=7)
    # annotate reconnection events on inset
    if not reconnect_df.empty:
        ax_inset.scatter(reconnect_df["scheduled_dt"], reconnect_df["start_diff"], s=20, c='blue', marker='*', label='Reconnect Start')
        ax_inset.scatter(reconnect_df["scheduled_dt"], reconnect_df["end_diff"],   s=20, c='red',  marker='*', label='Reconnect End')

    # legend for averages
    avg_text = (
        f"Avg delay (normal): {avg_normal:.1f} ms\n"
        f"Avg reconnection delay: {avg_reconn:.1f} ms"
    )
    ax_main.text(0.02, 0.02, avg_text, transform=ax_main.transAxes,
                 verticalalignment='bottom', bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))

    # titles and labels
    ax_main.set_title("Task Execution Timing Deviation (Typical Range)")
    ax_main.set_xlabel("Scheduled Time")
    ax_main.set_ylabel("Deviation (ms)")

    ax_main.grid(True, which="both", linestyle="--", alpha=0.5)
    ax_main.axhline(0, color="black", linewidth=1)
    ax_main.legend(loc="upper left")

    plt.tight_layout()
    plt.savefig("test/assets/timing.png")
    plt.close()

def correlation_heatmap():
    symbols = [
        "BTC-USDT", "ADA-USDT", "ETH-USDT", "DOGE-USDT",
        "XRP-USDT", "SOL-USDT", "LTC-USDT", "BNB-USDT"
    ]
    corr_matrix = pd.DataFrame(0.0, index=symbols, columns=symbols, dtype=float)
    counts = pd.DataFrame(0, index=symbols, columns=symbols, dtype=int)

    for i, symbol in enumerate(symbols):
        log_path = f"logs/corr/{symbol}.log"
        if not os.path.exists(log_path):
            continue
            
        df = pd.read_csv(log_path, header=None)
        for _, row in df.iterrows():
            correlations = row[3:].values
            for j in range(len(symbols)):
                if j >= len(correlations):
                    break
                other_symbol = symbols[j]
                corr = correlations[j]
                corr_matrix.loc[symbol, other_symbol] += corr
                counts.loc[symbol, other_symbol] += 1

    for symbol in symbols:
        for other_symbol in symbols:
            if counts.loc[symbol, other_symbol] > 0:
                corr_matrix.loc[symbol, other_symbol] /= counts.loc[symbol, other_symbol]
            else:
                corr_matrix.loc[symbol, other_symbol] = 0.0

    # average correlation (excluding diagonal)
    avg_corr = np.nanmean(corr_matrix.values.diagonal())
    avg_corr = np.nanmean(corr_matrix.values)
    avg_corr = f"{avg_corr:.2f}"

    # mask diagonal
    np.fill_diagonal(corr_matrix.values, np.nan)

    plt.figure(figsize=(12, 10))
    mask = np.eye(len(corr_matrix), dtype=bool)
    ax = sns.heatmap(
        corr_matrix.astype(float), 
        annot=True, 
        fmt=".2f", 
        cmap="coolwarm", 
        center=0, 
        vmin=-1, 
        vmax=1,
        mask=mask,
        cbar_kws={'label': 'Correlation Strength'}
    )
    
    # N/A to diagonal
    for i in range(len(corr_matrix)):
        ax.text(
            i + 0.5, i + 0.5, "N/A",
            ha="center", va="center",
            color="black", fontweight="bold",
            bbox=dict(facecolor="white", edgecolor="none", pad=1)
        )
    
    # average correlation legend
    ax.text(
        0.84, 0.02,  # bottom-right corner (x=84%, y=2%)
        f"Avg Correlation: {avg_corr}",
        transform=ax.transAxes,
        fontsize=12,
        bbox=dict(facecolor="white", edgecolor="gray", boxstyle="round,pad=0.3")
    )

    plt.title("Cryptocurrency Correlation Heatmap")
    plt.tight_layout()
    plt.savefig("test/assets/correlation.png")
    plt.close()

def analyze_cpu_idle():
    df = pd.read_csv(
        "logs/cpu.log",
        sep=r'\s+',
        header=None,
        names=["timestamp", "idle_pct"]
    )
    
    df["datetime"] = pd.to_datetime(df["timestamp"], unit='s')
    
    df.set_index("datetime", inplace=True)
    idle_pct = df["idle_pct"].resample('1min').mean()
    
    plt.figure(figsize=(12, 6))
    idle_pct.plot(label="CPU Idle %")
    plt.title("CPU Utilization Over Time")
    plt.xlabel("Time")
    plt.ylabel("Idle Percentage")
    plt.grid(True)
    plt.savefig("test/assets/cpu_idle.png")
    plt.close()

if __name__ == "__main__":
    timing_deviation()
    correlation_heatmap()
    analyze_cpu_idle()
    print("Generated historical plots in test/assets/ directory")