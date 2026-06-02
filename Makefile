# Especifica o compilador
CXX = g++

# Flags de compilação: C++17, Otimização Máxima (-O3) e Avisos do Compilador (-Wall)
CXXFLAGS = -std=c++17 -O3 -Wall

# Nome do executável final
TARGET = puzzle_solver

# Arquivos fonte
SRC = main.cpp

# Arquivos de cabeçalho (dependências do projeto)
DEPS = State.hpp Heuristics.hpp AStar.hpp

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
	@./$(TARGET)

# Regra para limpar os arquivos binários gerados
clean:
	rm -f $(TARGET)