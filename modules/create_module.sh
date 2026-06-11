#!/usr/bin/env bash
#
# Creates a new ShatterCore module from the skeleton template in
# doc/module-skeleton. Run from anywhere; the module is created under
# the modules/ directory next to this script.
#
# Usage: ./create_module.sh mod-my-feature

set -euo pipefail

MODULES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKELETON_DIR="${MODULES_DIR}/../doc/module-skeleton"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 mod-<name>"
    exit 1
fi

MODULE_NAME="$1"

if [[ ! "${MODULE_NAME}" =~ ^mod-[a-z0-9-]+$ ]]; then
    echo "Error: module name must match mod-<lowercase-name> (e.g. mod-my-feature)."
    exit 1
fi

TARGET_DIR="${MODULES_DIR}/${MODULE_NAME}"

if [[ -e "${TARGET_DIR}" ]]; then
    echo "Error: ${TARGET_DIR} already exists."
    exit 1
fi

if [[ ! -d "${SKELETON_DIR}" ]]; then
    echo "Error: skeleton template not found at ${SKELETON_DIR}."
    exit 1
fi

# Token forms derived from the module name
UNDERSCORED="${MODULE_NAME//-/_}"                    # mod_my_feature
STRIPPED="${MODULE_NAME#mod-}"                       # my-feature
CAMEL="$(echo "${STRIPPED}" | sed -E 's/(^|-)([a-z])/\U\2/g')"  # MyFeature

cp -r "${SKELETON_DIR}" "${TARGET_DIR}"

# Rename files containing skeleton tokens
find "${TARGET_DIR}" -depth -name '*skeleton*' | while read -r path; do
    newpath="$(dirname "${path}")/$(basename "${path}" | sed "s/mod_skeleton/${UNDERSCORED}/g; s/mod-skeleton/${MODULE_NAME}/g; s/skeleton/${STRIPPED}/g")"
    mv "${path}" "${newpath}"
done

# Replace tokens inside files
find "${TARGET_DIR}" -type f | while read -r file; do
    sed -i "s/mod_skeleton/${UNDERSCORED}/g; s/mod-skeleton/${MODULE_NAME}/g; s/Skeleton/${CAMEL}/g; s/skeleton/${UNDERSCORED#mod_}/g" "${file}"
done

echo "Module created at ${TARGET_DIR}."
echo "Loader function: Add${UNDERSCORED}Scripts()"
echo "Re-run CMake to pick it up."
