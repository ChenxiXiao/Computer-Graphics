# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.final_project.Debug:
/Users/ccmac/Documents/GitHub/CPE471/P4/build/Debug/final_project:\
	/usr/local/lib/libglfw.3.3.dylib
	/bin/rm -f /Users/ccmac/Documents/GitHub/CPE471/P4/build/Debug/final_project


PostBuild.final_project.Release:
/Users/ccmac/Documents/GitHub/CPE471/P4/build/Release/final_project:\
	/usr/local/lib/libglfw.3.3.dylib
	/bin/rm -f /Users/ccmac/Documents/GitHub/CPE471/P4/build/Release/final_project


PostBuild.final_project.MinSizeRel:
/Users/ccmac/Documents/GitHub/CPE471/P4/build/MinSizeRel/final_project:\
	/usr/local/lib/libglfw.3.3.dylib
	/bin/rm -f /Users/ccmac/Documents/GitHub/CPE471/P4/build/MinSizeRel/final_project


PostBuild.final_project.RelWithDebInfo:
/Users/ccmac/Documents/GitHub/CPE471/P4/build/RelWithDebInfo/final_project:\
	/usr/local/lib/libglfw.3.3.dylib
	/bin/rm -f /Users/ccmac/Documents/GitHub/CPE471/P4/build/RelWithDebInfo/final_project




# For each target create a dummy ruleso the target does not have to exist
/usr/local/lib/libglfw.3.3.dylib:
