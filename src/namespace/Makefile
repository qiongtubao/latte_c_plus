WORKSPACE?=$(CURDIR)/../..
BUILD_OBJ?=latte.xo
LIB_OBJ?=
#../zmalloc/zmalloc.o
TEST_MAIN?=latte_test

include $(WORKSPACE)/mks/sys.mk
include $(WORKSPACE)/mks/final.mk
include $(WORKSPACE)/mks/malloc.mk
include $(WORKSPACE)/mks/latte.mk
include $(WORKSPACE)/mks/info.mk

FINAL_CXX_CFLAGS += -std=c++2a -fcoroutines
