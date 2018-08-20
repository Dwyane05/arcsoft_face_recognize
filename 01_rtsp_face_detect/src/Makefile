CXX = g++
CC  = gcc

CFLAGS := -O2 -c -Wall -std=c++11 -fPIC -m64 -ffunction-sections
CFLAGS += -I${PWD}/../include
CFLAGS +=  -DDEBUG

#LDFLAGS += -larcsoft_fsdk_age_estimation -L${PWD}/lib/linux_x64
LDFLAGS += -larcsoft_fsdk_face_detection
LDFLAGS += -lrt -lpthread -ldl $(shell pkg-config --cflags --libs opencv) 
#-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio \
-lopencv_imgproc -lopencv_xfeatures2d -lopencv_features2d -lopencv_flann -lopencv_calib3d -lopencv_ml

# for optimizing, close it for debug
#CFLAGS += -O 
#CFLAGS += -Os
#CFLAGS += -std=gnu++11
#CFLAGS += -std=c++11

src := rtsp_face_detect.cpp 

src += facedetect/facedetect.cpp 
CFLAGS += -I${PWD}/facedetect

src += misc/error.cpp misc/sleepus.cpp misc/sleep.cpp

#src += json/json_reader.cpp json/json_writer.cpp json/json_value.cpp 
#CFLAGS += -I${PWD}/json

objects=$(patsubst %.cpp,%.o, $(src))
target = rtsp_face_detection

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

	

