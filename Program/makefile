all : split split_correctness

CCC = g++
CCFLAGS = -O3 -std=c++11
LIBS= -lm
TARGETDIR=.
CPPFLAGS += \
         -I.

        
OBJS2 = \
        $(TARGETDIR)/commandline.o \
	    $(TARGETDIR)/main.o \
        $(TARGETDIR)/Pb_Data.o \
	$(TARGETDIR)/Split_Bellman_Soft_CVRP.o \
        $(TARGETDIR)/Split_Bellman_VRPSPDTW.o \
        $(TARGETDIR)/Split_Bellman_Soft_VRPSPD.o \
        $(TARGETDIR)/Split_Bellman_Soft_VRPTW.o \
	$(TARGETDIR)/Split_Bellman_Soft_VRPSPDTW.o \
	$(TARGETDIR)/Split_Linear.o \
	$(TARGETDIR)/Split_Linear_Soft_CVRP.o \
        $(TARGETDIR)/Split_Linear_VRPSPDTW.o \
		$(TARGETDIR)/Split_Linear_VRPSPD.o \
	$(TARGETDIR)/Split_Linear_VRPTW.o \
        $(TARGETDIR)/Split_Linear_Soft_VRPSPD.o \
	$(TARGETDIR)/Split_Linear_Soft_VRPSPDTW.o \
	$(TARGETDIR)/Split_Linear_Soft_VRPTW.o \
	$(TARGETDIR)/Split_Linear_CVRP.o

CORRECTNESS_OBJS = \
	$(TARGETDIR)/main_correctness.o \
	$(TARGETDIR)/BenchmarkInstanceReader.o \
	$(TARGETDIR)/Pb_Data.o \
	$(TARGETDIR)/Split_Bellman_Soft_CVRP.o \
	$(TARGETDIR)/Split_Bellman_VRPSPDTW.o \
	$(TARGETDIR)/Split_Bellman_Soft_VRPSPD.o \
	$(TARGETDIR)/Split_Bellman_Soft_VRPTW.o \
	$(TARGETDIR)/Split_Bellman_Soft_VRPSPDTW.o \
	$(TARGETDIR)/Split_Linear_CVRP.o \
	$(TARGETDIR)/Split_Linear_Soft_CVRP.o \
	$(TARGETDIR)/Split_Linear_VRPSPD.o \
	$(TARGETDIR)/Split_Linear_VRPTW.o \
	$(TARGETDIR)/Split_Linear_VRPSPDTW.o \
	$(TARGETDIR)/Split_Linear_Soft_VRPSPD.o \
	$(TARGETDIR)/Split_Linear_Soft_VRPSPDTW.o \
	$(TARGETDIR)/Split_Linear_Soft_VRPTW.o

$(TARGETDIR)/split: $(OBJS2)
	$(CCC)  $(CCFLAGS) -o $(TARGETDIR)/split $(OBJS2)

$(TARGETDIR)/split_correctness: $(CORRECTNESS_OBJS)
	$(CCC) $(CCFLAGS) -o $(TARGETDIR)/split_correctness $(CORRECTNESS_OBJS)

$(TARGETDIR)/commandline.o: commandline.h commandline.cpp Pb_Data.h
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c commandline.cpp -o $(TARGETDIR)/commandline.o
	
$(TARGETDIR)/main.o: main.cpp commandline.h Pb_Data.h Split.h
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c main.cpp -o $(TARGETDIR)/main.o

$(TARGETDIR)/main_correctness.o: main_correctness.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c main_correctness.cpp -o $(TARGETDIR)/main_correctness.o

$(TARGETDIR)/BenchmarkInstanceReader.o: ../Benchmark-Instances/BenchmarkInstanceReader.h ../Benchmark-Instances/BenchmarkInstanceReader.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c ../Benchmark-Instances/BenchmarkInstanceReader.cpp -o $(TARGETDIR)/BenchmarkInstanceReader.o

$(TARGETDIR)/Pb_Data.o: Pb_Data.h Pb_Data.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Pb_Data.cpp -o $(TARGETDIR)/Pb_Data.o

$(TARGETDIR)/Split_Bellman.o: Split_Bellman.h Split_Bellman.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Bellman.cpp -o $(TARGETDIR)/Split_Bellman.o


$(TARGETDIR)/Split_Bellman_Soft_CVRP.o: Split_Bellman_Soft_CVRP.h Split_Bellman_Soft_CVRP.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Bellman_Soft_CVRP.cpp -o $(TARGETDIR)/Split_Bellman_Soft_CVRP.o

$(TARGETDIR)/Split_Linear.o: Split_Linear.h Split_Linear.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear.cpp -o $(TARGETDIR)/Split_Linear.o


$(TARGETDIR)/Split_Linear_Soft_CVRP.o: Split_Linear_Soft_CVRP.h Split_Linear_Soft_CVRP.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_Soft_CVRP.cpp -o $(TARGETDIR)/Split_Linear_Soft_CVRP.o

$(TARGETDIR)/Split_Linear_VRPSPDTW.o: Split_Linear_VRPSPDTW.h Split_Linear_VRPSPDTW.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_VRPSPDTW.cpp -o $(TARGETDIR)/Split_Linear_VRPSPDTW.o

$(TARGETDIR)/Split_Linear_VRPSPD.o: Split_Linear_VRPSPD.h Split_Linear_VRPSPD.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_VRPSPD.cpp -o $(TARGETDIR)/Split_Linear_VRPSPD.o

$(TARGETDIR)/Split_Linear_VRPTW.o: Split_Linear_VRPTW.h Split_Linear_VRPTW.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_VRPTW.cpp -o $(TARGETDIR)/Split_Linear_VRPTW.o

$(TARGETDIR)/Split_Linear_CVRP.o: Split_Linear_CVRP.h Split_Linear_CVRP.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_CVRP.cpp -o $(TARGETDIR)/Split_Linear_CVRP.o

$(TARGETDIR)/Split_Bellman_VRPSPDTW.o: Split_Bellman_VRPSPDTW.h Split_Bellman_VRPSPDTW.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Bellman_VRPSPDTW.cpp -o $(TARGETDIR)/Split_Bellman_VRPSPDTW.o

$(TARGETDIR)/Split_Linear_Soft_VRPSPD.o: Split_Linear_Soft_VRPSPD.h Split_Linear_Soft_VRPSPD.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_Soft_VRPSPD.cpp -o $(TARGETDIR)/Split_Linear_Soft_VRPSPD.o

$(TARGETDIR)/Split_Linear_Soft_VRPTW.o: Split_Linear_Soft_VRPTW.h Split_Linear_Soft_VRPTW.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_Soft_VRPTW.cpp -o $(TARGETDIR)/Split_Linear_Soft_VRPTW.o

$(TARGETDIR)/Split_Linear_Soft_VRPSPDTW.o: Split_Linear_Soft_VRPSPDTW.h Split_Linear_Soft_VRPSPDTW.cpp Outer_Deque_VRPSPDTW.h Inner_Deque_VRPSPDTW.h Trivial_Deque.h
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Linear_Soft_VRPSPDTW.cpp -o $(TARGETDIR)/Split_Linear_Soft_VRPSPDTW.o

$(TARGETDIR)/Split_Bellman_Soft_VRPTW.o: Split_Bellman_Soft_VRPTW.h Split_Bellman_Soft_VRPTW.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Bellman_Soft_VRPTW.cpp -o $(TARGETDIR)/Split_Bellman_Soft_VRPTW.o

$(TARGETDIR)/Split_Bellman_Soft_VRPSPDTW.o: Split_Bellman_Soft_VRPSPDTW.h Split_Bellman_Soft_VRPSPDTW.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Bellman_Soft_VRPSPDTW.cpp -o $(TARGETDIR)/Split_Bellman_Soft_VRPSPDTW.o

$(TARGETDIR)/Split_Bellman_Soft_VRPSPD.o: Split_Bellman_Soft_VRPSPD.h Split_Bellman_Soft_VRPSPD.cpp
	$(CCC) $(CCFLAGS) $(CPPFLAGS) -c Split_Bellman_Soft_VRPSPD.cpp -o $(TARGETDIR)/Split_Bellman_Soft_VRPSPD.o
clean:
	$(RM) $(TARGETDIR)/split $(TARGETDIR)/split_correctness $(sort $(OBJS2) $(CORRECTNESS_OBJS))
