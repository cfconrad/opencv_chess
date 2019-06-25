OPENCV=`pkg-config --libs --cflags opencv`

TARGETS=hough_line_transform.cpp \
	canny_edge_detector.cpp \
	blur.cpp \
	video.cpp

all: $(TARGETS:%.cpp=%)

canny_edge_detector: canny_edge_detector.cpp
	c++ -o $@ $< $(OPENCV)

hough_line_transform: hough_line_transform.cpp
	c++ -o $@ $< $(OPENCV)

blur: blur.cpp
	c++ -o $@ $< $(OPENCV)

video: video.cpp
	c++ -o $@ $< $(OPENCV)
