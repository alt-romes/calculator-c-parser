#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Create parser structure to keep track of the tokens consumed
typedef struct parser_t {
    char* tokens;
    int ntokens;
    int pos;
} parser_t;

// Create expression tree structure to hold our expression
typedef struct exprtree {
    char type;
    int value;
    struct exprtree* left;
    struct exprtree* right;
} exprtree;

#define VALID_TOKENS "+-*/0123456789()"

#define MAX_INPUT_SIZE 100

// Important function declarations
char* tokenize(char*);
exprtree* parse(char*);
int calculate(exprtree*);

// Helper parsing functions
static void free_exprtree(exprtree*);
static exprtree* create_exprtree(char, int, exprtree*, exprtree*);
exprtree* parse_add_expression(parser_t* parser);
exprtree* parse_mult_expression(parser_t* parser);
exprtree* parse_atomic_expression(parser_t* parser);
exprtree* parse_number(parser_t* parser);

/* Main loop */
int main(int argc, char* argv[]) {

    while (1) {

        // 1. Get input from the user
        char in[MAX_INPUT_SIZE];
        printf("Enter input: ");
        scanf("\n%[^\n]", in); // Read until \n

        // 2. Get tokens from the input string
        char* tokens = tokenize(in);

        // 3. Create expression tree from tokens
        exprtree* expression = parse(tokens);

        // 4. Calculate the value of the expression
        int value = calculate(expression);

        // 5. Print result
        printf("The result is: %d\n", value);

        // 6. Free the memory allocated for the expression
        free_exprtree(expression);

    }
    
    return 0;
}

/* Convert input string into tokens */
char* tokenize(char* in) {

    // Allocate space for the tokens (each token is a character)
    char* tokens = malloc(sizeof(char) * MAX_INPUT_SIZE);

    int token_pos = 0; // token_pos will track the amount of tokens in the token string

    // Iterate over the input string and everytime a token is found, add it to the tokens
    int in_len = strlen(in);
    for (int i = 0; i < in_len; i++)
        // Check if input character is a valid token
        if (strchr(VALID_TOKENS, in[i]))
            // Add to tokens if it is
            tokens[token_pos++] = in[i];

    // Terminate the tokens string with null
    tokens[token_pos] = '\0';

    return tokens;
}

/* Calculator grammar:
 *
 * add_expression := mult_expression (('+' | '-') mult_expression)*
 *
 * mult_expression := atomic_expression (('*' | '/') atomic_expression)*
 *
 * atomic_expression := number | left_parenthesis add_expression right_parenthesis
 *
 * number := (0-9)+
 *
 */

/* Parse tokens into expression tree based on grammar */
exprtree* parse(char* tokens) {

    // Allocate memory for the parse structure
    parser_t* parser = malloc(sizeof(parser_t));

    // Set initial values for the parser
    parser->tokens = tokens;
    parser->ntokens = strlen(tokens);
    parser->pos = 0;

    // Calculate the expression starting by parsing the tokens starting with the lowest priority
    exprtree* expression = parse_add_expression(parser);

    // Free allocated memory
    free(parser->tokens);
    free(parser);
    
    return expression;
}

int calculate(exprtree* expr) {

    if (expr->type == 'n')
        return expr->value;

    int left = calculate(expr->left);
    int right = calculate(expr->right);
    
    if (expr->type == '+')
        return left + right;
    else if (expr->type == '-')
        return left - right;
    else if (expr->type == '*')
        return left * right;
    else if (expr->type == '/')
        return left / right ? right : 0;

    return 0;
}

exprtree* parse_add_expression(parser_t* parser) {

    /* add_expression := mult_expression (('+' | '-') mult_expression) */

    // An add_expression is composed firstly of a mult_expression, so we parse one right away
    exprtree* expr = parse_mult_expression(parser);
    
    // After the mult_expression, add_expression can find 0 or more (+ or -) followed by another mult_expression
    while (parser->pos < parser->ntokens &&
            (parser->tokens[parser->pos] == '+' || parser->tokens[parser->pos] == '-')) {

        // Set expression type as either addition or subtraction
        char type = parser->tokens[parser->pos];

        // Consume the addition or subtraction token
        parser->pos++;

        // Parse a mult_expression that should come right after (+ or -)
        exprtree* right_expr = parse_mult_expression(parser);

        // We create a new expression with the ... //TODO
        expr = create_exprtree(type, 0, expr, right_expr); // value is set to 0 because operations don't use it
    }
    
    return expr;

} 

exprtree* parse_mult_expression(parser_t* parser) {


    /* mult_expression := atomic_expression (('*' | '/') atomic_expression) */

    // a mult_expression is composed firstly of an atomic_expression, so we parse one right away
    exprtree* expr = parse_atomic_expression(parser);
    
    // mult_expression can find 0 or more (* or /) followed by another atomic_expression
    while (parser->pos < parser->ntokens &&
            (parser->tokens[parser->pos] == '*' || parser->tokens[parser->pos] == '/')) {

        // set expression type as either multiplication or division
        char type = parser->tokens[parser->pos];

        // consume the multiplication or division token
        parser->pos++;

        // parse an atomic_expression that should come right after (* or /)
        exprtree* right_expr = parse_atomic_expression(parser);

        // we create a new expression with the //TODO
        expr = create_exprtree(type, 0, expr, right_expr); // value is set to 0 because operations don't use it
    }
    
    return expr;

}

exprtree* parse_atomic_expression(parser_t* parser) {

    /* atomic_expression := number | left_parenthesis add_expression right_parenthesis */

    exprtree* expr;

    // If we find parenthesis, then we read an add_expression as an atomic one
    if (parser->tokens[parser->pos] == '(') {

        parser->pos++; // Consume parenthesis

        // Parse add_expression that should come between parenthesis
        expr = parse_add_expression(parser);

        // Consume the closing parenthesis
        if (parser->tokens[parser->pos] == ')')
            parser->pos++;
        else {
            
            // Error if there aren't any
            fprintf(stderr, "Invalid input\n");
            exit(1);
        }
        
    } else {
 
        // This is the alternative production rule - an atomic expression can be just a number
        expr = parse_number(parser);
    }
    
    return expr;

}

exprtree* parse_number(parser_t* parser) {

    /* number := (0-9)+ */

    char number[MAX_INPUT_SIZE];
    int numberlen = 0;

    // We'll read the consecutive numbers into a character array, and then convert it to a number with atoi
    while (strchr("0123456789", parser->tokens[parser->pos]) &&
            numberlen < MAX_INPUT_SIZE && parser->pos < parser->ntokens) {

        number[numberlen++] = parser->tokens[parser->pos];
        parser->pos++;
    }
    number[numberlen] = '\0';

    // When no number characters could be found
    if (numberlen == 0) {
        fprintf(stderr, "Invalid input, couldn't parse number\n");
        exit(1);
    }

    // Convert the number characters array to an int
    int value = atoi(number);
    
    // Create an expression of type 'n' with the value set as the number value
    exprtree* number_expr = create_exprtree('n', value, NULL, NULL);

    return number_expr;

}

static exprtree* create_exprtree(char type, int value, exprtree* left, exprtree* right) {

    // Allocate memory for the expression
    exprtree* expr = malloc(sizeof(exprtree));

    // Set values for the expression
    expr->type = type;
    expr->value = value;
    expr->left = left;
    expr->right = right;

    return expr;

}

static void free_exprtree(exprtree* expr) {

    // Free the expression recursively

    if (expr) {

        if (expr->left)
            free_exprtree(expr->left);
        if (expr->right)
            free_exprtree(expr->right);

        free(expr);

    }

}
