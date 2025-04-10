from pathlib import Path
import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt

SCRIPT_DIR = Path(os.path.dirname(os.path.realpath(__file__)))
PROJECT_ROOT = SCRIPT_DIR.parent
RESULTS_DIR = SCRIPT_DIR / "results"
RESULTS_FILE = RESULTS_DIR / "results.txt"

def compile():
    subprocess.run(["make"], cwd=PROJECT_ROOT, check=True)

def experiment(producers, consumers, runs=10):

    wait_times = []
    binary_path = PROJECT_ROOT / "bin" / "pc"
    
    for _ in range(runs):
        proc = subprocess.Popen(
            [binary_path, str(producers), str(consumers)],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL
        )
        output = proc.communicate()[0].decode()
        times = [int(line) for line in output.splitlines() if line.strip()]
        wait_times.extend(times)
    return np.mean(wait_times) if wait_times else 0

def test(max_producers=5, max_consumers=6):

    compile()
    
    with open(RESULTS_FILE, "w") as f:

        for producers in range(1, max_producers+1):
            header = f"\n{'=' * 50}\n||{' ' * 17}PRODUCERS = {producers}{' ' * 17}||\n{'=' * 50}\n"
            print(header.strip())
            f.write(header)
            
            results = {}
            for consumers in range(1, max_consumers+1):
                avg_time = experiment(producers, consumers)
                results[consumers] = avg_time
                line = f"Consumers: {consumers}, Producers: {producers}, Avg Wait: {avg_time:.2f}μs\n"
                print(line.strip())
                f.write(line)
            
            plt.figure(figsize=(10, 6))
            plt.plot(results.keys(), results.values(), marker='o', linestyle='-')
            plt.xlabel('Number of Consumers')
            plt.ylabel('Average Wait Time (μs)')
            plt.title(f'Producer-Consumer Performance ({producers} Producers)')
            plt.grid(True)
            plt.xticks(range(1, max_consumers + 1))
            
            plot_file = RESULTS_DIR / f"p{producers}.png"
            plt.savefig(plot_file)
            plt.close()
            print(f"Plot saved to {plot_file}\n")
            f.write(f"Plot saved to {plot_file}\n")
            
            optimal = min(results, key=results.get)
            optimal_line = f"Optimal number of consumers for {producers} producer(s): {optimal}\n"
            print(optimal_line.strip())
            f.write(optimal_line)
            
if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Producer-Consumer Profiler')
    parser.add_argument('-c', '--max-consumers', type=int, default=6,
                        help='Maximum number of consumers to test for each producer count')
    args = parser.parse_args()
    
    test(max_consumers=args.max_consumers)
    print(f"\nAll results have been saved to {RESULTS_FILE}")