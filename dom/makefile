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

OUT_DIR := ..

OBJFILES := $(addprefix $(OUT_DIR)/,$(patsubst %.cpp,%.o,$(SRCFILES)))

TARGET_NAME := libsssdom

TARGET  := $(OUT_DIR)/$(TARGET_NAME).a

# MingW32 tool Chain setting
CC		 := gcc
CXX		 := g++
CXXFLAGS := -Wall -O2
RC		 := windres -O COFF
ARFLAGS  := rus

CXXFLAGS += -W -fexceptions -O2 -DNDEBUG -D_LIB -D_MBCS -DWIN32

# Dos shell tool Setting
RM		:= rm -f
MKDIR	:= mkdir -p

.PHONY: all clean

all: $(TARGET)

# mkdir -p 支持递归创建文件夹
# @echo $(MKDIR) `dirname $@`
$(OUT_DIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS)

$(TARGET): $(OBJFILES)
	$(AR) $(ARFLAGS) $@ $?

clean:
	-$(RM) $(TARGET) $(OBJFILES)
