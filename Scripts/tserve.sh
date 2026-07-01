#!/usr/bin/env bash
set -euo pipefail
# Run the IServer and Emulator to run a standalone binary.

# Store command line arguments for future parsing
ARGS=("$@")

# Launch the emulator in the background and capture its PID.
temulate -le -x &
T_PID=$!

# Run IServer with the stored arguments, passing through all stdout/stderr
iserver -le "${ARGS[@]}"
I_EXIT=$?

# Ensure the emulator is terminated if still running...
if kill -0 "$T_PID" 2>/dev/null; then
    kill "$T_PID"
    wait "$T_PID" 2>/dev/null || true
fi

# Retrieve the emulator's exit code
wait "$T_PID" 2>/dev/null
T_EXIT=$?

# If the emulator exited with an error, notify and exit with its code
if [ "$T_EXIT" -ne 0 ]; then
    echo "Error: temulate exited with code $T_EXIT" >&2
    exit "$T_EXIT"
fi

# Otherwise exit with the IServer's exit code
exit "$I_EXIT"
