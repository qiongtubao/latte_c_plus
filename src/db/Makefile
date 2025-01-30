WORKSPACE?=$(CURDIR)/../..
BUILD_OBJ?=db.xo 
LIB_OBJ?=
#../zmalloc/zmalloc.o
TEST_MAIN?=db_test
BUILD_INCLUDE=db.h leveldb.h rocksdb.h
LIB_MODULES?= 

include $(WORKSPACE)/mks/sys.mk
include $(WORKSPACE)/mks/final.mk
include $(WORKSPACE)/mks/malloc.mk
include $(WORKSPACE)/mks/latte.mk
include $(WORKSPACE)/mks/info.mk

options/%.xo: options/%.c
	$(CC) -c $< -o $@


