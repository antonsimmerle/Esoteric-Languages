#include <stdio.h>
#include <stdlib.h>

#include <limits.h>

#define NONE -1

typedef enum CHARACTER CHARACTER;

typedef enum OPERATOR OPERATOR;
typedef struct COMMAND COMMAND;
typedef struct COMMANDS COMMANDS;

typedef struct STACKNODE STACKNODE;
typedef struct STACK STACK;

typedef struct HEAP HEAP;

typedef struct RUNTIMEDATA RUNTIMEDATA;
typedef struct PARSERDATA PARSERDATA;

typedef enum PARAMETERTYPE PARAMETERTYPE;
typedef struct MAP MAP;
typedef struct TRIENODE TRIENODE;

char *readentirefile(FILE *input);
size_t getlength(const char *input);
int getcompare(const char *left, const char *right, size_t length);

void store(HEAP *heap, int address, int value);
int retrieve(HEAP *heap, int address);

TRIENODE *createnode();
size_t chartobranch(char input);
void inserttrie(TRIENODE *root, const char *sequence, OPERATOR operator, PARAMETERTYPE type);
void freetrie(TRIENODE *node);
TRIENODE *searchtrie(TRIENODE *root, const char *input, size_t *index);
int getparameter(const char *input, size_t *index, PARAMETERTYPE type);
PARSERDATA *parser(const char *input);

void push(STACK *stack, int value);
int pop(STACK *stack);
void runtime(PARSERDATA *input);

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

size_t getlength(const char *input) {
    size_t i = 0;
    for (; input[i] != '\0'; i++);
    return i;
}

#define CHUNK 10

enum CHARACTER {
    SPACE    = ' ',
    TAB      = '\t',
    LINEFEED = '\n',
};

enum OPERATOR {
    PUSH, DUPLICATE, COPY, SWAP, DISCARD, SLIDE,
    ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO,
    STORE, RETRIEVE,
    PRINTCHARACTER, PRINTNUMBER, READCHARACTER, READNUMBER,
    MARK, CALL, JUMP, JUMPIFZERO, JUMPIFNEGATIVE, RETURN, END,
};
struct COMMAND {
    OPERATOR operator;
    int parameter;
};
struct COMMANDS {
    COMMAND *items;
    size_t count;
    size_t capacity;
};

struct STACKNODE {
    int value;
    STACKNODE *next;
};
struct STACK {
    STACKNODE *items;
    size_t count;
};

struct HEAP {
    int *items;
    size_t capacity;
};

struct RUNTIMEDATA {
    STACK stack;
    HEAP heap;
    STACK calls;
};
struct PARSERDATA {
    COMMANDS commands;
    HEAP labels;
};

void store(HEAP *heap, int address, int value) {
    if (address >= heap->capacity) {
        heap->capacity = address + CHUNK;
        heap->items = realloc(heap->items, sizeof(int) * heap->capacity);
        if (!heap->items) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    heap->items[address] = value;
}
int retrieve(HEAP *heap, int address) {
    if (address >= heap->capacity) {
        fprintf(stderr, "Error: Heap access out of bounds\n");
        exit(EXIT_FAILURE);
    }
    return heap->items[address];
}

enum PARAMETERTYPE {
    SIGNED,
    UNSIGNED,
};
struct TRIENODE {
    TRIENODE *children[3];
    OPERATOR operator;
    PARAMETERTYPE type;
};
struct MAP {
    const char *sequence;
    OPERATOR operator;
    PARAMETERTYPE type;
};
MAP map[] = {
    { "  ",       PUSH,           SIGNED   },
    { " \n ",     DUPLICATE,      NONE     },
    { " \t ",     COPY,           SIGNED   },
    { " \n\t",    SWAP,           NONE     },
    { " \n\n",    DISCARD,        NONE     },
    { " \t\n",    SLIDE,          SIGNED   },
    { "\t   ",    ADD,            NONE     },
    { "\t  \t",   SUBTRACT,       NONE     },
    { "\t  \n",   MULTIPLY,       NONE     },
    { "\t \t ",   DIVIDE,         NONE     },
    { "\t \t\t",  MODULO,         NONE     },
    { "\t\t ",    STORE,          NONE     },
    { "\t\t\t",   RETRIEVE,       NONE     },
    { "\t\n  ",   PRINTCHARACTER, NONE     },
    { "\t\n \t",  PRINTNUMBER,    NONE     },
    { "\t\n\t ",  READCHARACTER,  NONE     },
    { "\t\n\t\t", READNUMBER,     NONE     },
    { "\n  ",     MARK,           UNSIGNED },
    { "\n \t",    CALL,           UNSIGNED },
    { "\n \n",    JUMP,           UNSIGNED },
    { "\n\t ",    JUMPIFZERO,     UNSIGNED },
    { "\n\t\t",   JUMPIFNEGATIVE, UNSIGNED },
    { "\n\t\n",   RETURN,         NONE     },
    { "\n\n\n",   END,            NONE     },
    { NULL,       NONE,           NONE     }
};
TRIENODE *createnode() {
    TRIENODE *node = calloc(1, sizeof(TRIENODE));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->operator = NONE;
    return node;
}
size_t chartobranch(char input) {
    switch (input) {
        case SPACE:    return 0;
        case TAB:      return 1;
        case LINEFEED: return 2;
    }
}
void inserttrie(TRIENODE *root, const char *sequence, OPERATOR operator, PARAMETERTYPE type) {
    TRIENODE *node = root;
    while (*sequence) {
        size_t branch = chartobranch(*sequence);
        if (!node->children[branch]) {
            node->children[branch] = createnode();
        }
        node = node->children[branch];
        sequence++;
    }
    node->operator = operator;
    node->type = type;
}
void freetrie(TRIENODE *node) {
    if (node) {
        for (size_t i = 0; i < 3; i++) {
            freetrie(node->children[i]);
        }
        free(node);
    }
}
TRIENODE *searchtrie(TRIENODE *node, const char *input, size_t *index) {
    if (!node || input[*index] == '\0') {
        return NULL;
    }
    size_t branch = chartobranch(input[*index]);
    node = node->children[branch];
    if (!node) {
        return NULL;
    }
    (*index)++;
    if (node->operator != NONE) {
        return node;
    }
    return searchtrie(node, input, index);
}
int getparameter(const char *input, size_t *index, PARAMETERTYPE type) {
    int value = 0;
    int sign = 1;
    if (type == SIGNED) {
        sign = input[*index] == TAB ? -1 : 1;
        (*index)++;
    }
    for (; input[*index] != LINEFEED; (*index)++) {
        value = (value << 1) + (input[*index] == TAB);
    }
    (*index)++;
    return value * sign;
}
PARSERDATA *parser(const char *input) {
    TRIENODE *root = createnode();
    for (size_t k = 0; map[k].sequence; k++) {
        inserttrie(root, map[k].sequence, map[k].operator, map[k].type);
    }
    PARSERDATA *parserdata = calloc(1, sizeof(PARSERDATA));
    if (!parserdata) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    size_t i = 0;
    size_t j = 0;
    while (input[i] != '\0') {
        if (j >= parserdata->commands.capacity) {
            parserdata->commands.capacity += CHUNK;
            parserdata->commands.items = realloc(parserdata->commands.items, sizeof(COMMAND) * parserdata->commands.capacity);
            if (!parserdata->commands.items) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
        }
        TRIENODE *match = searchtrie(root, input, &i);
        if (!match) {
            fprintf(stderr, "Error: Invalid command at %zu\n", i);
            exit(EXIT_FAILURE);
        }
        parserdata->commands.items[j].operator = match->operator;
        if (match->type != NONE) {
            parserdata->commands.items[j].parameter = getparameter(input, &i, match->type);
        }
        if (match->operator == MARK) {
            store(&parserdata->labels, parserdata->commands.items[j].parameter, j + 1);
        }
        j++;
    }
    parserdata->commands.count = j;
    freetrie(root);
    return parserdata;
}

void push(STACK *stack, int value) {
    stack->count++;
    STACKNODE *node = malloc(sizeof(STACKNODE));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->value = value;
    node->next = stack->items;
    stack->items = node;
}
int pop(STACK *stack) {
    if (stack->count == 0) {
        fprintf(stderr, "Error: Pop from empty stack\n");
        exit(EXIT_FAILURE);
    }
    stack->count--;
    STACKNODE *node = stack->items;
    int value = node->value;
    if (node) {
        stack->items = stack->items->next;
    }
    free(node);
    return value;
}
void runtime(PARSERDATA *input) {
    RUNTIMEDATA runtimedata = {
        .stack = { .items = NULL, .count    = 0, },
        .heap =  { .items = NULL, .capacity = 0, },
        .calls = { .items = NULL, .count =    0, },
    };
    size_t i = 0;
    while (i < input->commands.count) {
        int parameter = input->commands.items[i].parameter;
        switch (input->commands.items[i].operator) {
            case PUSH: {
                push(&runtimedata.stack, parameter);
                i++;
            } break;
            case DUPLICATE: {
                push(&runtimedata.stack, runtimedata.stack.items->value);
                i++;
            } break;
            case COPY: {
                if (parameter >= runtimedata.stack.count) {
                    fprintf(stderr, "Error: Index out of range\n");
                    exit(EXIT_FAILURE);
                }
                STACKNODE *node = runtimedata.stack.items;
                for  (size_t j = 0; j < parameter; j++) {
                    node = node->next;
                }
                push(&runtimedata.stack, node->value);
                i++;
            } break;
            case SWAP: {
                int left = pop(&runtimedata.stack);
                int right = pop(&runtimedata.stack);
                push(&runtimedata.stack, left);
                push(&runtimedata.stack, right);
                i++;
            } break;
            case DISCARD: {
                pop(&runtimedata.stack);
                i++;
            } break;
            case SLIDE: {
                int top = pop(&runtimedata.stack);
                for (size_t j = 0; j < parameter; j++) {
                    pop(&runtimedata.stack);
                }
                push(&runtimedata.stack, top);
                i++;
            } break;
            case ADD: {
                int left = pop(&runtimedata.stack);
                int right = pop(&runtimedata.stack);
                if ((right > 0 && left > INT_MAX - right) || (right < 0 && left < INT_MIN - right)) {
                    fprintf(stderr, "Error: Integer overflow\n");
                    exit(EXIT_FAILURE);
                }
                push(&runtimedata.stack, left + right);
                i++;
            } break;
            case SUBTRACT: {
                int left = pop(&runtimedata.stack);
                int right = pop(&runtimedata.stack);
                if ((left > 0 && right < INT_MIN + left) || (left < 0 && right > INT_MAX + left)) {
                    fprintf(stderr, "Error: Integer overflow\n");
                    exit(EXIT_FAILURE);
                }
                push(&runtimedata.stack, right - left);
                i++;
            } break;
            case MULTIPLY: {
                int left = pop(&runtimedata.stack);
                int right = pop(&runtimedata.stack);
                if (left != 0 && (right > INT_MAX / left || right < INT_MIN / left)) {
                    fprintf(stderr, "Error: Integer overflow\n");
                    exit(EXIT_FAILURE);
                }
                push(&runtimedata.stack, left * right);
                i++;
            } break;
            case DIVIDE: {
                int left = pop(&runtimedata.stack);
                int right = pop(&runtimedata.stack);
                if (left == 0) {
                    fprintf(stderr, "Error: Division by zero\n");
                    exit(EXIT_FAILURE);
                }
                push(&runtimedata.stack, right / left);
                i++;
            } break;
            case MODULO: {
                int left = pop(&runtimedata.stack);
                int right = pop(&runtimedata.stack);
                if (left == 0) {
                    fprintf(stderr, "Error: Division by zero\n");
                    exit(EXIT_FAILURE);
                }
                push(&runtimedata.stack, right % left);
                i++;
            } break;
            case STORE: {
                int value = pop(&runtimedata.stack);
                int address = pop(&runtimedata.stack);
                store(&runtimedata.heap, address, value);
                i++;
            } break;
            case RETRIEVE: {
                int address = pop(&runtimedata.stack);
                int value = retrieve(&runtimedata.heap, address);
                push(&runtimedata.stack, value);
                i++;
            } break;
            case PRINTCHARACTER: {
                printf("%c", (char)pop(&runtimedata.stack));
                i++;
            } break;
            case PRINTNUMBER: {
                printf("%d", pop(&runtimedata.stack));
                i++;
            } break;
            case READCHARACTER: {
                char read;
                int address = pop(&runtimedata.stack);
                scanf("%c", &read);
                store(&runtimedata.heap, address, read);
                i++;
            } break;
            case READNUMBER: {
                int read;
                int address = pop(&runtimedata.stack);
                scanf("%d", &read);
                store(&runtimedata.heap, address, read);
                i++;
            } break;
            case MARK: {
                i++;
            } break;
            case CALL: {
                push(&runtimedata.calls, i + 1);
                i = retrieve(&input->labels, parameter);
            } break;
            case JUMP: {
                i = retrieve(&input->labels, parameter);
            } break;
            case JUMPIFZERO: {
                if (pop(&runtimedata.stack) == 0) {
                    i = retrieve(&input->labels, parameter);
                } else {
                    i++;
                }
            } break;
            case JUMPIFNEGATIVE: {
                if (pop(&runtimedata.stack) < 0) {
                    i = retrieve(&input->labels, parameter);
                } else {
                    i++;
                }
            } break;
            case RETURN: {
                i = pop(&runtimedata.calls);
            } break;
            case END: {
                exit(EXIT_SUCCESS);
            } break;
        }
    }
    free(runtimedata.stack.items);
    free(runtimedata.heap.items);
    free(runtimedata.calls.items);
}

char *cleanupfile(const char *input) {
    char *output = malloc(sizeof(char) * getlength(input));
    if (!output) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case SPACE:
            case TAB:
            case LINEFEED: {
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
        fprintf(stderr, "Usage: %s <input.ws>\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    char *content = cleanupfile(readentirefile(file));
    fclose(file);
    PARSERDATA *parserdata = parser(content);
    free(content);
    runtime(parserdata);
    free(parserdata->commands.items);
    free(parserdata->labels.items);
    free(parserdata);

    return EXIT_SUCCESS;
}
