import subprocess
import time
import statistics
import matplotlib.pyplot as plt
import requests
import random
from concurrent.futures import ThreadPoolExecutor, as_completed

# List of common websites
WEBSITES = [
    "http://www.google.com",
    "http://www.youtube.com",
    "http://www.facebook.com",
    "http://www.amazon.com",
    "http://www.wikipedia.org",
    "http://www.twitter.com",
    "http://www.instagram.com",
    "http://www.linkedin.com",
    "http://www.reddit.com",
    "http://www.github.com"
]

def start_server(server_path, port):
    try:
        process = subprocess.Popen([server_path, str(port)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)  # Give the server time to start
        return_code = process.poll()
        if return_code is not None:
            stdout, stderr = process.communicate()
            print(f"Server failed to start. Return code: {return_code}")
            print(f"Stdout: {stdout.decode()}")
            print(f"Stderr: {stderr.decode()}")
            return None
        return process
    except Exception as e:
        print(f"Error starting server: {e}")
        return None

def stop_server(process):
    if process:
        try:
            process.terminate()
            process.wait(timeout=5)  # Wait for up to 5 seconds
        except subprocess.TimeoutExpired:
            process.kill()  # Force kill if it doesn't terminate
        finally:
            process.wait()  # Ensure the process is fully terminated
    
    # Wait a bit to ensure the port is freed
    time.sleep(2)

def make_request(proxy_url, max_retries=3):
    for attempt in range(max_retries):
        try:
            time.sleep(0.1 * (2 ** attempt))  # Exponential backoff
            start_time = time.time()
            target_url = random.choice(WEBSITES)
            url = f"{proxy_url}/{target_url}"
            response = requests.get(url, timeout=30)
            return time.time() - start_time, target_url
        except Exception as e:
            if attempt == max_retries - 1:
                print(f"Request failed after {max_retries} attempts: {e}")
                return None, None

def benchmark(server_name, proxy_url, num_requests, concurrency):
    print(f"Benchmarking {server_name}...")
    results = []
    
    # Warm-up period
    print("Warming up server...")
    for _ in range(10):
        make_request(proxy_url)
    time.sleep(2)
    
    # Gradual ramp-up
    print("Starting benchmark with gradual ramp-up...")
    with ThreadPoolExecutor(max_workers=concurrency) as executor:
        for i in range(1, concurrency + 1):
            futures = [executor.submit(make_request, proxy_url) for _ in range(num_requests // concurrency)]
            for future in as_completed(futures):
                result = future.result()
                if result[0] is not None:
                    results.append(result)
            print(f"Completed batch {i}/{concurrency}")
            time.sleep(1)  # Short pause between batches
    
    if not results:
        print(f"Error: No successful requests for {server_name}")
        return None

    # ... rest of the function
    
    if not results:
        print(f"Error: No successful requests for {server_name}")
        return None

    latencies = [r[0] for r in results]
    websites = [r[1] for r in results]

    return {
        'avg_latency': statistics.mean(latencies),
        'max_latency': max(latencies),
        'min_latency': min(latencies),
        'requests_per_second': len(latencies) / sum(latencies),
        'website_distribution': {site: websites.count(site) for site in set(websites)}
    }

def run_benchmarks(threadpool_server, semaphore_server, threadpool_port, semaphore_port):
    num_requests = 380 # Increased for better distribution
    concurrency = 50
    proxy_url = "http://localhost:{}"

    results = {}

    for server_name, server_path, port in [
        ("ThreadPool", threadpool_server, threadpool_port),
        ("Semaphore", semaphore_server, semaphore_port)
    ]:
        process = start_server(server_path, port)
        if process:
            results[server_name] = benchmark(server_name, proxy_url.format(port), num_requests, concurrency)
            stop_server(process)
        else:
            print(f"Skipping benchmark for {server_name} due to server start failure")
            results[server_name] = None

    return results

def plot_results(results):
    if all(result is None for result in results.values()):
        print("No data to plot. All benchmarks failed.")
        return

    metrics = ['avg_latency', 'max_latency', 'min_latency', 'requests_per_second']
    
    fig, axs = plt.subplots(2, 2, figsize=(12, 10))
    fig.suptitle('Proxy Server Performance Comparison')
    
    for i, metric in enumerate(metrics):
        ax = axs[i // 2, i % 2]
        values = []
        labels = []
        for server, data in results.items():
            if data and metric in data:
                values.append(data[metric])
                labels.append(server)
        if values:
            ax.bar(labels, values)
            ax.set_title(metric.replace('_', ' ').title())
            ax.set_ylabel('Seconds' if 'latency' in metric else 'Requests/second')

    plt.tight_layout()
    plt.savefig('benchmark_results.png')
    print("Results plot saved as benchmark_results.png")

    # Plot website distribution
    fig, axs = plt.subplots(1, 2, figsize=(15, 5))
    fig.suptitle('Website Distribution')
    
    for i, (server, data) in enumerate(results.items()):
        if data and 'website_distribution' in data:
            websites = list(data['website_distribution'].keys())
            counts = list(data['website_distribution'].values())
            axs[i].pie(counts, labels=websites, autopct='%1.1f%%')
            axs[i].set_title(f'{server} Server')

    plt.tight_layout()
    plt.savefig('website_distribution.png')
    print("Website distribution plot saved as website_distribution.png")

if __name__ == "__main__":
    threadpool_server = "./proxy2"
    semaphore_server = "./proxy3"
    threadpool_port = 20002
    semaphore_port = 20003

    results = run_benchmarks(threadpool_server, semaphore_server, threadpool_port, semaphore_port)

    print("\nResults:")
    for server, metrics in results.items():
        print(f"\n{server} Server:")
        if metrics:
            for metric, value in metrics.items():
                if metric != 'website_distribution':
                    print(f"  {metric}: {value:.4f}")
                else:
                    print("  Website Distribution:")
                    for site, count in value.items():
                        print(f"    {site}: {count}")
        else:
            print("  No results available")

    plot_results(results)