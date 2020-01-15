#!/bin/sh
make -C /Users/ccmac/Documents/L01/build -f /Users/ccmac/Documents/L01/build/CMakeScripts/ZERO_CHECK_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all
