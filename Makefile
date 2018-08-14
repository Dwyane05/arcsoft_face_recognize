CXX = g++
CC  = gcc

CFLAGS += -O2 -c -Wall -std=c++11 -fPIC -m64 -ffunction-sections
#LDFLAGS += -lrt

CFLAGS += -I${PWD}/include
#LDFLAGS += -larcsoft_fsdk_age_estimation -L${PWD}/lib/linux_x64
LDFLAGS += -larcsoft_fsdk_face_detection

src := src/arcsoft_face.cpp 

objects=$(patsubst %.cpp,%.o, $(src))
target = face_detection

.PHONY:all
all:${target}

${target}:$(objects)
	$(Q)$(CXX) $^ ${LDFLAGS} -o $@
	
%.o:%.cpp
	$(Q)$(CXX) -c $^ ${CFLAGS} ${CFLAGS_FULL} -o $@

.PHONY:clean
clean:
	@rm -f  ${target}
	@find -name "*.o" -execdir rm {} \;

	

