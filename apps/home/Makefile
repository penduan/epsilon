app_home_src = $(addprefix apps/home/,\
  app.cpp \
  app_cell.cpp \
  controller.cpp \
)

apps_src += $(app_home_src)

i18n_files += $(call i18n_without_universal_for,home/i18n/base)

# Apps layout file generation

# The header is refered to as <apps/home/apps_layout.h> so make sure it's
# findable this way
SFLAGS += -I$(BUILD_DIR)

apps_layout = apps/home/apps_layout.csv

$(eval $(call rule_for, \
  APPSLYT, \
  apps/home/apps_layout.h, \
  apps/home/apps_layout.py $(apps_layout), \
  $$(PYTHON) apps/home/apps_layout.py --layouts $(apps_layout) --header $$@ --implementation $$(subst .h,.cpp,$$@) --apps $$(EPSILON_APPS), \
  global \
))

$(BUILD_DIR)/apps/home/apps_layout.cpp: $(BUILD_DIR)/apps/home/apps_layout.h
apps_src += $(BUILD_DIR)/apps/home/apps_layout.cpp

# This kind of dependency between a .cpp and a .h is usually created by the
# compiler for recompilations to take header changes into account.
# However we have to specify it explicitely here because we need the header to
# be generated before the .cpp even on the first build.
$(call object_for, apps/home/controller.cpp): $(BUILD_DIR)/apps/home/apps_layout.h
