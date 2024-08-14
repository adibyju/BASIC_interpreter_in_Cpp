#pragma once

// Libraries
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>


extern std::string DIGITS;
extern std::string LETTERS;
extern std::string LETTERS_DIGITS;
extern std::string TT_INT;
extern std::string TT_FLOAT;
extern std::string TT_STRING;
extern std::string TT_IDENTIFIER;
extern std::string TT_KEYWORD;
extern std::string TT_PLUS;
extern std::string TT_MINUS;
extern std::string TT_MUL;
extern std::string TT_DIV;
extern std::string TT_POW;
extern std::string TT_EQ;
extern std::string TT_LPAREN;
extern std::string TT_RPAREN;
extern std::string TT_LSQUARE;
extern std::string TT_RSQUARE;
extern std::string TT_EE;
extern std::string TT_NE;
extern std::string TT_LT;
extern std::string TT_GT;
extern std::string TT_LTE;
extern std::string TT_GTE;
extern std::string TT_COMMA;
extern std::string TT_ARROW;
extern std::string TT_NEWLINE;
extern std::string TT_EOF;
extern std::vector<std::string> KEYWORDS;

class Node;
class Position;
class Context;
class Error;
class RTError;
class IllegalCharError;
class ExpectedCharError;
class InvalidSyntaxError;
class Token;
class Lexer;
class NumberNode;
class StringNode;
class VarAccessNode;
class VarAssignNode;
class BinOpNode;
class UnaryOpNode;
class IfNode;
class ForNode;
class WhileNode;
class FuncDefNode;
class CallNode;
class ParseResult;
class Parser;
class Number;
class Function;
class BaseFunction;
class BuiltInFunction;
class RTResult;
class SymbolTable;
class Interpreter;


// Node
class Node { // defined extra
public:
    virtual ~Node() = default;
    virtual void print(std::ostream& os) const = 0; // Pure virtual function
    virtual std::string get_class_name() const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Node& node) {
    node.print(os);
    return os;
}

// Position
class Position
{
public:    
    Position();
    Position(int idx, int ln, int col, std::string fn, std::string ftxt);
    Position advance(char current_char = '\0');
    Position copy();

    static Position none() {
        return Position(-1, -1, -1, "", "");
    }

    bool operator==(const Position& other) const;
    bool operator!=(const Position& other) const;

    int idx, ln, col;
    std::string fn, ftxt;
};

//Context
class Context 
{
public:
    Context();
    Context(std::string display_name, Context* parent = nullptr, Position parent_entry_pos = Position::none());

    std::string display_name;
    Context* parent;
    SymbolTable* symbol_table; 
    Position parent_entry_pos;
};

// Errors
class Error 
{
public:
    Error();
    Error(Position pos_start, Position pos_end, std::string error_name, std::string details, Context* context=nullptr);
    std::string is_error() const;
    std::string generate_traceback() const;
    std::string as_string() const;  

    Position pos_start;
    Position pos_end;
    std::string error_name, details;  
    Context* context;    
};

class RTError : public Error {
public:
    RTError(Position pos_start, Position pos_end, std::string details, Context* context);       
};

class IllegalCharError : public Error {
public:
    IllegalCharError(Position pos_start, Position pos_end, std::string details);
};

class ExpectedCharError : public Error {
public:
    ExpectedCharError(Position pos_start, Position pos_end, std::string details);
};

class InvalidSyntaxError : public Error {
public:
    InvalidSyntaxError(Position pos_start, Position pos_end, std::string details);
};


// Tokens
class Token : public Node{
public:    
    Token();
    Token(std::string type_, double value = DBL_MAX, std::string text="", Position pos_start = Position::none(), Position pos_end = Position::none(), bool is_none=0);
    bool matches(std::string type_, std::string text);
    std::string print() const;
    bool operator==(const Token& other) const; // defined extra

    void print(std::ostream& os) const override;
    std::string get_class_name() const override;

    std::string type_, text;
    double value;
    Position pos_start, pos_end;
    bool is_none;
};


// Lexer
class Lexer {
public:
    Lexer(std::string fn, std::string text);
    std::pair<std::vector<Token>, Error> make_tokens();
    void advance();
    Token make_number();
    Token make_string();
    Token make_identifier();
    Token make_minus_or_arrow();
    std::pair<Token, Error> make_not_equals();
    Token make_equals();
    Token make_less_than();
    Token make_greater_than();

private:   
    std::string fn, text;
    Position pos;
    char current_char;
};


// Nodes
class NumberNode : public Node
{
public:    
    NumberNode();
    NumberNode(Token tok);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const NumberNode& obj);

    Token tok;
    Position pos_start, pos_end;
};

class StringNode : public Node
{
public:
    StringNode();
    StringNode(Token tok);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const StringNode& obj);

    Token tok;
    Position pos_start, pos_end;
};

class ListNode : public Node
{
public:
    ListNode();
    ListNode(std::vector<std::shared_ptr<Node>> element_nodes, Position pos_start = Position::none(), Position pos_end = Position::none());
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const ListNode& obj);

    std::vector<std::shared_ptr<Node>> element_nodes;
    Position pos_start, pos_end;
};

class VarAccessNode : public Node
{
public:
    VarAccessNode(Token var_name_tok);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const NumberNode& obj);

    Token var_name_tok;
    Position pos_start, pos_end;
};

class VarAssignNode : public Node
{
public:
    VarAssignNode(Token var_name_tok, std::shared_ptr<Node> value_node);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const NumberNode& obj);

    Token var_name_tok;
    std::shared_ptr<Node> value_node;
    Position pos_start, pos_end;
};



class BinOpNode : public Node
{
public:    
    BinOpNode();
    BinOpNode(std::shared_ptr<Node> left_node, Token op_tok, std::shared_ptr<Node> right_node);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const BinOpNode& obj);

    std::shared_ptr<Node> left_node;
    Token op_tok;
    std::shared_ptr<Node> right_node;
    Position pos_start, pos_end;
};

class UnaryOpNode : public Node
{
public:
    UnaryOpNode(Token op_tok, std::shared_ptr<Node> node);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const UnaryOpNode& obj);

    Token op_tok;
    std::shared_ptr<Node> node;
    Position pos_start, pos_end;
};

class IfNode : public Node
{
public:
    IfNode(std::vector<std::vector<std::shared_ptr<Node>>> cases, std::shared_ptr<Node> else_case);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const IfNode& obj);

    std::vector<std::vector<std::shared_ptr<Node>>> cases;
    std::shared_ptr<Node> else_case;
    Position pos_start, pos_end;
};

class ForNode : public Node
{
public:
    ForNode(Token var_name_tok, std::shared_ptr<Node> start_value_node, std::shared_ptr<Node> end_value_node, std::shared_ptr<Node> step_value_node, std::shared_ptr<Node> body_node);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const ForNode& obj);

    Token var_name_tok;
    Position pos_start, pos_end;
    std::shared_ptr<Node> start_value_node, end_value_node, step_value_node, body_node;
};

class WhileNode : public Node
{
public:
    WhileNode(std::shared_ptr<Node> condition_node, std::shared_ptr<Node> body_node);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const WhileNode& obj);

    Position pos_start, pos_end;
    std::shared_ptr<Node> condition_node, body_node;
};

class FuncDefNode : public Node
{
public:
    FuncDefNode(Token var_name_tok, std::vector<Token> arg_name_toks, std::shared_ptr<Node> body_node);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const FuncDefNode& obj);

    Token var_name_tok;
    std::vector<Token> arg_name_toks;
    std::shared_ptr<Node> body_node;
    Position pos_start, pos_end;
};

class CallNode : public Node
{
public:
    CallNode(std::shared_ptr<Node> node_to_call, std::vector<std::shared_ptr<Node>> arg_nodes);
    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    friend std::ostream& operator<<(std::ostream& os, const CallNode& obj);


    std::shared_ptr<Node> node_to_call;
    std::vector<std::shared_ptr<Node>> arg_nodes;
    Position pos_start, pos_end;
};

// Parse Result
class ParseResult : public Node 
{
public:
    ParseResult();
    void register_advancement();
    std::shared_ptr<Node> register_result(std::shared_ptr<Node>);
    std::shared_ptr<Node> try_register_result(std::shared_ptr<Node>);
    ParseResult success(std::shared_ptr<Node> node);
    ParseResult failure(Error error);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    Error error;
    std::shared_ptr<Node> node;
    int advance_count, to_reverse_count, last_registered_advance_count;
};


// Parser
class Parser
{
public:
    Parser();
    Parser(std::vector<Token> tokens);
    Token advance();
    Token reverse(int amount=1);
    void update_current_tok();
    std::shared_ptr<Node> parse();
    std::shared_ptr<Node> statements();
    std::shared_ptr<Node> list_expr();
    std::shared_ptr<Node> if_expr();
    std::shared_ptr<Node> if_expr_b();
    std::shared_ptr<Node> if_expr_c();
    std::shared_ptr<Node> if_expr_b_or_c();
    std::shared_ptr<Node> if_expr_cases(std::string case_keyword);
    std::shared_ptr<Node> for_expr();
    std::shared_ptr<Node> while_expr();
    std::shared_ptr<Node> func_def();
    std::shared_ptr<Node> call();
    std::shared_ptr<Node> atom();
    std::shared_ptr<Node> power();
    std::shared_ptr<Node> factor();
    std::shared_ptr<Node> term();
    std::shared_ptr<Node> arith_expr();
    std::shared_ptr<Node> comp_expr();
    std::shared_ptr<Node> expr();
    std::shared_ptr<Node> bin_op(std::function<std::shared_ptr<Node>()> func_a, std::vector<std::string> ops, std::function<std::shared_ptr<Node>()> func_b=nullptr);

private:
    std::vector<Token> tokens;
    Token current_tok;
    int tok_idx;
};


// Values
class Number : public Node
{
public:
    Number();
    Number(double value, bool is_none=0);
    //Number(std::shared_ptr<Node> node);
    Number set_pos(Position pos_start = Position::none(), Position pos_end = Position::none());
    Number set_context(Context* context = nullptr);
    std::pair<Number, Error> added_to(std::shared_ptr<Node> other); // See this, its different in code
    std::pair<Number, Error> subbed_by(std::shared_ptr<Node> other);
    std::pair<Number, Error> multed_by(std::shared_ptr<Node> other);
    std::pair<Number, Error> dived_by(std::shared_ptr<Node> other);
    std::pair<Number, Error> powed_by(std::shared_ptr<Node> other);
    std::pair<Number, Error> get_comparison_eq(std::shared_ptr<Node> other);
    std::pair<Number, Error> get_comparison_ne(std::shared_ptr<Node> other);
    std::pair<Number, Error> get_comparison_lt(std::shared_ptr<Node> other);
    std::pair<Number, Error> get_comparison_gt(std::shared_ptr<Node> other);
    std::pair<Number, Error> get_comparison_lte(std::shared_ptr<Node> other);
    std::pair<Number, Error> get_comparison_gte(std::shared_ptr<Node> other);
    std::pair<Number, Error> anded_by(std::shared_ptr<Node> other);
    std::pair<Number, Error> ored_by(std::shared_ptr<Node> other);
    std::pair<Number, Error> notted();
    Number copy();
    bool is_true();

    friend std::ostream& operator<<(std::ostream& os, const Number& obj);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    double value;
    Position pos_start, pos_end;
    Context* context;
    bool is_none;

    static Number null_;
    static Number false_;
    static Number true_;
    static Number math_PI_;
};

class String : public Node {
public:
    String(std::string value);
    String set_pos(Position pos_start = Position::none(), Position pos_end = Position::none());
    String set_context(Context* context = nullptr);
    std::pair<String, Error> added_to(std::shared_ptr<Node> other);
    std::pair<String, Error> multed_by(std::shared_ptr<Node> other);
    String copy();
    bool is_true();

    friend std::ostream& operator<<(std::ostream& os, const Number& obj);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    std::string value;
    Position pos_start, pos_end;
    Context* context;
};

class List : public Node 
{
public:
    List();
    List(std::vector<std::shared_ptr<Node>> elements);
    std::pair<List, Error> added_to(std::shared_ptr<Node> other);
    std::pair<List, Error> subbed_by(std::shared_ptr<Node> other);
    std::pair<List, Error> multed_by(std::shared_ptr<Node> other);
    std::pair<std::shared_ptr<Node>, Error> dived_by(std::shared_ptr<Node> other);
    List copy();
    List set_pos(Position pos_start = Position::none(), Position pos_end = Position::none());
    List set_context(Context* context = nullptr);

    friend std::ostream& operator<<(std::ostream& os, const List& obj);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    std::vector<std::shared_ptr<Node>> elements;
    Context* context;
    Position pos_start, pos_end;
};

class BaseFunction : public Node
{
public:
    BaseFunction();
    BaseFunction(std::string name);
    BaseFunction set_pos(Position pos_start = Position::none(), Position pos_end = Position::none());
    BaseFunction set_context(Context* context = nullptr);
    Context* generate_new_context();
    RTResult check_args(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Node>> args);
    void populate_args(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Node>> args, Context* exec_ctx);
    RTResult check_and_populate_args(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Node>> args, Context* exec_ctx);

    friend std::ostream& operator<<(std::ostream& os, const Function& obj);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    std::string name;
    Context* context;
    Position pos_start, pos_end;
};

class Function : public BaseFunction
{
public:
    Function(std::string name, std::shared_ptr<Node> body_node, std::vector<std::string> arg_names);   
    Function copy();
    RTResult execute_result(std::vector<std::shared_ptr<Node>> args);

    friend std::ostream& operator<<(std::ostream& os, const Function& obj);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    std::shared_ptr<Node> body_node;
    std::vector<std::string> arg_names;
};

class BuiltInFunction : public BaseFunction
{
public:
    BuiltInFunction(std::string name);
    BuiltInFunction copy();
    RTResult execute_result(std::vector<std::shared_ptr<Node>> args);
    RTResult execute_print(Context* exec_ctx);
    RTResult execute_print_ret(Context* exec_ctx);
    RTResult execute_input(Context* exec_ctx);
    RTResult execute_input_int(Context* exec_ctx);
    RTResult execute_clear(Context* exec_ctx);
    RTResult execute_is_number(Context* exec_ctx);
    RTResult execute_is_string(Context* exec_ctx);
    RTResult execute_is_list(Context* exec_ctx);
    RTResult execute_is_function(Context* exec_ctx);
    RTResult execute_append(Context* exec_ctx);
    RTResult execute_pop(Context* exec_ctx);
    RTResult execute_extend(Context* exec_ctx);
    RTResult no_visit_method(Context* context);

    friend std::ostream& operator<<(std::ostream& os, const Function& obj);

    void print(std::ostream& os) const override; // Override print method
    std::string get_class_name() const override;

    static std::vector<std::string> execute_print_arg_names_;
    static std::vector<std::string> execute_print_ret_arg_names_;
    static std::vector<std::string> execute_input_arg_names_;
    static std::vector<std::string> execute_input_int_arg_names_;
    static std::vector<std::string> execute_clear_arg_names_;
    static std::vector<std::string> execute_is_number_arg_names_;
    static std::vector<std::string> execute_is_string_arg_names_;
    static std::vector<std::string> execute_is_list_arg_names_;
    static std::vector<std::string> execute_is_function_arg_names_;
    static std::vector<std::string> execute_append_arg_names_;
    static std::vector<std::string> execute_pop_arg_names_;
    static std::vector<std::string> execute_extend_arg_names_;

    static BuiltInFunction BuiltInFunction_print;
    static BuiltInFunction BuiltInFunction_print_ret;
    static BuiltInFunction BuiltInFunction_input;
    static BuiltInFunction BuiltInFunction_input_int;
    static BuiltInFunction BuiltInFunction_clear;
    static BuiltInFunction BuiltInFunction_is_number;
    static BuiltInFunction BuiltInFunction_is_string;
    static BuiltInFunction BuiltInFunction_is_list;
    static BuiltInFunction BuiltInFunction_is_function;
    static BuiltInFunction BuiltInFunction_append;
    static BuiltInFunction BuiltInFunction_pop;
    static BuiltInFunction BuiltInFunction_extend;
};

// Runtime Result
class RTResult
{
public:
    RTResult();
    std::shared_ptr<Node> register_result(RTResult res);
    RTResult success(std::shared_ptr<Node> value);
    RTResult failure(Error error);

    std::shared_ptr<Node> value;
    Error error;
};

// Symbol Table

class SymbolTable
{
public:
    SymbolTable(SymbolTable* parent=nullptr);
    std::shared_ptr<Node> get(std::string name);
    void set(std::string name, std::shared_ptr<Node> value);
    void remove(std::string name);

    std::unordered_map<std::string, std::shared_ptr<Node>> symbols;
    SymbolTable* parent;
};

// Interpreter
class Interpreter
{
public:
    Interpreter();
    RTResult visit(std::shared_ptr<Node> node, Context* context);
    RTResult no_visit_method(std::shared_ptr<Node> node, Context* context);
    RTResult visit_NumberNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_StringNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_ListNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_VarAccessNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_VarAssignNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_BinOpNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_UnaryOpNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_IfNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_ForNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_WhileNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_FuncDefNode(std::shared_ptr<Node> node, Context* context);
    RTResult visit_CallNode(std::shared_ptr<Node> node, Context* context);
};

// Run
extern SymbolTable global_symbol_table;

std::pair<std::shared_ptr<Node>, Error> run(std::string fn, std::string text);