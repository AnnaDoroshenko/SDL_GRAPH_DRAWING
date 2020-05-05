# The name of the main file and executable
mainFileName = main
# Compilation flags
OPTIMIZATION_FLAG = -O0
LANGUAGE_LEVEL = -std=c++17
COMPILER_FLAGS = -Wall -Wextra -Wno-unused-parameter


# Auxiliary
filesObj = $(addsuffix .o, $(mainFileName) $(classFiles))


all: cleanExe $(mainFileName)


# Compiler
%.o: %.cpp $(filesH)
	g++ $(COMPILER_FLAGS) $(OPTIMIZATION_FLAG) $(LANGUAGE_LEVEL) -c $<


# Linker
$(mainFileName): $(filesObj)
	g++ $(COMPILER_FLAGS) $(OPTIMIZATION_FLAG) $(LANGUAGE_LEVEL) $^ -o $@ -lSDL2 -lSDL2_ttf


# Utils
clean:
	rm -f *.o $(mainllFileName) *.gch .*.gch:

cleanExe:
	rm -f $(mainFileName)
