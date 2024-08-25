import subprocess
import re
import csv
import time

# List of items to extract from the logs
items_to_extract = [
    'load time', 'sample time', 'prompt eval time', 'eval time', 'total time',
    'Token Generation Timer', 'PIM Timer', 
    'GPU GEMV OPS NOT REPLAYED', 'GPU REPLAYED GEMV OPS', 'GPU GEMV OPS NOT RECORDED', 
    'GPU RECORDED GEMV OPS', 'GPU GEMV Timer', 
    'CPU GEMV OPS NOT REPLAYED', 'CPU GEMV OPS REPLAYED', 'CPU GEMV OPS NOT RECORDED', 
    'CPU GEMV OPS RECORDED', 'CPU GEMV Timer'
]

# Function to extract specific items from the log
def extract_values(output):
    values = {}
    for item in items_to_extract:
        pattern = re.compile(rf'{re.escape(item)}\s*=\s*([\d.]+)')
        match = pattern.search(output)
        values[item] = float(match.group(1)) if match else None
    return values

# Function to run a command and extract timing information
def run_application(command, mode):
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    output = result.stdout + result.stderr
    values = extract_values(output)
    return [mode] + [values[item] for item in items_to_extract]

# Function to calculate averages manually for replay mode
def calculate_averages(replay_runs_data):
    num_columns = len(replay_runs_data[0]) - 1  # Exclude the mode column
    averages = []
    
    for i in range(1, num_columns + 1):  # Start from index 1 to skip mode column
        sum_column = sum(float(run[i]) for run in replay_runs_data if run[i] is not None)
        count = sum(1 for run in replay_runs_data if run[i] is not None)
        averages.append(sum_column / count if count > 0 else None)
    
    return averages

# Function to append results to a CSV file
def append_to_csv(run_data, filename):
    with open(filename, 'a', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(run_data)

# Function to write averages to the CSV file
def write_averages_to_csv(averages, filename):
    with open(filename, 'a', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(['Replay Mode Average'] + averages)

# Function to write the header to the CSV file
def write_header_to_csv(filename):
    header = ['Mode'] + items_to_extract
    with open(filename, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(header)

# Main script execution
if __name__ == "__main__":
    record_command = input("Enter the command to run in record mode: ")
    replay_command = input("Enter the command to run in replay mode: ")
    num_replay_runs = int(input("How many times do you want to run the replay mode command? "))
    csv_filename = input("Enter the name for the CSV file (e.g., results.csv): ")
    
    # Write header to CSV based on the specified items to extract
    write_header_to_csv(csv_filename)
    
    # Run record mode command once
    record_run_data = run_application(record_command, "Record Mode")
    append_to_csv(record_run_data, csv_filename)
    print("Record mode run completed.")
    time.sleep(5)
    
    replay_runs_data = []
    
    # Run replay mode command specified number of times
    for i in range(num_replay_runs):
        replay_run_data = run_application(replay_command, "Replay Mode")
        replay_runs_data.append(replay_run_data)
        append_to_csv(replay_run_data, csv_filename)
        print(f"Replay mode run {i+1} completed.")
        time.sleep(5)
    
    # Calculate averages for replay mode only
    averages = calculate_averages(replay_runs_data)
    
    # Write averages to the CSV file
    write_averages_to_csv(averages, csv_filename)
    
    print(f"Data saved to {csv_filename}")

