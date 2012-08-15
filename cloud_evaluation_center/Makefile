#****************************************************************************
# This is a GNU make (gmake) makefile
#****************************************************************************

OutProgram := cloud_evaluation_center

SRC_PATH := ./src/

all:
	make -C  ${SRC_PATH}
	-cp "${SRC_PATH}/${OutProgram}" .

install:
	-cp "${SRC_PATH}/${OutProgram}" .

uninstall:
	-rm -f ${OutProgram}

clean:
	make -C ${SRC_PATH} clean
	-rm -f ${OutProgram}


