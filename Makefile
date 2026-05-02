# ============================================================
# Makefile — DSL Compiler
# Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
LEX     = flex
YACC    = bison
YFLAGS  = -d -v

SRC     = src
OBJ_DIR = obj
OUT_DIR = output

SRCS    = $(OBJ_DIR)/lexer.c  \
          $(OBJ_DIR)/parser.tab.c \
          $(SRC)/ast.c        \
          $(SRC)/symtable.c   \
          $(SRC)/semantic.c   \
          $(SRC)/codegen.c    \
          $(SRC)/main.c

OBJS    = $(patsubst %.c,%.o,$(SRCS))

TARGET  = dslc

.PHONY: all clean run test dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(OBJ_DIR) $(OUT_DIR)

# ---- Lex → C ----
$(OBJ_DIR)/lexer.c: $(SRC)/lexer.l
	$(LEX) -o $@ $<

# ---- Bison → C + header ----
$(OBJ_DIR)/parser.tab.c $(OBJ_DIR)/parser.tab.h: $(SRC)/parser.y
	$(YACC) $(YFLAGS) -o $(OBJ_DIR)/parser.tab.c $<

# ---- Compile lexer with Bison header ----
$(OBJ_DIR)/lexer.o: $(OBJ_DIR)/lexer.c $(OBJ_DIR)/parser.tab.h
	$(CC) $(CFLAGS) -I$(OBJ_DIR) -c $< -o $@

$(OBJ_DIR)/parser.tab.o: $(OBJ_DIR)/parser.tab.c
	$(CC) $(CFLAGS) -I$(OBJ_DIR) -c $< -o $@

$(SRC)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -I$(OBJ_DIR) -c $< -o $@

$(TARGET): $(OBJ_DIR)/lexer.o $(OBJ_DIR)/parser.tab.o \
           $(SRC)/ast.o $(SRC)/symtable.o $(SRC)/semantic.o \
           $(SRC)/codegen.o $(SRC)/main.o
	$(CC) $(CFLAGS) $^ -o $@
	@echo "Build complete → ./$(TARGET)"

run: all
	./$(TARGET) test/sample1.dsl --ast --sym

test: all
	@echo "Running all tests..."
	@for f in test/*.dsl; do \
	    echo "  Testing $$f ..."; \
	    ./$(TARGET) $$f -o $(OUT_DIR)/$$(basename $$f .dsl).ll && echo "  PASS" || echo "  FAIL"; \
	done

clean:
	rm -f $(OBJ_DIR)/*.c $(OBJ_DIR)/*.h $(OBJ_DIR)/*.o
	rm -f $(SRC)/*.o
	rm -f $(TARGET)
	rm -f $(OUT_DIR)/*.ll $(OUT_DIR)/*.s
