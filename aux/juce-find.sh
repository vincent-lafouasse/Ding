#!/bin/bash

JUCE_MODULES_PATH=${JUCE_MODULES_PATH:-"./lib/juce/modules"}

if [ -z "$1" ]; then
    echo "Usage: $0 <ClassName>"
    exit 1
fi

CLASS_NAME=$1

DEFINITION_FILE=$(grep -rlE "class[[:space:]]+([A-Z_]+_API[[:space:]]+)?${CLASS_NAME}([[:space:]]|:|\{)" "$JUCE_MODULES_PATH" --include="*.h" | head -n 1)

if [ -z "$DEFINITION_FILE" ]; then
    echo "Error: Could not find definition for $CLASS_NAME"
    exit 1
fi

MODULE_ID=$(grep -oE "ID:[[:space:]]+juce_[a-z_]+" "$DEFINITION_FILE" | awk '{print $2}')

# If not found in the leaf header, check the parent directory name
if [ -z "$MODULE_ID" ]; then
    MODULE_ID=$(echo "$DEFINITION_FILE" | grep -oE "juce_[a-z_]+" | head -n 1)
fi

echo "--- JUCE Symbol Finder ---"
echo -e "Class:\t\t$CLASS_NAME"
echo -e "Definition:\t$DEFINITION_FILE"
echo -e "Module:\t\t$MODULE_ID"
echo "--------------------------"
echo "Header:"
echo -e "\t<$MODULE_ID/$MODULE_ID.h>"
echo
echo "cmake module:"
echo -e "\tjuce::$MODULE_ID"
