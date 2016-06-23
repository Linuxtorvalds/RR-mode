
OBJS = rrmode.o master.o worker.o

LIBS = -levent

TARGET = rrmode

$(TARGET):	$(OBJS)
	$(CXX) -g -o  $(TARGET)  $(OBJS) $(LIBS) 

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
