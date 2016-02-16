define walk
$(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

ALLFILES := $(patsubst ./%,%,$(filter-out .,$(call walk, .)))

# 剔除测试程序
ALLFILES := $(filter-out %/test/%,$(ALLFILES))

SRCFILES := $(filter %.cpp,$(ALLFILES))
HEAFILES := $(filter %.hpp,$(ALLFILES))

#dirs := $(shell find . -maxdepth 1 -type d)
dirs := $(shell find . -type d)
dirs := $(basename $(patsubst ./%,%,$(dirs)))
dirs := $(filter-out $(exclude_dirs),$(dirs))

OUT_DIR := ../../lib

OBJFILES := $(addprefix $(OUT_DIR)/,$(patsubst %.cpp,%.o,$(SRCFILES)))

TARGET_NAME := libsss

TARGET  := $(OUT_DIR)/$(TARGET_NAME).a

# MingW32 tool Chain setting
CC		 := gcc
CXX		 := g++
CXXFLAGS := -Wall -O2
RC		 := windres -O COFF
ARFLAGS  := rus

CFLAGS   += -W -fexceptions -O2 -DNDEBUG -D_LIB -D_MBCS -DWIN32

# Dos shell tool Setting
RM		:= rm -f
MKDIR	:= mkdir

.PHONY: all clean

all: $(TARGET)
#	echo
#	$(AR) $(ARFLAGS) $@ $?

$(OUT_DIR)/%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS)

$(TARGET): $(OBJFILES)
	$(AR) $(ARFLAGS) $@ $?

clean:
	-$(RM) $(TARGET) $(OBJFILES)
