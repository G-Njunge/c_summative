#!/bin/bash
# Linux System Health and Resource Monitoring Manager
# Run it with: ./system_health.sh

# Use the C locale so numbers always have a dot for decimals
export LC_ALL=C

# Keep the log next to the script, wherever it is run from
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
LOG_FILE="$SCRIPT_DIR/activity.log"
DEFAULT_REPORT_DIR="$SCRIPT_DIR/reports"

# Warning thresholds in percent, the user can change them from the menu
CPU_THRESHOLD=80
MEMORY_THRESHOLD=80
DISK_THRESHOLD=90

# Processes of system accounts have user IDs below this, so they are protected
FIRST_NORMAL_UID=1000

# Columns shown for processes, the name is last because it can have spaces
PROCESS_COLUMNS="pid,user,%cpu,%mem,stat,etime,comm"

LOG_ENABLED=1

# ===== Helper functions =====

# Writes one line with the date and time to the activity log
log_action() {
    if [ "$LOG_ENABLED" -eq 1 ]; then
        if ! printf '%s [%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$1" "$2" 2>/dev/null >> "$LOG_FILE"; then
            # Only warn once, so the screen is not filled with the same message
            echo "Warning: cannot write to $LOG_FILE, activity will not be logged." >&2
            LOG_ENABLED=0
        fi
    fi
}

# Reads one line into the variable named in $2, returns 1 if the input has ended
ask() {
    if ! read -r -p "$1" "$2"; then
        echo
        return 1
    fi
}

# Asks the user to type yes, returns 0 only if they did
confirm() {
    local answer

    ask "$1 Type 'yes' to confirm: " answer || return 1
    [ "${answer,,}" = "yes" ]
}

# Checks that a command exists, so a missing tool gives a clear message
require_command() {
    if ! command -v "$1" > /dev/null 2>&1; then
        echo "Error: the '$1' command is not available on this system."
        log_action "ERROR" "Command '$1' is not available"
        return 1
    fi
}

# Asks for a threshold until it is a whole number from 1 to 100, Enter keeps the current one
read_threshold() {
    local name=$1
    local var=$2
    local value

    while true; do
        ask "Enter the $name warning threshold in % (1-100) [${!var}]: " value || return 1

        if [ -z "$value" ]; then
            return 0
        fi

        # Digits only, so text, decimals and negative numbers are rejected, 10# stops 08 being read as octal
        if [[ $value =~ ^[0-9]{1,3}$ ]] && (( 10#$value >= 1 && 10#$value <= 100 )); then
            printf -v "$var" '%d' "$(( 10#$value ))"
            log_action "INFO" "$name threshold set to ${!var}%"
            return 0
        fi

        echo "Invalid threshold. Enter a whole number from 1 to 100."
    done
}

# Returns 0 if the first number is above the second, awk is used because bash cannot compare decimals
is_above() {
    awk -v a="$1" -v b="$2" 'BEGIN { exit !(a > b) }'
}

# ===== Monitoring =====

# Shows the hostname, operating system, kernel, uptime and the date and time
display_system_info() {
    local os="Unknown"

    echo "===== SYSTEM INFORMATION ====="
    echo "Date and time    : $(date '+%Y-%m-%d %H:%M:%S %Z')"

    # PRETTY_NAME holds the full name of the distribution
    if [ -r /etc/os-release ]; then
        os=$(grep -m 1 '^PRETTY_NAME=' /etc/os-release | cut -d '=' -f 2- | tr -d '"')
    fi

    if require_command uname; then
        echo "Hostname         : $(uname -n)"
        echo "Operating system : ${os:-Unknown} ($(uname -o))"
        echo "Kernel version   : $(uname -r)"
        echo "Architecture     : $(uname -m)"
    fi

    if require_command uptime; then
        echo "Uptime           : $(uptime -p)"
        echo "Running since    : $(uptime -s)"
    fi

    log_action "INFO" "Displayed system information"
}

# Shows CPU usage and the busiest processes, the report passes the threshold so the user is not asked
monitor_cpu() {
    local threshold=$1
    local first second total=0 idle usage cores high i

    if [ -z "$threshold" ]; then
        read_threshold "CPU" CPU_THRESHOLD || return 1
        threshold=$CPU_THRESHOLD
    fi

    echo "===== CPU UTILIZATION ====="

    if [ ! -r /proc/stat ]; then
        echo "Error: cannot read /proc/stat, CPU usage is unavailable."
        log_action "ERROR" "Cannot read /proc/stat"
        return 1
    fi

    # Read the CPU time counters twice, one second apart, and compare them
    read -r -a first < /proc/stat
    sleep 1
    read -r -a second < /proc/stat

    # Fields 1 to 8 are user, nice, system, idle, iowait, irq, softirq and steal time
    for i in 1 2 3 4 5 6 7 8; do
        total=$(( total + second[i] - first[i] ))
    done
    idle=$(( second[4] + second[5] - first[4] - first[5] ))
    usage=$(awk -v t="$total" -v i="$idle" 'BEGIN { if (t > 0) printf "%.1f", (t - i) * 100 / t; else print "0.0" }')

    cores=$(grep -c '^processor' /proc/cpuinfo 2>/dev/null)
    echo "CPU cores        : ${cores:-Unknown}"
    echo "Load average     : $(cut -d ' ' -f 1-3 /proc/loadavg 2>/dev/null) (1, 5 and 15 minutes)"
    echo "CPU usage        : $usage% (warning above $threshold%)"
    log_action "INFO" "CPU usage checked: $usage%"

    if is_above "$usage" "$threshold"; then
        echo "WARNING: CPU usage is $usage%, above the $threshold% threshold."
        log_action "WARNING" "CPU usage $usage% is above the $threshold% threshold"
    fi

    require_command ps || return 1

    echo
    echo "Top 5 processes by CPU (average since each process started):"
    ps -eo "$PROCESS_COLUMNS" --sort=-%cpu | head -n 6

    # Fields are PID, CPU and the name, which may contain spaces
    high=$(ps -eo pid=,%cpu=,comm= --sort=-%cpu | awk -v t="$threshold" '$2 > t { n = $3; for (i = 4; i <= NF; i++) n = n " " $i; print $1 " " $2 " " n }')

    if [ -n "$high" ]; then
        echo
        while read -r pid cpu name; do
            echo "WARNING: $name (PID $pid) is using $cpu% CPU, above the $threshold% threshold."
            log_action "WARNING" "Process $name (PID $pid) is using $cpu% CPU"
        done <<< "$high"
    fi
}

# Shows total, used and available memory and warns above the threshold
monitor_memory() {
    local threshold=$1
    local total available used percent

    if [ -z "$threshold" ]; then
        read_threshold "memory" MEMORY_THRESHOLD || return 1
        threshold=$MEMORY_THRESHOLD
    fi

    echo "===== MEMORY UTILIZATION ====="
    require_command free || return 1

    # Columns 2 and 7 of the Mem line are total and available, in bytes
    read -r total available < <(free -b | awk '/^Mem:/ { print $2, $7 }')

    if [[ ! $total =~ ^[0-9]+$ ]] || [[ ! $available =~ ^[0-9]+$ ]] || (( total == 0 )); then
        echo "Error: could not read the memory information."
        log_action "ERROR" "Could not read the output of free"
        return 1
    fi

    # Used is everything that is not available, so caches that can be freed do not count
    used=$(( total - available ))
    percent=$(awk -v u="$used" -v t="$total" 'BEGIN { printf "%.1f", u * 100 / t }')

    awk -v t="$total" -v u="$used" -v a="$available" 'BEGIN {
        printf "Total memory     : %.2f GiB\n", t / 1073741824
        printf "Used memory      : %.2f GiB\n", u / 1073741824
        printf "Available memory : %.2f GiB\n", a / 1073741824
    }'
    echo "Memory usage     : $percent% (warning above $threshold%)"
    log_action "INFO" "Memory usage checked: $percent%"

    if is_above "$percent" "$threshold"; then
        echo "WARNING: memory usage is $percent%, above the $threshold% threshold."
        log_action "WARNING" "Memory usage $percent% is above the $threshold% threshold"
    fi
}

# Shows the space on each mounted filesystem and warns about the ones above the threshold
monitor_disk() {
    local threshold=$1
    local table usage mount found=0

    if [ -z "$threshold" ]; then
        read_threshold "disk" DISK_THRESHOLD || return 1
        threshold=$DISK_THRESHOLD
    fi

    echo "===== DISK UTILIZATION ====="
    require_command df || return 1

    # Leave out filesystems that only live in memory, and snap images, which are read-only and always full
    table=$(df -h -x tmpfs -x devtmpfs -x squashfs -x fuse.snapfuse --output=source,fstype,size,used,avail,pcent,target 2>/dev/null)

    if [ -z "$table" ]; then
        echo "Error: could not read the filesystem information."
        log_action "ERROR" "Could not read the output of df"
        return 1
    fi

    echo "$table"
    echo
    log_action "INFO" "Disk usage checked"

    # The mount point is read last because it can contain spaces
    while read -r usage mount; do
        usage=${usage%\%}

        # Some filesystems show - because they have no size
        if [[ $usage =~ ^[0-9]+$ ]] && is_above "$usage" "$threshold"; then
            echo "WARNING: $mount is $usage% full, above the $threshold% threshold."
            log_action "WARNING" "Filesystem $mount is $usage% full"
            found=1
        fi
    done < <(df -x tmpfs -x devtmpfs -x squashfs -x fuse.snapfuse --output=pcent,target 2>/dev/null | tail -n +2)

    if [ "$found" -eq 0 ]; then
        echo "No filesystem is above the $threshold% threshold."
    fi
}

# ===== Processes =====

# Lists the busiest processes, then searches by name or PID
view_processes() {
    local term result

    require_command ps || return 1

    echo "===== RUNNING PROCESSES ====="
    echo "Total running processes: $(ps -e --no-headers | wc -l)"
    echo "Top 15 by CPU:"
    ps -eo "$PROCESS_COLUMNS" --sort=-%cpu | head -n 16
    log_action "INFO" "Viewed running processes"

    echo
    ask "Search by name or PID (press Enter to go back): " term || return 1

    if [ -z "$term" ]; then
        return 0
    fi

    if [[ $term =~ ^[0-9]+$ ]]; then
        # PIDs on Linux have at most 7 digits, longer ones would make ps fail
        if (( ${#term} <= 7 )) && result=$(ps -p "$(( 10#$term ))" -o "$PROCESS_COLUMNS" 2>/dev/null); then
            echo "$result"
            log_action "INFO" "Searched for PID $term: found"
        else
            echo "No process with PID $term is running."
            log_action "INFO" "Searched for PID $term: not found"
        fi
        return 0
    fi

    # Match part of the name and ignore capitals, ENVIRON passes the text without awk changing backslashes
    result=$(ps -eo "$PROCESS_COLUMNS" --sort=-%cpu | SEARCH="$term" awk '
        NR == 1 { print; next }
        {
            name = $7
            for (i = 8; i <= NF; i++) name = name " " $i
            if (index(tolower(name), tolower(ENVIRON["SEARCH"])) > 0) print
        }')

    if [ "$(echo "$result" | wc -l)" -le 1 ]; then
        echo "No running process matches '$term'."
        log_action "INFO" "Searched for '$term': not found"
    else
        echo "$result"
        log_action "INFO" "Searched for '$term': $(( $(echo "$result" | wc -l) - 1 )) match(es)"
    fi
}

# Terminates a process after checking it is allowed and the user confirms
terminate_process() {
    local pid uid user name i

    require_command ps || return 1

    ask "Enter the PID of the process to terminate: " pid || return 1

    if [[ ! $pid =~ ^[0-9]{1,7}$ ]] || (( 10#$pid == 0 )); then
        echo "Invalid PID. Enter a whole number above 0."
        return 1
    fi
    pid=$(( 10#$pid ))

    read -r uid user name < <(ps -p "$pid" -o uid=,user=,comm= 2>/dev/null)

    if [ -z "$uid" ]; then
        echo "No process with PID $pid is running."
        log_action "ERROR" "Terminate failed: PID $pid does not exist"
        return 1
    fi

    # PID 1 starts the whole system, and stopping this script or its shell would end the program
    if (( pid == 1 || pid == $$ || pid == PPID )); then
        echo "Refused: $name (PID $pid) is protected and cannot be terminated."
        log_action "WARNING" "Refused to terminate protected process $name (PID $pid)"
        return 1
    fi

    if (( uid < FIRST_NORMAL_UID )); then
        echo "Refused: $name (PID $pid) is a system process owned by $user."
        log_action "WARNING" "Refused to terminate system process $name (PID $pid) owned by $user"
        return 1
    fi

    # Only root can stop processes that belong to another user
    if (( EUID != 0 && uid != EUID )); then
        echo "Permission denied: $name (PID $pid) belongs to $user."
        log_action "WARNING" "No permission to terminate $name (PID $pid) owned by $user"
        return 1
    fi

    ps -p "$pid" -o "$PROCESS_COLUMNS"

    if ! confirm "Terminate $name (PID $pid)?"; then
        echo "Cancelled. The process was not terminated."
        log_action "INFO" "Cancelled terminating $name (PID $pid)"
        return 0
    fi

    if ! kill -TERM "$pid" 2>/dev/null; then
        echo "Error: could not terminate $name (PID $pid). It may have ended or you lack permission."
        log_action "ERROR" "Could not terminate $name (PID $pid)"
        return 1
    fi

    # Give the process up to 3 seconds to stop
    for i in 1 2 3; do
        kill -0 "$pid" 2>/dev/null || break
        sleep 1
    done

    if kill -0 "$pid" 2>/dev/null; then
        echo "Sent the stop signal to $name (PID $pid), but it is still running."
        log_action "WARNING" "Sent SIGTERM to $name (PID $pid), still running"
    else
        echo "Terminated $name (PID $pid)."
        log_action "ACTION" "Terminated $name (PID $pid) owned by $user"
    fi
}

# ===== Reports and the log =====

# Saves a report with the system information, CPU, memory and disk in a folder the user picks
generate_report() {
    local dir file

    ask "Folder to save the report in [$DEFAULT_REPORT_DIR]: " dir || return 1
    dir=${dir:-$DEFAULT_REPORT_DIR}

    # Turn a ~ at the start into the home folder, because quotes stop bash doing it
    dir=${dir/#\~/$HOME}

    if [ -e "$dir" ] && [ ! -d "$dir" ]; then
        echo "Error: $dir is a file, not a folder."
        log_action "ERROR" "Report not saved: $dir is not a folder"
        return 1
    fi

    if [ ! -d "$dir" ]; then
        if ! confirm "$dir does not exist. Create it?"; then
            echo "Cancelled. No report was saved."
            return 0
        fi

        if ! mkdir -p -- "$dir" 2>/dev/null; then
            echo "Error: cannot create $dir. Check the permissions."
            log_action "ERROR" "Report not saved: cannot create $dir"
            return 1
        fi
    fi

    # Writing needs w, and opening files inside the folder needs x
    if [ ! -w "$dir" ] || [ ! -x "$dir" ]; then
        echo "Error: you do not have permission to write to $dir."
        log_action "ERROR" "Report not saved: no permission to write to $dir"
        return 1
    fi

    file="$dir/health_report_$(date '+%Y%m%d_%H%M%S').txt"

    # Check the file can be created before collecting anything
    if ! { : > "$file"; } 2>/dev/null; then
        echo "Error: cannot create $file."
        log_action "ERROR" "Could not create the report $file"
        return 1
    fi

    echo "Collecting the data..."

    # The report uses the saved thresholds, so the user is not asked again
    {
        echo "===== SYSTEM HEALTH REPORT ====="
        echo "Generated on     : $(date '+%Y-%m-%d %H:%M:%S %Z')"
        echo "Thresholds       : CPU $CPU_THRESHOLD%, memory $MEMORY_THRESHOLD%, disk $DISK_THRESHOLD%"
        echo
        display_system_info
        echo
        monitor_cpu "$CPU_THRESHOLD"
        echo
        monitor_memory "$MEMORY_THRESHOLD"
        echo
        monitor_disk "$DISK_THRESHOLD"
    } > "$file"

    # A missing tool only leaves out one section, but an empty file means writing failed
    if [ ! -s "$file" ]; then
        echo "Error: could not write the report to $file."
        log_action "ERROR" "Could not write the report to $file"
        rm -f -- "$file" 2>/dev/null
        return 1
    fi

    echo "Report saved to $file"
    log_action "ACTION" "Health report saved to $file"
}

# Shows every line of the activity log
view_log() {
    if [ ! -s "$LOG_FILE" ]; then
        echo "The activity log is empty."
        return 0
    fi

    if [ ! -r "$LOG_FILE" ]; then
        echo "Error: you do not have permission to read $LOG_FILE."
        return 1
    fi

    echo "===== ACTIVITY LOG ($(wc -l < "$LOG_FILE") entries) ====="
    cat "$LOG_FILE"
    log_action "INFO" "Viewed the activity log"
}

# Empties the activity log after the user confirms
clear_log() {
    if [ ! -s "$LOG_FILE" ]; then
        echo "The activity log is already empty."
        return 0
    fi

    if ! confirm "This deletes all $(wc -l < "$LOG_FILE") entries in the activity log."; then
        echo "Cancelled. The log was not cleared."
        log_action "INFO" "Cancelled clearing the activity log"
        return 0
    fi

    # The braces let 2>/dev/null hide the error if the file cannot be opened
    if ! { : > "$LOG_FILE"; } 2>/dev/null; then
        echo "Error: you do not have permission to clear $LOG_FILE."
        return 1
    fi

    echo "The activity log was cleared."
    LOG_ENABLED=1
    log_action "ACTION" "Activity log cleared"
}

# ===== Menu =====

# Shows the menu until the user exits or the input ends
main() {
    local choice

    # The commands and /proc files used here only exist on Linux
    if [[ $OSTYPE != linux* ]]; then
        echo "Error: this script must be run on Linux."
        exit 1
    fi

    # Ctrl+C still leaves a record in the log
    trap 'echo; log_action "INFO" "Session ended with Ctrl+C"; exit 130' INT

    log_action "INFO" "Session started by $(id -un)"

    while true; do
        echo
        echo "===== SYSTEM HEALTH AND RESOURCE MONITOR ====="
        echo "1. Display system information"
        echo "2. Monitor CPU utilization"
        echo "3. Monitor memory utilization"
        echo "4. Monitor disk utilization"
        echo "5. View and search running processes"
        echo "6. Terminate a process"
        echo "7. Generate a system health report"
        echo "8. View activity history"
        echo "9. Clear activity log"
        echo "10. Exit"

        # Treat the end of the input like choosing Exit
        ask "Enter your choice (1-10): " choice || choice=10
        echo

        case $choice in
            1) display_system_info ;;
            2) monitor_cpu ;;
            3) monitor_memory ;;
            4) monitor_disk ;;
            5) view_processes ;;
            6) terminate_process ;;
            7) generate_report ;;
            8) view_log ;;
            9) clear_log ;;
            10)
                echo "Exiting..."
                log_action "INFO" "Session ended"
                exit 0
                ;;
            *) echo "Invalid choice. Enter a number from 1 to 10." ;;
        esac
    done
}

main
