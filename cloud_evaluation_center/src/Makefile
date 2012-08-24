#****************************************************************************
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := NO

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

#****************************************************************************
# environment variables
#****************************************************************************

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib


#****************************************************************************
# debug flags
#****************************************************************************
DEBUG_CFLAGS     := -g -Wall -Wno-format
DEBUG_CFLAGS += -DDEBUG #调试
DEBUG_CFLAGS += -DOPEN_LOGDEBUG
#DEBUG_CFLAGS += -DOPEN_LOG_TO_TERMINAL

RELEASE_CFLAGS   := -m32 -Wall -Wno-unknown-pragmas -Wno-format -O2

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    :=
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg
   CXXFLAGS := ${CXXFLAGS} -pg
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************


#****************************************************************************
# Library paths 
#****************************************************************************
LIBS := -L/usr/local/lib \
		-L/opt/gnome/lib \
		-L/usr/local/mysql/lib/mysql \
		-lpthread -ldl -lrt \
		-levent\
		-ljson\
		-lz\
		-lmysqlclient\
		#/usr/local/mysql/lib/mysql/libmysqlclient.a
		#-lmysqlclient

#****************************************************************************
# Include paths
#****************************************************************************
INCS := -I../\
		-I../include\
		-I/usr/local/include/json/\
		-I/opt/gnome/include\
		-I/usr/local/mysql/include\

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := cloud_evaluation_center

all: ${OUTPUT}

#****************************************************************************
# Source files
#****************************************************************************

SRCS := $(wildcard *.cpp)

#OBJS := $(addsuffix .o,$(basename ${SRCS}))
OBJS := $(patsubst %.cpp,%.o,$(SRCS))


#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS} ${MODULES}
			${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS} ${MODULES}

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} ${OUTPUT}
#	make -C $(HTTPFETCHER_PATH) clean

depend:
	#makedepend ${INCS} ${SRCS}

