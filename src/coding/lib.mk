ifneq ($(USED_CODING), yes) 
FINAL_CC_CFLAGS+= -I../coding
LIB_OBJ+= ../coding/coding.xo
USED_CODING=yes
../coding/coding.xo:
	cd ../coding && make coding.xo
	

endif