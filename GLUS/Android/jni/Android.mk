# Comment out the version used.

GLES_VERSION := 20
#GLES_VERSION := 30
#GLES_VERSION := 31

LOCAL_PATH := $(call my-dir)

#
# GLUS.
#
 
include $(CLEAR_VARS)

LOCAL_MODULE    := GLUS

# All files.

PROJECT_FILES := $(wildcard $(LOCAL_PATH)/../../src/*.c)

# Remove specific files.

GL_FILES := $(LOCAL_PATH)/../../src/glus_window_glfw.c
GL_FILES += $(LOCAL_PATH)/../../src/glus_programpipeline.c
GL_FILES += $(LOCAL_PATH)/../../src/glus_program.c
GL_FILES += $(LOCAL_PATH)/../../src/glus_shape_adjacency.c

NOT_USED_FILES := $(LOCAL_PATH)/../../src/glus_memory_nodm.c

OS_FILES := $(wildcard $(LOCAL_PATH)/../../src/glus_os_*.c)

VG_FILES := $(wildcard $(LOCAL_PATH)/../../src/*_vg.c)

ES_FILES := $(wildcard $(LOCAL_PATH)/../../src/*_es.c)

ES2_FILES := $(wildcard $(LOCAL_PATH)/../../src/*_es2.c)

ES31_FILES := $(wildcard $(LOCAL_PATH)/../../src/*_es31.c)

PROJECT_FILES := $(filter-out $(GL_FILES), $(PROJECT_FILES))
PROJECT_FILES := $(filter-out $(OS_FILES), $(PROJECT_FILES))
PROJECT_FILES := $(filter-out $(NOT_USED_FILES), $(PROJECT_FILES))
PROJECT_FILES := $(filter-out $(VG_FILES), $(PROJECT_FILES))
ifeq ($(GLES_VERSION),20)
	PROJECT_FILES := $(filter-out $(ES31_FILES), $(PROJECT_FILES))

	LOCAL_CFLAGS    := -DGLUS_ES2=1
endif
ifeq ($(GLES_VERSION),30)
	PROJECT_FILES := $(filter-out $(ES2_FILES), $(PROJECT_FILES))
	PROJECT_FILES := $(filter-out $(ES31_FILES), $(PROJECT_FILES))

	LOCAL_CFLAGS    := -DGLUS_ES3=1
endif
ifeq ($(GLES_VERSION),31)
	PROJECT_FILES := $(filter-out $(ES2_FILES), $(PROJECT_FILES))
	
	LOCAL_CFLAGS    := -DGLUS_ES31=1
endif

# Change base path in general.

LOCAL_CFLAGS    += -DGLUS_BASE_DIRECTORY=\"/sdcard/Download/Binaries/\"

# Add specfic files.

PROJECT_FILES += $(LOCAL_PATH)/../../src/glus_os_android_es.c

# Generate the final list.

PROJECT_FILES := $(PROJECT_FILES:$(LOCAL_PATH)/%=%)
 
#

LOCAL_SRC_FILES := $(PROJECT_FILES)
 
LOCAL_STATIC_LIBRARIES := android_native_app_glue
 
include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/native_app_glue)
