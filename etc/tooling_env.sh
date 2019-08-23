#!/bin/bash

# This is just a very basic script that launches a "virtual environment"
# with enabled permissions to collect CPU-level performance data. This
# script should be used before running the 'perf' tool on a binary for
# code instrumentation purposes.

echo "Entering Tooling Env"
sudo sysctl -w kernel.perf_event_paranoid=1
/bin/bash
sudo sysctl -w kernel.perf_event_paranoid=3
echo "Exiting Tooling Env"
