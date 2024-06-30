import requests
import concurrent.futures
import time

# Replace with your proxy server's URL
proxy_server_url = 'http://localhost:8080/'

# Number of concurrent requests to send
num_requests = 10

# URL to request through the proxy
url_to_request = proxy_server_url + 'https://www.cs.princeton.edu'

# Function to send requests to the proxy server sequentially
def send_requests_sequentially(url):
    response_times = []
    for _ in range(num_requests):
        start_time = time.time()
        try:
            response = requests.get(url)
            print(f"Request to {url} - Status code: {response.status_code} - Response time: {time.time() - start_time:.4f} seconds")
        except requests.exceptions.RequestException as e:
            print(f"Request to {url} - Exception: {e}")
        response_times.append(time.time() - start_time)
        time.sleep(1)  # Adjust sleep time as needed
    return response_times

# Use ThreadPoolExecutor to send concurrent requests
response_times = []
with concurrent.futures.ThreadPoolExecutor(max_workers=1) as executor:  # max_workers=1 for sequential execution
    results = executor.map(send_requests_sequentially, [url_to_request])
    for result in results:
        response_times.extend(result)

# Calculate performance metrics
total_time = sum(response_times)
average_response_time = total_time / (num_requests * len(response_times)) if response_times else 0
throughput = len(response_times) / total_time if total_time > 0 else 0

# Print performance metrics
print(f"\nTotal time taken: {total_time:.2f} seconds")
print(f"Average response time per request: {average_response_time:.4f} seconds")
print(f"Throughput: {throughput:.2f} requests per second")
