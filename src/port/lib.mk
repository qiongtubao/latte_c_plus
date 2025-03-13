ifneq ($(USED_PORT), yes) 
FINAL_CC_CFLAGS+= -I../port
LIB_OBJ+= ../port/sys_time.xo
USED_PORT=yes
../port/sys_time.xo:
	cd ../port && make sys_time.xo
	

endif