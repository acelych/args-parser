NVCC        = g++
OPENCV_INC  += /usr/include/opencv4    # Specify the OpenCV include path
#OPENCV_INC  += $(shell pkg-config --cflags opencv4)
OPENCV_LIBS = -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching  -lopencv_video    # Specify the necessary OpenCV libraries
#OPENCV_LIBS += $(shell pkg-config --libs opencv4)

ifeq (,$(shell which nvprof))
NVCC_FLAGS  = -O2
else
NVCC_FLAGS  = -O2 --std=c++11
endif
LD_FLAGS    = $(OPENCV_LIBS)
EXE         = jpeg
OBJ         = main.cpp match.cpp sar_sift.cpp

default: $(EXE)

main.o: ./main.cpp ./match.cpp ./sar_sift.cpp
	$(NVCC) -c -o $@ ./main.cpp $(NVCC_FLAGS) $(OPENCV_INC)
	$(NVCC) -c -o $@ ./match.cpp $(NVCC_FLAGS) $(OPENCV_INC)
	$(NVCC) -c -o $@ ./sar_sift.cpp $(NVCC_FLAGS) $(OPENCV_INC)

$(EXE): $(OBJ)
	$(NVCC) $(OBJ) -o $(EXE) $(LD_FLAGS)

clean:
	rm -rf *.o $(EXE)
