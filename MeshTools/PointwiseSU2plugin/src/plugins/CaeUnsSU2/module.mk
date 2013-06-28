########################################################################
# Pointwise - Proprietary software product of Pointwise, Inc.
#             Copyright (c) 1995-2012 Pointwise, Inc.
#             All rights reserved.
#
# module.mk for src\plugins\CaeUnsSU2 plugin
########################################################################

########################################################################
########################################################################
#
#                   DO NOT EDIT THIS FILE
#
# To simplify SDK upgrades, the standard module.mk file should NOT be edited.
#
# If you want to modify a plugin's build process, you should rename
# modulelocal-sample.mk to modulelocal.mk and edit its settings.
#
# See the comments in modulelocal-sample.mk for more information.
#
#                   DO NOT EDIT THIS FILE
#
########################################################################
########################################################################

CaeUnsSU2_LOC := $(PLUGINS_LOC)/CaeUnsSU2
CaeUnsSU2_LIB := CaeUnsSU2$(DBG_SUFFIX)
CaeUnsSU2_CXX_LOC := $(CaeUnsSU2_LOC)
CaeUnsSU2_OBJ_LOC := $(PLUGINS_OBJ_LOC)/CaeUnsSU2

CaeUnsSU2_FULLNAME := lib$(CaeUnsSU2_LIB).$(SHLIB_SUFFIX)
CaeUnsSU2_FULLLIB := $(PLUGINS_DIST_DIR)/$(CaeUnsSU2_FULLNAME)

CaeUnsSU2_DEPS = \
	$(NULL)

MODCXXFILES := \
	runtimeWrite.c \
	$(NULL)

# IMPORTANT:
# Must recompile the shared/XXX/.c files for each plugin. These .c files
# include the plugin specific settings defined in the ./CaeUnsSU2/*.h
# files.
CaeUnsSU2_SRC := \
	$(PLUGINS_RT_PWPSRC) \
	$(PLUGINS_RT_PWGMSRC) \
	$(PLUGINS_RT_CAEPSRC) \
	$(patsubst %,$(CaeUnsSU2_CXX_LOC)/%,$(MODCXXFILES))

CaeUnsSU2_SRC_C := $(filter %.c,$(MODCXXFILES))
CaeUnsSU2_SRC_CXX := $(filter %.cxx,$(MODCXXFILES))

# place the .o files generated from shared sources in the plugin's
# OBJ folder.
CaeUnsSU2_OBJ := \
	$(patsubst %.c,$(CaeUnsSU2_OBJ_LOC)/%.o,$(PLUGINS_RT_PWPFILES)) \
	$(patsubst %.c,$(CaeUnsSU2_OBJ_LOC)/%.o,$(PLUGINS_RT_PWGMFILES)) \
	$(patsubst %.c,$(CaeUnsSU2_OBJ_LOC)/%.o,$(PLUGINS_RT_CAEPFILES)) \
    $(patsubst %.c,$(CaeUnsSU2_OBJ_LOC)/%.o,$(CaeUnsSU2_SRC_C)) \
    $(patsubst %.cxx,$(CaeUnsSU2_OBJ_LOC)/%.o,$(CaeUnsSU2_SRC_CXX)) \
	$(NULL)

# To allow over-rides, search FIRST for headers in the local module's folder.
# For example, a site.h file in the local module's folder will preempt the
# file .../src/plugins/site.h
CaeUnsSU2_INCL = \
	-I$(CaeUnsSU2_LOC) \
	$(PLUGINS_RT_INCL) \
	$(NULL)

CaeUnsSU2_LIBS = \
	$(NULL)

CaeUnsSU2_MAINT_TARGETS := \
    CaeUnsSU2_info \
    CaeUnsSU2_install \
    CaeUnsSU2_installnow \
    CaeUnsSU2_uninstall


########################################################################
# Get (OPTIONAL) locally defined make targets. If a plugin developer wants
# to extend a plugin's make scheme, they should create a modulelocal.mk file
# in the plugin's base folder. To provide for future SDK upgrades, the standard
# module.mk file should NOT be directly edited.
#
ifneq ($(wildcard $(CaeUnsSU2_LOC)/modulelocal.mk),)
    CaeUnsSU2_DEPS += $(CaeUnsSU2_LOC)/modulelocal.mk
    include $(CaeUnsSU2_LOC)/modulelocal.mk
endif


# merge in plugin private settings

CaeUnsSU2_OBJ += \
    $(patsubst %.c,$(CaeUnsSU2_OBJ_LOC)/%.o,$(filter %.c,$(CaeUnsSU2_CXXFILES_PRIVATE))) \
    $(patsubst %.cxx,$(CaeUnsSU2_OBJ_LOC)/%.o,$(filter %.cxx,$(CaeUnsSU2_CXXFILES_PRIVATE))) \
	$(NULL)

CaeUnsSU2_SRC += \
	$(patsubst %,$(CaeUnsSU2_CXX_LOC)/%,$(CaeUnsSU2_CXXFILES_PRIVATE)) \
	$(NULL)

CaeUnsSU2_INCL += $(CaeUnsSU2_INCL_PRIVATE)
CaeUnsSU2_LIBS += $(CaeUnsSU2_LIBS_PRIVATE)
CaeUnsSU2_CXXFLAGS += $(CaeUnsSU2_CXXFLAGS_PRIVATE)
CaeUnsSU2_LDFLAGS += $(CaeUnsSU2_LDFLAGS_PRIVATE)
CaeUnsSU2_MAINT_TARGETS += $(CaeUnsSU2_MAINT_TARGETS_PRIVATE)
CaeUnsSU2_DEPS += $(CaeUnsSU2_DEPS_PRIVATE)

PLUGIN_MAINT_TARGETS += $(CaeUnsSU2_MAINT_TARGETS)
PLUGIN_OBJ += $(CaeUnsSU2_OBJ)

# add to plugin maint targets to the global .PHONY target
.PHONY: \
	$(CaeUnsSU2_MAINT_TARGETS) \
	$(NULL)


########################################################################
# Set the final build macros
CaeUnsSU2_CXXFLAGS := $(CXXFLAGS) $(PLUGINS_STDDEFS) $(CaeUnsSU2_INCL) -DPWGM_HIDE_STRUCTURED_API

ifeq ($(machine),macosx)
CaeUnsSU2_LDFLAGS = -install_name "$(REL_BIN_TO_PW_LIB_DIR)/$(CaeUnsSU2_FULLNAME)"
else
CaeUnsSU2_LDFLAGS =
endif


########################################################################
# list of plugin's build targets
#
CaeUnsSU2: $(CaeUnsSU2_FULLLIB)

$(CaeUnsSU2_FULLLIB): $(CaeUnsSU2_OBJ) $(CaeUnsSU2_DEPS)
	@echo "***"
	@echo "*** $@"
	@echo "***"
	@mkdir -p $(PLUGINS_DIST_DIR)
	$(SHLIB_LD) $(ARCH_FLAGS) $(CaeUnsSU2_LDFLAGS) -o $(CaeUnsSU2_FULLLIB) $(CaeUnsSU2_OBJ) $(CaeUnsSU2_LIBS) $(SYS_LIBS)

CaeUnsSU2_info:
	@echo ""
	@echo "--------------------------------------------------------------"
ifeq ($(machine),macosx)
	otool -L -arch all $(CaeUnsSU2_FULLLIB)
	@echo ""
endif
	file $(CaeUnsSU2_FULLLIB)
	@echo "--------------------------------------------------------------"
	@echo ""


########################################################################
# list of plugin's intermediate targets
#
$(CaeUnsSU2_OBJ_LOC):
	mkdir -p $(CaeUnsSU2_OBJ_LOC)

#.......................................................................
# build .d files for the plugin and each of the shared runtime sources
# the .d files will be placed in the plugins OBJ folder CaeUnsSU2_OBJ_LOC
$(CaeUnsSU2_OBJ_LOC)/%.d: $(CaeUnsSU2_CXX_LOC)/%.c
	@echo "Updating dependencies $(CaeUnsSU2_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	@./depend.sh $(CaeUnsSU2_OBJ_LOC) $(CaeUnsSU2_CXXFLAGS) $< > $@

$(CaeUnsSU2_OBJ_LOC)/%.d: $(PLUGINS_RT_PWPLOC)/%.c
	@echo "Updating dependencies $(CaeUnsSU2_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	@./depend.sh $(CaeUnsSU2_OBJ_LOC) $(CaeUnsSU2_CXXFLAGS) $< > $@

$(CaeUnsSU2_OBJ_LOC)/%.d: $(PLUGINS_RT_PWGMLOC)/%.c
	@echo "Updating dependencies $(CaeUnsSU2_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	@./depend.sh $(CaeUnsSU2_OBJ_LOC) $(CaeUnsSU2_CXXFLAGS) $< > $@

$(CaeUnsSU2_OBJ_LOC)/%.d: $(PLUGINS_RT_CAEPLOC)/%.c
	@echo "Updating dependencies $(CaeUnsSU2_OBJ_LOC)/$*.d"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	@./depend.sh $(CaeUnsSU2_OBJ_LOC) $(CaeUnsSU2_CXXFLAGS) $< > $@

#.......................................................................
# build .o files for the plugin and each of the shared runtime sources.
# the .o files will be placed in the plugins OBJ folder CaeUnsSU2_OBJ_LOC
$(CaeUnsSU2_OBJ_LOC)/%.o: $(CaeUnsSU2_CXX_LOC)/%.c
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	$(CXX) $(CaeUnsSU2_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<

$(CaeUnsSU2_OBJ_LOC)/%.o: $(PLUGINS_RT_PWPLOC)/%.c
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	$(CXX) $(CaeUnsSU2_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<

$(CaeUnsSU2_OBJ_LOC)/%.o: $(PLUGINS_RT_PWGMLOC)/%.c
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	$(CXX) $(CaeUnsSU2_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<

$(CaeUnsSU2_OBJ_LOC)/%.o: $(PLUGINS_RT_CAEPLOC)/%.c
	@echo "***"
	@echo "*** $*"
	@echo "***"
	@mkdir -p $(CaeUnsSU2_OBJ_LOC)
	$(CXX) $(CaeUnsSU2_CXXFLAGS) $(ARCH_FLAGS) -o $@ -c $<


########################################################################
# list of plugin's clean targets
#
CaeUnsSU2_cleandep:
	-$(RMR) $(CaeUnsSU2_OBJ_LOC)/*.d

CaeUnsSU2_clean:
	-$(RMR) $(CaeUnsSU2_OBJ_LOC)/*.{d,o}

CaeUnsSU2_distclean: CaeUnsSU2_clean
	-$(RMF) $(CaeUnsSU2_FULLLIB) > /dev/null 2>&1

########################################################################
# list of plugin's clean targets
#
CaeUnsSU2_install: install_validate CaeUnsSU2_installnow
	@echo "CaeUnsSU2 Installed to '$(PLUGIN_INSTALL_FULLPATH)'"

CaeUnsSU2_installnow:
	-@$(CP) $(CaeUnsSU2_FULLLIB) "$(PLUGIN_INSTALL_FULLPATH)/libCaeUnsSU2.$(SHLIB_SUFFIX)"

CaeUnsSU2_uninstall:
	@echo "CaeUnsSU2 Uninstalled from '$(PLUGIN_INSTALL_FULLPATH)'"
	-@$(RMF) "$(PLUGIN_INSTALL_FULLPATH)/libCaeUnsSU2.$(SHLIB_SUFFIX)"
