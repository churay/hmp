#!/bin/bash

# This script invokes the simulation application and generates a replay
# movie using the debug re-simulation feature.

REPLAY_ID=${1}
REPLAY_TYPE=${2:-mp4}
REPLAY_FPS=60

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
PROJ_PATH=$(dirname ${SCRIPT_PATH})
OUTPUT_PATH=${PROJ_PATH}/build/install/out

REPLAY_BASE_PATH=${OUTPUT_PATH}/replay${REPLAY_ID}.mp4
REPLAY_PATH=${OUTPUT_PATH}/replay${REPLAY_ID}.${REPLAY_TYPE}

# FFMPEG="ffmpeg"
FFMPEG="ffmpeg -hide_banner -loglevel panic"

EXIT_CODE=0

if ! [[ ${REPLAY_ID} =~ ^[0-9]+ ]]; then
    echo "ERROR: Please specify a replay number"
    EXIT_CODE=1
elif ! [[ -f "${OUTPUT_PATH}/input${REPLAY_ID}.dat" && -f "${OUTPUT_PATH}/state${REPLAY_ID}.dat" ]]; then
    echo "ERROR: Replay doesn't exist for replay number ${REPLAY_ID}"
    EXIT_CODE=2
else
    ${PROJ_PATH}/llce.out -r ${REPLAY_ID}
    ${FFMPEG} -framerate ${REPLAY_FPS} -r ${REPLAY_FPS} \
        -i ${OUTPUT_PATH}/render${REPLAY_ID}-%d.png -c:v libx264 \
        -pix_fmt yuv420p -y -crf 0 ${REPLAY_BASE_PATH}
    if [[ ${REPLAY_TYPE} = "gif" ]]; then
        REPLAY_TEMP_FPS=$((100 / ((100+${REPLAY_FPS}-1) / ${REPLAY_FPS})))
        REPLAY_TEMP_PATH=$(dirname ${REPLAY_BASE_PATH})/.$(basename ${REPLAY_BASE_PATH})
        REPLAY_PALETTE_PATH=$(dirname ${REPLAY_PATH})/.${REPLAY_ID}.png

        mv ${REPLAY_BASE_PATH} ${REPLAY_TEMP_PATH}
        ${FFMPEG} -i ${REPLAY_TEMP_PATH} -filter:v palettegen -y ${REPLAY_PALETTE_PATH}
        ${FFMPEG} -i ${REPLAY_TEMP_PATH} -i ${REPLAY_PALETTE_PATH} \
            -filter_complex "fps=${REPLAY_TEMP_FPS}[x];[x][1:v]paletteuse" -y ${REPLAY_PATH}
    elif ! [[ ${REPLAY_TYPE} = "mp4" ]]; then
        echo "WARNING: Unrecognized replay type ${REPLAY_TYPE}"
    fi

    echo "SUCCESS: Replay generated at ${REPLAY_PATH}"
    EXIT_CODE=0
fi

exit ${EXIT_CODE}
