# Especifica o compilador
CXX = g++

# Flags de compilação: C++17, Otimização Máxima e Suporte a Threads
CXXFLAGS = -std=c++17 -O3 -Wall -pthread

# Nome do executável final
TARGET = puzzle_solver

BIN = *.bin

# Arquivos fonte
SRC = main.cpp

# Arquivos de cabeçalho (dependências do projeto)
DEPS = State.hpp \
       algorithms/IAlgorithm.hpp \
       algorithms/AStar.hpp \
       algorithms/IDAStar.hpp \
       algorithms/ParallelIDAStar.hpp \
       heuristics/IHeuristic.hpp \
       heuristics/Heuristics_1.hpp \
       heuristics/Heuristics_2.hpp \
       heuristics/Heuristics_3.hpp \
       heuristics/Heuristics_4.hpp \
       heuristics/PatternDatabase.hpp

# Declara 'all', 'clean' e 'run' como alvos falsos (não são arquivos físicos)
.PHONY: all clean run

# Regra padrão: apenas compila
all: $(TARGET)

# Compila o executável principal
$(TARGET): $(SRC) $(DEPS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# REGRA NOVA: Executa o programa compilado
# O sinal '@' esconde o comando do terminal, deixando a saída limpa
run: $(TARGET)
	@./$(TARGET) $(ARGS)

# Regra para limpar os arquivos binários gerados
clean:
	rm -f $(TARGET) $(BIN)