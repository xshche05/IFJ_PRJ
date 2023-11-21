#include "parts.h"
#include "utils.h"
#include "new_symtable.h"
#include "../error.h"

bool inside_func = false;
bool inside_loop = false;
bool inside_branch = false;
int scope = 0;
int stayed = 0;
token_t *last_token = NULL;
token_t *last_id = NULL;

static char *tokens_as_str[] = {
        "TOKEN_IDENTIFIER",
        "TOKEN_DOUBLE_TYPE",
        "TOKEN_ELSE",
        "TOKEN_FUNC",
        "TOKEN_IF",
        "TOKEN_INT_TYPE",
        "TOKEN_LET",
        "TOKEN_RETURN",
        "TOKEN_STRING_TYPE",
        "TOKEN_VAR",
        "TOKEN_WHILE",
        "TOKEN_BOOL_TYPE",
        "TOKEN_FOR",
        "TOKEN_IN",
        "TOKEN_BREAK",
        "TOKEN_CONTINUE",
        "TOKEN_UNDERSCORE",
        "TOKEN_ASSIGNMENT",
        "TOKEN_CLOSED_RANGE",
        "TOKEN_HALF_OPEN_RANGE",
        "TOKEN_REAL_LITERAL",
        "TOKEN_STRING_LITERAL",
        "TOKEN_NIL_LITERAL",
        "TOKEN_TRUE_LITERAL",
        "TOKEN_FALSE_LITERAL",
        "TOKEN_INTEGER_LITERAL",
        "TOKEN_ADDITION",
        "TOKEN_SUBTRACTION",
        "TOKEN_MULTIPLICATION",
        "TOKEN_DIVISION",
        "TOKEN_LESS_THAN",
        "TOKEN_LESS_THAN_OR_EQUAL_TO",
        "TOKEN_GREATER_THAN",
        "TOKEN_GREATER_THAN_OR_EQUAL_TO",
        "TOKEN_EQUAL_TO",
        "TOKEN_NOT_EQUAL_TO",
        "TOKEN_IS_NIL",
        "TOKEN_UNWRAP_NILLABLE",
        "TOKEN_LOGICAL_AND",
        "TOKEN_LOGICAL_OR",
        "TOKEN_LOGICAL_NOT",
        "TOKEN_LEFT_BRACKET",
        "TOKEN_RIGHT_BRACKET",
        "TOKEN_LEFT_BRACE",
        "TOKEN_RIGHT_BRACE",
        "TOKEN_COMMA",
        "TOKEN_COLON",
        "TOKEN_SEMICOLON",
        "TOKEN_ARROW",
        "TOKEN_NEWLINE",
        "TOKEN_EOF"
};

bool match(token_type_t type) {
    if (type == TOKEN_RETURN) {
        if (!inside_func) {
            fprintf(stderr, "Syntax error: TOKEN_RETURN outside of function\n");
            exit(BAD_SYNTAX_ERR);
            return false;
        }
    }
    if (type == TOKEN_BREAK || type == TOKEN_CONTINUE) {
        if (!inside_loop) {
            fprintf(stderr, "Syntax error: %s outside of loop\n", tokens_as_str[type]);
            exit(BAD_SYNTAX_ERR);
            return false;   //TODO SEMANTIC ERROR
        }
    }
    if (type == TOKEN_FUNC) {
        if (scope - 1 != 0 || inside_loop || inside_branch) { //TODO FIX
            fprintf(stderr, "Syntax error: function declaration outside of global scope\n", tokens_as_str[type]);
            exit(BAD_SYNTAX_ERR);
            return false; //TODO SEMANTIC ERROR
        }
    }
    if (lookahead->type == type) {
        nl_flag = lookahead->has_newline_after;
        last_token = lookahead;
        if (lookahead->type == TOKEN_IDENTIFIER) {
            last_id = lookahead;
        }
        lookahead = TokenArray.next();
        return true;
    }
    fprintf(stderr, "Syntax error: expected %s, got %s\n", tokens_as_str[type], tokens_as_str[lookahead->type]);
    exit(BAD_SYNTAX_ERR);
    return false;
}

void increment_scope() {
    scope++;
    printf("scope inc: %d\n", scope);
}

void decrement_scope() {
    scope--;
    printf("scope dec: %d\n", scope);
    stayed = 0;
}

void scope_new() {
    stayed++;
    printf("scope stay: %d\n", scope);
}

void scope_leave() {
    printf("scope leave: %d\n", scope);
}

bool call_expr_parser(token_t *token) {
    return match(token->type);
}

bool nl_check() {
    if (nl_flag) {
        return true;
    }
    sprintf(error_msg, "Syntax error: New line expected\n");
    return false;
}

bool S() {
    bool s;
    lookahead = TokenArray.next();
    switch (lookahead->type) {
        case TOKEN_FUNC:
        case TOKEN_LET:
        case TOKEN_CONTINUE:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_IDENTIFIER:
        case TOKEN_RETURN:
        case TOKEN_BREAK:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_EOF:
            s = CODE();
            s = s && match(TOKEN_EOF);
            break;
        default:
            fprintf(stderr, "Syntax error [S]: expected ['TOKEN_EOF', 'TOKEN_IF', 'TOKEN_IDENTIFIER', 'TOKEN_WHILE', 'TOKEN_FOR', 'TOKEN_VAR', 'TOKEN_LET', 'TOKEN_RETURN', 'TOKEN_FUNC', 'TOKEN_BREAK', 'TOKEN_CONTINUE'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool CODE() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_VAR:
            s = VAR_DECL();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_LET:
            s = LET_DECL();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_FUNC:
            s = FUNC_DECL();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_WHILE:
            s = WHILE_LOOP();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_FOR:
            s = FOR_LOOP();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_IF:
            s = BRANCH();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_IDENTIFIER:
            s = ID_CALL_OR_ASSIGN();
            s = s && nl_check();
            s = s && CODE();
            break;
        case TOKEN_RETURN:
            s = RETURN();
            break;
        case TOKEN_BREAK:
            s = match(TOKEN_BREAK);
            break;
        case TOKEN_CONTINUE:
            s = match(TOKEN_CONTINUE);
            break;
        case TOKEN_RIGHT_BRACE:
        case TOKEN_EOF:
            s = true;
            break;
        default:
            fprintf(stderr, "Syntax error [CODE]: expected ['TOKEN_VAR', 'TOKEN_LET', 'TOKEN_FUNC', 'TOKEN_WHILE', 'TOKEN_FOR', 'TOKEN_IF', 'TOKEN_IDENTIFIER', 'TOKEN_RETURN', 'TOKEN_BREAK', 'TOKEN_CONTINUE', 'TOKEN_RIGHT_BRACE', 'TOKEN_EOF'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool RETURN() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_RETURN:
            s = match(TOKEN_RETURN);
            s = s && RET_EXPR();
            break;
        default:
            fprintf(stderr, "Syntax error [RETURN]: expected ['TOKEN_RETURN'], got %s\n", tokens_as_str[lookahead->type]);
            s = false;
    }
    return s;
}


bool RET_EXPR() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_RIGHT_BRACE:
        case TOKEN_EOF:
            s = true;
            break;
        default:
            if (call_expr_parser(lookahead)) return true;
            fprintf(stderr, "Syntax error [RET_EXPR]: expected ['TOKEN_FALSE_LITERAL', 'TOKEN_LESS_THAN', 'TOKEN_LOGICAL_AND', 'TOKEN_LEFT_BRACKET', 'TOKEN_NIL_LITERAL', 'TOKEN_REAL_LITERAL', 'TOKEN_STRING_LITERAL', 'TOKEN_ADDITION', 'TOKEN_SUBTRACTION', 'TOKEN_LOGICAL_OR', 'TOKEN_IDENTIFIER', 'TOKEN_EQUAL_TO', 'TOKEN_LESS_THAN_OR_EQUAL_TO', 'TOKEN_GREATER_THAN', 'TOKEN_NOT_EQUAL_TO', 'TOKEN_GREATER_THAN_OR_EQUAL_TO', 'TOKEN_TRUE_LITERAL', 'TOKEN_IS_NIL', 'TOKEN_DIVISION', 'TOKEN_MULTIPLICATION', 'TOKEN_LOGICAL_NOT', 'TOKEN_UNWRAP_NILLABLE', 'TOKEN_INTEGER_LITERAL', 'TOKEN_RIGHT_BRACE', 'TOKEN_EOF'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool VAR_DECL() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_VAR:
            s = match(TOKEN_VAR);
            s = s && match(TOKEN_IDENTIFIER);
            s = s && VAR_LET_TYPE();
            s = s && VAR_LET_EXP();
            break;
        default:
            fprintf(stderr, "Syntax error [VAR_DECL]: expected ['TOKEN_VAR'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool VAR_LET_TYPE() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_COLON:
            s = match(TOKEN_COLON);
            s = s && TYPE();
            break;
        case TOKEN_ASSIGNMENT:
        default:
            if (nl_flag) return true;
            fprintf(stderr, "Syntax error [VAR_LET_TYPE]: expected ['TOKEN_COLON', 'TOKEN_ASSIGNMENT', 'NL'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool VAR_LET_EXP() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_ASSIGNMENT:
            s = match(TOKEN_ASSIGNMENT);
            s = s && call_expr_parser(lookahead);
            break;
        default:
            if (nl_flag) return true;
            fprintf(stderr, "Syntax error [VAR_LET_EXP]: expected ['TOKEN_ASSIGNMENT', 'NL'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool LET_DECL() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_LET:
            s = match(TOKEN_LET);
            s = s && match(TOKEN_IDENTIFIER);
            s = s && VAR_LET_TYPE();
            s = s && VAR_LET_EXP();
            break;
        default:
            fprintf(stderr, "Syntax error [LET_DECL]: expected ['TOKEN_LET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool FUNC_DECL() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_FUNC:
            inside_func = true;
            increment_scope();
            s = match(TOKEN_FUNC);
            s = s && match(TOKEN_IDENTIFIER);
            s = s && match(TOKEN_LEFT_BRACKET);
            s = s && PARAM_LIST();
            s = s && match(TOKEN_RIGHT_BRACKET);
            s = s && FUNC_RET_TYPE();
            s = s && match(TOKEN_LEFT_BRACE);
            s = s && CODE();
            s = s && match(TOKEN_RIGHT_BRACE);
            inside_func = false;
            decrement_scope();
            break;
        default:
            fprintf(stderr, "Syntax error [FUNC_DECL]: expected ['TOKEN_FUNC'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool FUNC_RET_TYPE() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_ARROW:
            s = match(TOKEN_ARROW);
            s = s && TYPE();
            break;
        case TOKEN_LEFT_BRACE:
            s = true;
            break;
        default:
            fprintf(stderr, "Syntax error [FUNC_RET_TYPE]: expected ['TOKEN_ARROW', 'TOKEN_LEFT_BRACE'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool PARAM_LIST() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_UNDERSCORE:
            s = PARAM();
            s = s && NEXT_PARAM();
            break;
        case TOKEN_RIGHT_BRACKET:
            s = true;
            break;
        default:
            fprintf(stderr, "Syntax error [PARAM_LIST]: expected ['TOKEN_UNDERSCORE', 'TOKEN_IDENTIFIER', 'TOKEN_RIGHT_BRACKET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool PARAM() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_UNDERSCORE:
            s = PARAM_NAME();
            s = s && match(TOKEN_IDENTIFIER);
            s = s && match(TOKEN_COLON);
            s = s && TYPE();
            break;
        default:
            fprintf(stderr, "Syntax error [PARAM]: expected ['TOKEN_UNDERSCORE', 'TOKEN_IDENTIFIER'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool PARAM_NAME() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
            s = match(TOKEN_IDENTIFIER);
            break;
        case TOKEN_UNDERSCORE:
            s = match(TOKEN_UNDERSCORE);
            break;
        default:
            fprintf(stderr, "Syntax error [PARAM_NAME]: expected ['TOKEN_IDENTIFIER', 'TOKEN_UNDERSCORE'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool NEXT_PARAM() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_COMMA:
            s = match(TOKEN_COMMA);
            s = s && PARAM();
            s = s && NEXT_PARAM();
            break;
        case TOKEN_RIGHT_BRACKET:
            s = true;
            break;
        default:
            fprintf(stderr, "Syntax error [NEXT_PARAM]: expected ['TOKEN_COMMA', 'TOKEN_RIGHT_BRACKET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool BRANCH() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IF:
            inside_branch = true;
            increment_scope();
            s = match(TOKEN_IF);
            s = s && BR_EXPR();
            s = s && match(TOKEN_LEFT_BRACE);
            s = s && CODE();
            s = s && match(TOKEN_RIGHT_BRACE);
            s = s && ELSE();
            inside_branch = false;
            decrement_scope();
            break;
        default:
            fprintf(stderr, "Syntax error [BRANCH]: expected ['TOKEN_IF'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool BR_EXPR() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_LET:
            s = match(TOKEN_LET);
            s = s && match(TOKEN_IDENTIFIER);
            break;
        default:
            if (call_expr_parser(lookahead)) return true;
            fprintf(stderr, "Syntax error [BR_EXPR]: expected ['TOKEN_FALSE_LITERAL', 'TOKEN_LESS_THAN', 'TOKEN_LOGICAL_AND', 'TOKEN_LEFT_BRACKET', 'TOKEN_NIL_LITERAL', 'TOKEN_REAL_LITERAL', 'TOKEN_STRING_LITERAL', 'TOKEN_ADDITION', 'TOKEN_SUBTRACTION', 'TOKEN_LOGICAL_OR', 'TOKEN_IDENTIFIER', 'TOKEN_EQUAL_TO', 'TOKEN_LESS_THAN_OR_EQUAL_TO', 'TOKEN_GREATER_THAN', 'TOKEN_NOT_EQUAL_TO', 'TOKEN_GREATER_THAN_OR_EQUAL_TO', 'TOKEN_TRUE_LITERAL', 'TOKEN_IS_NIL', 'TOKEN_DIVISION', 'TOKEN_MULTIPLICATION', 'TOKEN_LOGICAL_NOT', 'TOKEN_UNWRAP_NILLABLE', 'TOKEN_INTEGER_LITERAL', 'TOKEN_LET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool ELSE() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_ELSE:
            s = match(TOKEN_ELSE);
            s = s && ELSE_IF();
            break;
        default:
            if (nl_flag) return true;
            fprintf(stderr, "Syntax error [ELSE]: expected ['TOKEN_ELSE', 'NL'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool ELSE_IF() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IF:
            scope_new();
            s = match(TOKEN_IF);
            s = s && BR_EXPR();
            s = s && match(TOKEN_LEFT_BRACE);
            s = s && CODE();
            s = s && match(TOKEN_RIGHT_BRACE);
            s = s && ELSE();
            scope_leave();
            break;
        case TOKEN_LEFT_BRACE:
            scope_new();
            s = match(TOKEN_LEFT_BRACE);
            s = s && CODE();
            s = s && match(TOKEN_RIGHT_BRACE);
            scope_leave();
            break;
        default:
            if (nl_flag) return true;
            fprintf(stderr, "Syntax error [ELSE_IF]: expected ['TOKEN_IF', 'TOKEN_LEFT_BRACE', 'NL'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool WHILE_LOOP() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_WHILE:
            inside_loop = true;
            increment_scope();
            s = match(TOKEN_WHILE);
            s = s && call_expr_parser(lookahead);
            s = s && match(TOKEN_LEFT_BRACE);
            s = s && CODE();
            s = s && match(TOKEN_RIGHT_BRACE);
            inside_loop = false;
            decrement_scope();
            break;
        default:
            fprintf(stderr, "Syntax error [WHILE_LOOP]: expected ['TOKEN_WHILE'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool FOR_LOOP() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_FOR:
            inside_loop = true;
            increment_scope();
            s = match(TOKEN_FOR);
            s = s && FOR_ID();
            s = s && match(TOKEN_IN);
            s = s && call_expr_parser(lookahead);
            s = s && RANGE();
            s = s && match(TOKEN_LEFT_BRACE);
            s = s && CODE();
            s = s && match(TOKEN_RIGHT_BRACE);
            inside_loop = false;
            decrement_scope();
            break;
        default:
            fprintf(stderr, "Syntax error [FOR_LOOP]: expected ['TOKEN_FOR'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool FOR_ID() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
            s = match(TOKEN_IDENTIFIER);
            break;
        case TOKEN_UNDERSCORE:
            s = match(TOKEN_UNDERSCORE);
            break;
        default:
            fprintf(stderr, "Syntax error [FOR_ID]: expected ['TOKEN_IDENTIFIER', 'TOKEN_UNDERSCORE'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool RANGE() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_CLOSED_RANGE:
            s = match(TOKEN_CLOSED_RANGE);
            s = s && call_expr_parser(lookahead);
            break;
        case TOKEN_HALF_OPEN_RANGE:
            s = match(TOKEN_HALF_OPEN_RANGE);
            s = s && call_expr_parser(lookahead);
            break;
        default:
            fprintf(stderr, "Syntax error [RANGE]: expected ['TOKEN_CLOSED_RANGE', 'TOKEN_HALF_OPEN_RANGE'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool CALL_PARAM_LIST() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
            s = CALL_PARAM();
            s = s && NEXT_CALL_PARAM();
            break;
        case TOKEN_RIGHT_BRACKET:
            s = true;
            break;
        default:
            if (call_expr_parser(lookahead)) return true;
            fprintf(stderr, "Syntax error [CALL_PARAM_LIST]: expected ['TOKEN_FALSE_LITERAL', 'TOKEN_COLON', 'TOKEN_LESS_THAN', 'TOKEN_LOGICAL_AND', 'TOKEN_LEFT_BRACKET', 'TOKEN_NIL_LITERAL', 'TOKEN_REAL_LITERAL', 'TOKEN_STRING_LITERAL', 'TOKEN_ADDITION', 'TOKEN_SUBTRACTION', 'TOKEN_LOGICAL_OR', 'TOKEN_IDENTIFIER', 'TOKEN_UNWRAP_NILLABLE', 'TOKEN_EQUAL_TO', 'TOKEN_LESS_THAN_OR_EQUAL_TO', 'TOKEN_GREATER_THAN', 'TOKEN_NOT_EQUAL_TO', 'TOKEN_GREATER_THAN_OR_EQUAL_TO', 'TOKEN_TRUE_LITERAL', 'TOKEN_IS_NIL', 'TOKEN_DIVISION', 'TOKEN_MULTIPLICATION', 'TOKEN_LOGICAL_NOT', 'TOKEN_INTEGER_LITERAL', 'TOKEN_RIGHT_BRACKET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool CALL_PARAM() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
            s = match(TOKEN_IDENTIFIER);
            if (!match(TOKEN_COLON)) {
                lookahead = TokenArray.prev();
            }
            s = s && call_expr_parser(lookahead);
            break;
        default:
            if (call_expr_parser(lookahead)) return true;
            fprintf(stderr, "Syntax error [CALL_PARAM_VALUE]: expected ['TOKEN_FALSE_LITERAL', 'TOKEN_LESS_THAN', 'TOKEN_LOGICAL_AND', 'TOKEN_LEFT_BRACKET', 'TOKEN_NIL_LITERAL', 'TOKEN_REAL_LITERAL', 'TOKEN_STRING_LITERAL', 'TOKEN_ADDITION', 'TOKEN_SUBTRACTION', 'TOKEN_LOGICAL_OR', 'TOKEN_IDENTIFIER', 'TOKEN_EQUAL_TO', 'TOKEN_LESS_THAN_OR_EQUAL_TO', 'TOKEN_GREATER_THAN', 'TOKEN_NOT_EQUAL_TO', 'TOKEN_GREATER_THAN_OR_EQUAL_TO', 'TOKEN_TRUE_LITERAL', 'TOKEN_IS_NIL', 'TOKEN_DIVISION', 'TOKEN_MULTIPLICATION', 'TOKEN_LOGICAL_NOT', 'TOKEN_UNWRAP_NILLABLE', 'TOKEN_INTEGER_LITERAL'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool NEXT_CALL_PARAM() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_COMMA:
            s = match(TOKEN_COMMA);
            s = s && CALL_PARAM();
            s = s && NEXT_CALL_PARAM();
            break;
        case TOKEN_RIGHT_BRACKET:
            s = true;
            break;
        default:
            fprintf(stderr, "Syntax error [NEXT_CALL_PARAM]: expected ['TOKEN_COMMA', 'TOKEN_RIGHT_BRACKET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool ID_CALL_OR_ASSIGN() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_IDENTIFIER:
            s = match(TOKEN_IDENTIFIER);
            s = s && NEXT_ID_CALL_OR_ASSIGN();
            break;
        default:
            fprintf(stderr, "Syntax error [ID_CALL_OR_ASSIGN]: expected ['TOKEN_IDENTIFIER'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool NEXT_ID_CALL_OR_ASSIGN() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_LEFT_BRACKET:
            s = match(TOKEN_LEFT_BRACKET);
            s = s && CALL_PARAM_LIST();
            s = s && match(TOKEN_RIGHT_BRACKET);
            break;
        case TOKEN_ASSIGNMENT:
            s = match(TOKEN_ASSIGNMENT);
            s = s && call_expr_parser(lookahead);
            break;
        default:
            fprintf(stderr, "Syntax error [NEXT_ID_CALL_OR_ASSIGN]: expected ['TOKEN_LEFT_BRACKET', 'TOKEN_ASSIGNMENT'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}


bool TYPE() {
    bool s;
    switch (lookahead->type) {
        case TOKEN_STRING_TYPE:
            s = match(TOKEN_STRING_TYPE);
            break;
        case TOKEN_INT_TYPE:
            s = match(TOKEN_INT_TYPE);
            break;
        case TOKEN_BOOL_TYPE:
            s = match(TOKEN_BOOL_TYPE);
            break;
        case TOKEN_DOUBLE_TYPE:
            s = match(TOKEN_DOUBLE_TYPE);
            break;
        default:
            fprintf(stderr, "Syntax error [EXPR]: expected ['TOKEN_IDENTIFIER', 'TOKEN_REAL_LITERAL', 'TOKEN_STRING_LITERAL', 'TOKEN_NIL_LITERAL', 'TOKEN_TRUE_LITERAL', 'TOKEN_FALSE_LITERAL', 'TOKEN_INTEGER_LITERAL', 'TOKEN_ADDITION', 'TOKEN_SUBTRACTION', 'TOKEN_MULTIPLICATION', 'TOKEN_DIVISION', 'TOKEN_LESS_THAN', 'TOKEN_LESS_THAN_OR_EQUAL_TO', 'TOKEN_GREATER_THAN', 'TOKEN_GREATER_THAN_OR_EQUAL_TO', 'TOKEN_EQUAL_TO', 'TOKEN_NOT_EQUAL_TO', 'TOKEN_IS_NIL', 'TOKEN_UNWRAP_NILLABLE', 'TOKEN_LOGICAL_AND', 'TOKEN_LOGICAL_OR', 'TOKEN_LOGICAL_NOT', 'TOKEN_LEFT_BRACKET'], got %s\n", tokens_as_str[lookahead->type]);
            exit(BAD_SYNTAX_ERR);
            s = false;
    }
    return s;
}