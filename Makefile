# Especifica o compilador
CXX = g++

# Flags de compilação: C++17, Otimização Máxima e Suporte a Threads
CXXFLAGS = -std=c++17 -O3 -Wall -pthread

# Nome do executável final
TARGET = puzzle_solver

# Arquivo fonte principal
SRC = main.cpp

# Declara os alvos que não geram arquivos
.PHONY: all clean run

# Regra padrão: compila o executável
all: $(TARGET)

# Compilação limpa sem precisar listar as dependências manualmente
# (O GCC resolve os #includes automaticamente)
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Regra auxiliar para rodar via Make (usando a variável ARGS opcional)
run: $(TARGET)
	@./$(TARGET) $(ARGS)

# Limpeza dos binários gerados
clean:
	rm -f $(TARGET) *.bin