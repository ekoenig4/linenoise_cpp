CC=g++

HDR=interface/
SRC=src/
OBJ=.obj/
PRG=test/
EXE=bin/

HDRS=$(wildcard $(HDR)*.h)
SRCS=$(patsubst $(HDR)%, $(SRC)%, $(HDRS:.h=.cpp))
OBJS=$(patsubst $(HDR)%, $(OBJ)%, $(HDRS:.h=.obj))
PRGS=$(wildcard $(PRG)*.cpp)
EXES=$(patsubst $(PRG)%, $(EXE)%, $(PRGS:.cpp=))
PATS=$(patsubst $(EXE)%, %, $(EXES))

CFLAGS=-Wall -W -ggdb -std=c++11
INCLUDE=-l boost_regex -I $(HDR)

.PHONY: all test clean
.SECONDARY: $(OBJS) $(EXES)

all: $(EXES) 

$(OBJ)%.obj: $(SRC)%.cpp $(HDR)%.h 
	$(CC) $(ARGS) -c $< -o $@ $(INCLUDE)

$(EXE)%: $(PRG)%.cpp $(OBJS) 
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(INCLUDE)

%: $(EXE)%
	@echo > /dev/null

clean:
	rm -f $(EXE)*
	rm -f $(OBJ)*

test:
	@echo "..............................."
	@echo "HDRS          = $(HDRS)"
	@echo "....."
	@echo "SRCS          = $(SRCS)"
	@echo "....."
	@echo "OBJS          = $(OBJS)"
	@echo "....."
	@echo "PRGS          = $(PRGS)"
	@echo "....."
	@echo "EXES          = $(EXES)"
	@echo "....."
	@echo "PATS          = $(PATS)"
	@echo "..............................."
