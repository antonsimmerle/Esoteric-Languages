#include <stdio.h>
#include <stdlib.h>

typedef enum KIND KIND;
typedef struct OPERATOR OPERATOR;
typedef struct OPERATORS OPERATORS;

typedef struct NODE NODE;

typedef struct STACK STACK;
typedef struct TAPE TAPE;

char *readentirefile(FILE *input);

void push(STACK *stack, size_t value);
size_t pop(STACK *stack);
OPERATORS *parser(const char *input);

void runtime(OPERATORS *input);

char *cleanupfile(const char *input);

int main(int argc, char **argv);

char *readentirefile(FILE *input) {
    fseek(input, 0, SEEK_END);
    size_t size = ftell(input);
    rewind(input);
    char *content = malloc(sizeof(char) * (size + 1));
    if (!content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    fread(content, 1, size, input);
    content[size] = '\0';
    return content;
}

#define CHUNK 10

enum KIND {
    LEFT          = '<',
    RIGHT         = '>',
    INCREMENT     = '+',
    DECREMENT     = '-',
    OUTPUT        = '.',
    INPUT         = ',',
    JUMPIFZERO    = '[',
    JUMPIFNOTZERO = ']',
};
struct OPERATOR {
    size_t address;
    KIND kind;
    size_t property;
};
struct OPERATORS {
    OPERATOR *items;
    size_t count;
    size_t capacity;
};

struct NODE {
    size_t value;
    NODE *next; 
};
struct STACK {
    NODE *items;
    size_t count;
};
void push(STACK *stack, size_t value) {
    stack->count++;
    NODE *node = malloc(sizeof(NODE));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->value = value;
    node->next = stack->items;
    stack->items = node;
}
size_t pop(STACK *stack) {
    if (stack->count == 0) {
        fprintf(stderr, "Error: Pop from empty stack\n");
        exit(EXIT_FAILURE);
    }
    stack->count--;
    NODE *node = stack->items;
    size_t value = node->value;
    if (node) {
        stack->items = node->next;
    }
    free(node);
    return value;
}
OPERATORS *parser(const char *input) {
    OPERATORS *operators = calloc(1, sizeof(OPERATORS));
    if (!operators) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    STACK stack = { .count = 0, .items = NULL, };
    for (size_t i = 0, j = 0; input[i] != '\0'; i++, j++) {
        operators->count++;
        if (j >= operators->capacity) {
            operators->capacity += CHUNK;
            operators->items = realloc(operators->items, sizeof(OPERATOR) * operators->capacity);
        }
        operators->items[j].address = j;
        operators->items[j].kind = input[i];
        switch (operators->items[j].kind) {
            case INCREMENT:
            case DECREMENT:
            case LEFT:
            case RIGHT:
            case INPUT:
            case OUTPUT: {
                size_t count = 1;
                for (; input[i + 1] == (char)operators->items[j].kind; i++, count++);
                operators->items[j].property = count;
            } break;
            case JUMPIFZERO: {
                push(&stack, operators->items[j].address);
            } break;
            case JUMPIFNOTZERO: {
                if (stack.count == 0) {
                    fprintf(stderr, "Error: Unmatched ']'\n");
                    exit(EXIT_FAILURE);
                }
                size_t address = pop(&stack);
                operators->items[address].property = operators->count;
                operators->items[j].property = address + 1;
            } break;
        }
    }
    free(stack.items);
    return operators;
}

struct TAPE {
    char *items;
    size_t capacity;
};
void runtime(OPERATORS *input) {
    TAPE tape = { .capacity = CHUNK, .items = calloc(tape.capacity, sizeof(char)), };
    if (!tape.items) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    size_t i = 0;
    size_t pointer = 0;
    while (i < input->count) {
        switch (input->items[i].kind) {
            case INCREMENT: {
                tape.items[pointer] += input->items[i].property;
                i++;
            } break;
            case DECREMENT: {
                tape.items[pointer] -= input->items[i].property;
                i++;
            } break;
            case LEFT: {
                if (pointer < input->items[i].property) {
                    fprintf(stderr, "Error: Memory underflow\n");
                    exit(EXIT_FAILURE);
                }
                pointer -= input->items[i].property;
                i++;
            } break;
            case RIGHT: {
                pointer += input->items[i].property;
                while (pointer >= tape.capacity) {
                    tape.capacity += CHUNK;
                    tape.items = realloc(tape.items, sizeof(char) * tape.capacity);
                    if (!tape.items) {
                        fprintf(stderr, "Error: Memory allocation failed\n");
                        exit(EXIT_FAILURE);
                    }
                    for (size_t j = tape.capacity - CHUNK; j < tape.capacity; j++) {
                        tape.items[j] = 0;
                    }
                }
                i++;
            } break;
            case INPUT: {
                for (size_t j = 0; j < input->items[i].property; j++) {
                    scanf("%c", &tape.items[pointer]);
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                }
                i++;
            } break;
            case OUTPUT: {
                for (size_t j = 0; j < input->items[i].property; j++) {
                    printf("%c", tape.items[pointer]);
                }
                i++;
            } break;
            case JUMPIFZERO: {
                if (tape.items[pointer] == 0) {
                    i = input->items[i].property;
                } else {
                    i++;
                }
            } break;
            case JUMPIFNOTZERO: {
                if (tape.items[pointer] != 0) {
                    i = input->items[i].property;
                } else {
                    i++;
                }
            } break;
        }
    }
    free(tape.items);
}

char *cleanupfile(const char *input) {
    size_t length = 0;
    for (; input[length] != '\0'; length++);
    char *output = malloc(sizeof(char) * length);
    if (!output) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case INCREMENT:
            case DECREMENT:
            case LEFT:
            case RIGHT:
            case INPUT:
            case OUTPUT:
            case JUMPIFZERO:
            case JUMPIFNOTZERO: {
                output[j] = input[i];
                j++;
            } break;
        }
    }
    output[j] = '\0';
    return output;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.b>\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    char *content = cleanupfile(readentirefile(file));
    fclose(file);
    OPERATORS *operators = parser(content);
    free(content);
    runtime(operators);
    free(operators->items);
    free(operators);

    return EXIT_SUCCESS;
}
