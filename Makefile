CC=g++ -std=c++11

HDR=interface/
SRC=src/
OBJ=.obj/
PRG=test/
EXE=bin/

HDRS=$(wildcard $(HDR)*.h)
SRCS=$(wildcard $(SRC)*.cpp)
OBJS=$(patsubst $(SRC)%, $(OBJ)%, $(SRCS:.cpp=.obj))
PRGS=$(wildcard $(PRG)*.cpp)
EXES=$(patsubst $(PRG)%, $(EXE)%, $(PRGS:.cpp=))
PATS=$(patsubst $(EXE)%, %, $(EXES))

CFLAGS=-Wall -W -ggdb
INCLUDE=-l boost_regex -I $(HDR)

.PHONY: all test clean
.SECONDARY: $(OBJS) $(EXES)

all: $(EXES) 

$(OBJ)%.obj: $(SRC)%.cpp $(HDR)%.h 
	@mkdir -p $(@D)
	$(CC) $(ARGS) -c $< -o $@ $(INCLUDE)

$(EXE)%: $(PRG)%.cpp $(OBJS) 
	@mkdir -p $(@D)
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
