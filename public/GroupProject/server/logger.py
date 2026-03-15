import os
import json

def update_log(entry, log_dir="./server/log"):
    """
    Append the given entry to the JSON log file in the specified directory.
    """
    os.makedirs(log_dir, exist_ok=True)
    log_file = os.path.join(log_dir, "log.json")
    
    # Load existing logs if available.
    if os.path.exists(log_file):
        try:
            with open(log_file, "r") as f:
                logs = json.load(f)
        except Exception:
            logs = []
    else:
        logs = []
    
    logs.append(entry)
    with open(log_file, "w") as f:
        json.dump(logs, f, indent=4)
    
    print(f"Log updated with entry: {entry}")