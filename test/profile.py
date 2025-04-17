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
    df["start_diff"] = (df["start"] - df["scheduled"])*1000
    df["end_diff"] = (df["end"] - df["scheduled"])*1000
    
    plt.figure(figsize=(14, 8))
    
    plt.plot(df["scheduled_dt"], df["start_diff"], 
             label="Start Delay", 
             linestyle="--", 
             marker="o",
             markersize=4,
             alpha=0.7,
             color="navy")
    
    plt.plot(df["scheduled_dt"], df["end_diff"], 
             label="End Delay", 
             linestyle="-",
             marker="s",
             markersize=4,
             alpha=0.7,
             color="crimson")
    
    max_dev = max(abs(df["start_diff"].max()), 
                 abs(df["end_diff"].max())) * 1.2
    plt.ylim(-max_dev, max_dev)
    
    plt.title("Task Execution Timing Deviation (All Time)")
    plt.xlabel("Scheduled Time")
    plt.ylabel("Deviation (ms)")
    
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend(loc="upper left")
    
    plt.axhline(0, color="black", linewidth=1)
    
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
    avg_corr = np.nanmean(corr_matrix.values)  # More accurate: exclude diagonal
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
        0.84, 0.02,  # Bottom-right corner (x=84%, y=2%)
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
        sep=r'\s+',          # Split on whitespace
        header=None,         # No header
        names=["timestamp", "idle_pct"]
    )
    
    # Convert timestamp to datetime
    df["datetime"] = pd.to_datetime(df["timestamp"], unit='s')
    
    # Resample to minute averages
    df.set_index("datetime", inplace=True)
    idle_pct = df["idle_pct"].resample('1min').mean()
    
    # Plot
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