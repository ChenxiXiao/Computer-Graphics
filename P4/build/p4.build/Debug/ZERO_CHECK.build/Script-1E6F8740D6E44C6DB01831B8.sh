#!/bin/sh
make -C /Users/ccmac/Documents/GitHub/CPE471/P4/build -f /Users/ccmac/Documents/GitHub/CPE471/P4/build/CMakeScripts/ZERO_CHECK_cmakeRulesBuildPhase.make$CONFIGURATION OBJDIR=$(basename "$OBJECT_FILE_DIR_normal") all
