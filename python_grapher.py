import os
import datetime
import time
import csv
import json
import matplotlib.pyplot as plt
import sys
import subprocess

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Load graph configuration from JSON
with open("graph_config.json", "r") as config_file:
    config = json.load(config_file)

# Load tank settings
with open("tank_settings.json", "r") as settings_file:
    settings = json.load(settings_file)

# Get selected graph type and corresponding dates
selected_graph_type = config.get("selected_graph_type", "daily")
date_config = config.get(selected_graph_type, config["daily"])

dateStart = date_config["start_date"]
dateEnd = date_config["end_date"]

dSt = datetime.datetime.strptime(dateStart, "%Y-%m-%d")
dEn = datetime.datetime.strptime(dateEnd, "%Y-%m-%d")

dSt = dSt.replace(hour=0, minute=0, second=0, microsecond=0)
dEn = dEn.replace(hour=0, minute=0, second=0, microsecond=0)

start_ts = int(time.mktime(dSt.timetuple()))
end_ts = int(time.mktime(dEn.timetuple())) + 86400  # + one day in seconds

print(f"Debug: Date range {dateStart} to {dateEnd}")
print(f"Debug: Timestamp range {start_ts} to {end_ts}")

# Get current tank - handle both string and int formats
currentTank = str(settings.get("index", {}).get("currentTank", "0"))

# Validate currentTank is not "0" (no tank selected)
if currentTank == "0" or not currentTank:
    print("Error: No tank currently selected in tank_settings.json")
    print("Please configure a tank first using the C program.")
    sys.exit(1)

# Prefer slot with matching tankId; fall back to slot{currentTank}
tank_config = settings.get(f"slot{currentTank}", {})
if not tank_config or str(tank_config.get("tankId", "")) != currentTank:
    tank_config = {}
    for i in range(1, 9):
        c = settings.get(f"slot{i}", {})
        if str(c.get("tankId", "")) == currentTank:
            tank_config = c
            break

if not tank_config:
    print(f"Error: Tank configuration for tankId {currentTank} not found in any slot")
    sys.exit(1)

tankCsv = tank_config.get("nameCsv")
if not tankCsv or tankCsv.strip().lower() == "data_tank0.csv":
    print(f"Error: Invalid CSV configured for current tank (got '{tankCsv}')")
    print("Please configure the tank in the C program so nameCsv points to a real file.")
    sys.exit(1)


def aggregate_data(tankCsv, start_ts, end_ts, graph_type):
    """Aggregate data based on graph type"""
    aggregated = {}
    all_timestamps = []
    
    try:
        with open(tankCsv, "r") as df:
            reader = csv.DictReader(df, delimiter=';')
            for row in reader:
                try:
                    # Strip keys from row dict to handle spaces in CSV headers
                    row_clean = {k.strip(): v for k, v in row.items()}
                    
                    timestamp = int(row_clean["timestamp"].strip())
                    date_obj = datetime.datetime.fromtimestamp(timestamp)
                    date = int(time.mktime(date_obj.timetuple()))
                    
                    dVperiodical = float(row_clean["dVperiodical"].strip()) if row_clean["dVperiodical"].strip() else 0
                    dVcompelled = float(row_clean["dVcompelled"].strip()) if row_clean["dVcompelled"].strip() else 0
                    value = dVperiodical if dVperiodical else dVcompelled
                    
                    all_timestamps.append((date, date_obj, value))
                except (ValueError, KeyError):
                    continue
    except IOError as e:
        print(f"Error reading CSV: {e}")
        sys.exit(1)
    
    if not all_timestamps:
        print("Warning: No valid data found in CSV")
        return aggregated
    
    # For weekly: get latest datapoint and go back 7 days
    if graph_type == "weekly":
        latest_date = max(all_timestamps, key=lambda x: x[0])[1]
        week_start = latest_date - datetime.timedelta(days=7)
        week_start = week_start.replace(hour=0, minute=0, second=0, microsecond=0)
        week_end = latest_date.replace(hour=23, minute=59, second=59, microsecond=999999)
        
        start_ts = int(time.mktime(week_start.timetuple()))
        end_ts = int(time.mktime(week_end.timetuple())) + 1
    
    # Aggregate data
    for date, date_obj, value in all_timestamps:
        if date >= start_ts and date < end_ts:
            if graph_type == "daily":
                key = date_obj.strftime("%Y-%m-%d")
            elif graph_type == "weekly":
                key = date_obj.strftime("%A (%Y-%m-%d)")
            elif graph_type == "yearly":
                key = date_obj.strftime("%Y")
            else:  # custom
                key = date_obj.strftime("%Y-%m-%d")
            
            aggregated.setdefault(key, []).append(value)
    
    return aggregated


# Call Append_CSV.py to process and update the CSV file
try:
    print("Processing CSV data with Append_CSV.py...")
    result = subprocess.run(["python", "Append_CSV.py"], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running Append_CSV.py: {result.stderr}")
        sys.exit(1)
    print(result.stdout)
except Exception as e:
    print(f"Error calling Append_CSV.py: {e}")
    sys.exit(1)

# Calculate volume and aggregate data
try:
    aggregated_values = aggregate_data(tankCsv, start_ts, end_ts, selected_graph_type)
    
    # If no data in requested range, try to use all available data
    if not aggregated_values:
        print("Warning: No data in requested date range. Using all available data...")
        aggregated_values = aggregate_data(tankCsv, 0, int(time.time()) + 86400*365*10, selected_graph_type)
    
    if not aggregated_values:
        print("Error: No data available for selected date range")
        sys.exit(1)
    
    labels = sorted(aggregated_values.keys())
    values = [sum(aggregated_values[label]) for label in labels]
    
    if values:
        avg_value = sum(values) / len(values)
    else:
        avg_value = 0
    
    # Draw graph
    plt.figure(figsize=(14, 7))
    bars = plt.bar(range(len(labels)), values, color='steelblue')
    
    # Set x-axis ticks and labels
    plt.xticks(range(len(labels)), labels, rotation=45, ha='right', fontsize=9)
    
    plt.title(f"Water Usage - {selected_graph_type.upper()}\n{dateStart} to {dateEnd}\nAverage: {avg_value:.2f} L", fontsize=12)
    plt.xlabel("Period", fontsize=11)
    plt.ylabel("Water Usage (L)", fontsize=11)
    plt.grid(axis='y', alpha=0.3)
    
    # Add value labels on top of bars
    for i, (bar, value) in enumerate(zip(bars, values)):
        plt.text(bar.get_x() + bar.get_width()/2, bar.get_height(), 
                f'{value:.1f}', ha='center', va='bottom', fontsize=8)
    
    plt.tight_layout()
    plt.savefig(f"{selected_graph_type}_tank{currentTank}.png", dpi=300, bbox_inches='tight')
    print(f"\nGraph saved: {selected_graph_type}_tank{currentTank}.png ({dateStart} to {dateEnd})")
    
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)