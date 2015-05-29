CXXFLAGS =	-O2 -g -std=c++11 -Wall -fmessage-length=0

OBJS =		t2BigData.o pugixml.o

LIBS =

TARGET =	t2BigData

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.csv
