import os
import datetime
import time
import csv
import json
import matplotlib.pyplot as plt
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Load graph configuration from JSON
with open("graph_config.json", "r") as config_file:
    config = json.load(config_file)

# Load tank settings
with open("tank_settings.json", "r") as settings_file:
    settings = json.load(settings_file)

def calculate_tank_volume(tankCsv, tank_config):
    tank_type = tank_config.get("tankType", {})
    
    # Determine if cylindrical (key "1") or rectangular (key "0")
    is_cylindrical = "1" in tank_type and tank_type["1"] == "cylindrical"
    
    if is_cylindrical:
        diameter = float(tank_config.get("cylindrical", {}).get("diameter", 0))
        height = float(tank_config.get("cylindrical", {}).get("height", 0))
        max_volume = 3.14159 * (diameter / 2) ** 2 * height
    else:
        width = float(tank_config.get("rectangular", {}).get("width", 0))
        length = float(tank_config.get("rectangular", {}).get("length", 0))
        height = float(tank_config.get("rectangular", {}).get("height", 0))
        max_volume = width * length * height
    
    if max_volume <= 0:
        print("Error: Invalid tank dimensions (volume <= 0)")
        sys.exit(1)
    
    rows = []
    sumChange = 0  # Track total water used from the start
    fieldnames = []
    lastProcessedRowIndex = -1  # Track last row that was already processed
    
    try:
        with open(tankCsv, "r") as df:
            reader = csv.DictReader(df, delimiter=';')
            fieldnames = list(reader.fieldnames) if reader.fieldnames else []
            
            if not fieldnames:
                print(f"Error: {tankCsv} has no headers or is empty")
                sys.exit(1)
            
            # Strip whitespace from fieldnames and remove empty columns
            fieldnames = [col.strip() for col in fieldnames if col and col.strip()]
            
            # Check if currentVolume and MaxVolume columns already exist
            has_calculated_volumes = "currentVolume" in fieldnames and "MaxVolume" in fieldnames
            
            # Remove any existing new columns to ensure recalculation
            new_columns = ["Serial/Compelled", "dVsum", "date", "currentVolume", "MaxVolume"]
            fieldnames = [col for col in fieldnames if col not in new_columns]
            
            # Add new columns to fieldnames
            for col in new_columns:
                fieldnames.append(col)
            
            # First pass: read all rows to find the last one with calculated currentVolume
            all_rows_data = []
            if has_calculated_volumes:
                with open(tankCsv, "r") as df2:
                    reader2 = csv.DictReader(df2, delimiter=';')
                    for row in reader2:
                        all_rows_data.append(row)
                    
                    # Find the last row with a valid currentVolume value
                    for i in range(len(all_rows_data) - 1, -1, -1):
                        last_vol_str = all_rows_data[i].get("currentVolume", "").strip()
                        if last_vol_str:
                            try:
                                sumChange = max_volume - float(last_vol_str)
                                lastProcessedRowIndex = i
                                print(f"Starting from row {i + 1} with currentVolume: {last_vol_str} L")
                                break
                            except ValueError:
                                continue
            
            # Re-read the file to process rows
            with open(tankCsv, "r") as df:
                reader = csv.DictReader(df, delimiter=';')
                for row_idx, row in enumerate(reader):
                    try:
                        # Strip keys from row dict to handle spaces in CSV headers
                        # Also filter out empty keys from trailing semicolons
                        row_clean = {k.strip(): v for k, v in row.items() if k and k.strip()}
                        
                        # Get timestamp and convert to date
                        timestamp = int(row_clean.get("timestamp", "0").strip()) if row_clean.get("timestamp", "").strip() else 0
                        if timestamp > 0:
                            date_obj = datetime.datetime.fromtimestamp(timestamp)
                            row_clean["date"] = date_obj.strftime("%Y-%m-%d")
                        else:
                            row_clean["date"] = ""
                        
                        # Calculate dVsum (sum of periodical and compelled)
                        dVperiodical = float(row_clean.get("dVperiodical", "0").strip()) if row_clean.get("dVperiodical", "").strip() else 0
                        dVcompelled = float(row_clean.get("dVcompelled", "0").strip()) if row_clean.get("dVcompelled", "").strip() else 0
                        
                        dVsum = dVperiodical + dVcompelled
                        row_clean["dVsum"] = f"{dVsum}"
                        
                        # Determine if periodic (P) or compelled (C) or both (P+C)
                        if dVperiodical > 0 and dVcompelled > 0:
                            serial_compelled = "P+C"
                        elif dVperiodical > 0:
                            serial_compelled = "P"
                        elif dVcompelled > 0:
                            serial_compelled = "C"
                        else:
                            serial_compelled = ""
                        
                        # Track cumulative changes and calculate current volume
                        # Start from the last calculated row, then work forward
                        if row_idx > lastProcessedRowIndex:
                            sumChange += dVsum
                        
                        currentVolume = max_volume - sumChange
                        if currentVolume < 0:
                            currentVolume = 0 
                        row_clean["currentVolume"] = f"{currentVolume}"
                        row_clean["MaxVolume"] = f"{max_volume}"
                        row_clean["Serial/Compelled"] = serial_compelled
                    
                        rows.append(row_clean)
                    except (ValueError, AttributeError) as e:
                        print(f"Warning: Skipping malformed row")
                        continue
        
        if not rows:
            print(f"Error: No valid data in {tankCsv}")
            sys.exit(1)
        
        # Write with all fieldnames including new columns
        with open(tankCsv, "w", newline='') as df:
            writer = csv.DictWriter(df, fieldnames=fieldnames, delimiter=';')
            writer.writeheader()
            writer.writerows(rows)
        
        print(f"Processing: {tankCsv} ({len(rows)} rows)")
        print(f"Final current volume: {currentVolume:.2f} L")
        
    except IOError as e:
        print(f"Error reading/writing file: {e}")
        sys.exit(1)


# Main execution
if __name__ == "__main__":
    # Get current tank from settings
    currentTank = str(settings.get("index", {}).get("currentTank", "0"))
    
    # Validate currentTank is not "0" (no tank selected)
    if currentTank == "0" or not currentTank:
        print("Error: No tank currently selected in tank_settings.json")
        sys.exit(1)
    
    # Find tank configuration
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
        sys.exit(1)
    
    # Process the CSV file
    print(f"Processing {tankCsv} for tank {currentTank}...")
    calculate_tank_volume(tankCsv, tank_config)