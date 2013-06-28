########################################################################
# Pointwise - Proprietary software product of Pointwise, Inc.
#             Copyright (c) 1995-2012 Pointwise, Inc.
#             All rights reserved.
#
# modulelocal.mk for src\plugins\CaeUnsSU2 plugin
########################################################################

#-----------------------------------------------------------------------
# OPTIONAL, locally defined plugin make targets. If a plugin developer wants
# to extend a plugin's make scheme, they should create a modulelocal.mk file
# in the plugin's base folder. To provide for future SDK upgrades, the standard
# module.mk file should NOT be directly edited.
#
# Uncomment, copy and/or edit the sections below as needed.
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Adds plugin specific source files to the build.
# These files will be compiled and then linked to the plugin.
# The file paths are relative to the project path macro CaeUnsSU2_LOC.
# For example,
#    myFile.cxx is located in $(CaeUnsSU2_LOC)/myFile.cxx
#    sub/myOtherFile.cxx is located in $(CaeUnsSU2_LOC)/sub/myOtherFile.cxx
#
#CaeUnsSU2_CXXFILES_PRIVATE := \
#    yourfile.c \
#    yourfile.cxx \
#	$(NULL)

#-----------------------------------------------------------------------
# Adds plugin specific include flags to the build.
#
#CaeUnsSU2_INCL_PRIVATE := \
#	-I./path/to/an/include/folder \
#	-I./path/to/another/include/folder \
#	$(NULL)

#-----------------------------------------------------------------------
# Adds plugin specific compile flags to the build.
#
#CaeUnsSU2_CXXFLAGS_PRIVATE := \
#	$(NULL)


#-----------------------------------------------------------------------
# Adds plugin specific -lfile and -Llibpath to build.
# These flags will be added to the link.
#
#CaeUnsSU2_LIBS_PRIVATE := \
#	-L./path/to/a/lib/folder \
#	-la \
#	-L./path/to/another/lib/folder \
#	-lanother \
#	$(NULL)

#-----------------------------------------------------------------------
# Adds plugin specific link flags to the build.
#
#CaeUnsSU2_LDFLAGS_PRIVATE := \
#	$(NULL)

#-----------------------------------------------------------------------
# Add any locally defined targets that do NOT produce an output object.
# This would include targets used for cleaning, printing, etc. These targets
# will be automatically added the .PHONY target.
#
#CaeUnsSU2_MAINT_TARGETS_PRIVATE = \
#	$(NULL)

#-----------------------------------------------------------------------
# Sample macro. Prefix with CAE name to prevent conflicts.
#
#CaeUnsSU2_DUMMY = \
#	DUMMY1 \
#	DUMMY2 \
#	$(NULL)

#-----------------------------------------------------------------------
# Sample target. Prefix with CAE name to prevent conflicts.
#
#CaeUnsSU2_sample: CaeUnsSU2_clean CaeUnsSU2
#	@echo "done building CaeUnsSU2"
