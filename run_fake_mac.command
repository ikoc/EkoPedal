#!/bin/zsh
set -e
cd "${0:A:h}"
exec /Users/etkaguler/.local/miniforge3/envs/ko-pedal/bin/python desktop/main.py --fake
