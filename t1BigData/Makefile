CXXFLAGS =	-O2 -g -std=c++11 -Wall -fmessage-length=0

OBJS =		t1BigData.o pugixml.o

LIBS =

TARGET =	t1BigData

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.csv
