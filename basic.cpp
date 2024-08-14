////////////////////////////
//////// LIBRARIES /////////
////////////////////////////
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <functional>
#include <memory>
#include <cmath>
#include <unordered_map>
#include "basic.h"
#include "string_with_arrows.h"


std::string DIGITS = "0123456789";
std::string LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string LETTERS_DIGITS = LETTERS + DIGITS;
std::vector<std::string> KEYWORDS = { "VAR", "AND", "OR", "NOT", "IF", "ELIF", "ELSE", "FOR", "TO", "STEP", "WHILE", "FUN", "THEN", "END"};

SymbolTable global_symbol_table = SymbolTable();

////////////////////////////
////////// ERRORS //////////
////////////////////////////

Error::Error() {}

Error::Error(Position pos_start, Position pos_end, std::string error_name, std::string details, Context* context)
	: pos_start(pos_start), pos_end(pos_end), error_name(error_name), details(details), context(context) {}

std::string Error::is_error() const {
    if(details == "None" || error_name == "") return "None";
    return error_name + ": " + details;
}

std::string Error::generate_traceback() const {
    std::string result = "";
    Position pos = pos_start;
    Context* ctx = context;
    //std::cout << "hi3: " << context->display_name << std::endl;

    while (ctx != nullptr) {
        std::cout << "hi4\n";
        //std::cout << "hi5: " << "  File " + pos.fn + ", line " + std::to_string(pos.ln + 1) + ", in " + ctx->display_name << std::endl;
        result = "  File " + pos.fn + ", line " + std::to_string(pos.ln + 1) + ", in " + ctx->display_name + "\n" + result;
        pos = ctx->parent_entry_pos;
        ctx = ctx->parent;
    }

    return "Traceback (most recent call last):\n" + result;
}

std::string Error::as_string() const {
    std::string result = "";
    std::cout<<"Error as_string() method called"<<std::endl;
    if (error_name == "Runtime Error") {
        std::cout<<"Runtime Error as_string() method called"<<std::endl;
        result = generate_traceback();
        result += error_name + ": " + details;
    }
    else {
        result = error_name + ": " + details;
        result += "\nFile " + pos_start.fn + ", line " + std::to_string(pos_start.ln + 1);
    }
    std::cout<<"Error as_string() method called"<<std::endl;    
    std::cout<< "pos_start.ftxt: " << pos_start.ftxt << ", pos_start: " << pos_start.col <<" pos_end:" << pos_end.col <<std::endl;
    result += "\n\n" + string_with_arrows(pos_start.ftxt, pos_start, pos_end);
    return result;
}

RTError::RTError(Position pos_start, Position pos_end, std::string details, Context* context)
    : Error(pos_start, pos_end, "Runtime Error", details, context) {}


//std::string RTError::as_string() const {
//    std::cout << "Runtime Error as_string() method called" << std::endl;
//    std::string result = generate_traceback();
//    std::cout<< "Generated traceback: " << result << std::endl;
//    result += error_name + ": " + details;
//    result += "\n\n" + string_with_arrows(pos_start.ftxt, pos_start, pos_end);
//    return result;
//}

IllegalCharError::IllegalCharError(Position pos_start, Position pos_end, std::string details)
	: Error(pos_start, pos_end, "Illegal Character", details) {}

ExpectedCharError::ExpectedCharError(Position pos_start, Position pos_end, std::string details)
	: Error(pos_start, pos_end, "Expected Character", details) {}

InvalidSyntaxError::InvalidSyntaxError(Position pos_start, Position pos_end, std::string details)
    : Error(pos_start, pos_end, "Invalid Syntax", details) {}


////////////////////////////
///////// POSITION /////////
////////////////////////////

Position::Position(){}

Position::Position(int idx, int ln, int col, std::string fn, std::string ftxt)
	: idx(idx), ln(ln), col(col), fn(fn) , ftxt(ftxt) {}

Position Position::advance(char current_char) {
	idx += 1;
	col += 1;
	if (current_char == '\n') {
		ln += 1;
		col = 0;
	}
	return *this;
}

Position Position::copy() {
	return Position(idx, ln, col, fn, ftxt);
}

bool Position::operator==(const Position& other) const {
    return idx == other.idx && ln == other.ln && col == other.col &&
        fn == other.fn && ftxt == other.ftxt;
}

bool Position::operator!=(const Position& other) const {
    return !(*this == other);
}

////////////////////////////
////////// TOKENS //////////
////////////////////////////

// TT = Token Type
std::string TT_INT = "TT_INT";
std::string TT_FLOAT = "TT_FLOAT";
std::string TT_STRING = "TT_STRING";
std::string TT_IDENTIFIER = "TT_IDENTIFIER";
std::string TT_KEYWORD = "TT_KEYWORD";
std::string TT_PLUS = "TT_PLUS";
std::string TT_MINUS = "TT_MINUS";
std::string TT_MUL = "TT_MUL";
std::string TT_DIV = "TT_DIV";
std::string TT_POW = "TT_POW";
std::string TT_EQ = "TT_EQ";
std::string TT_LPAREN = "TT_LPAREN";
std::string TT_RPAREN = "TT_RPAREN";
std::string TT_LSQUARE = "TT_LSQUARE";
std::string TT_RSQUARE = "TT_RSQUARE";
std::string TT_EE = "TT_EE";
std::string TT_NE = "TT_NE";
std::string TT_LT = "TT_LT";
std::string TT_GT = "TT_GT";
std::string TT_LTE = "TT_LTE";
std::string TT_GTE = "TT_GTE";
std::string TT_COMMA = "TT_COMMA";
std::string TT_ARROW = "TT_ARROW";
std::string TT_NEWLINE = "TT_NEWLINE";
std::string TT_EOF = "TT_EOF";

Token::Token() {}

Token::Token(std::string type_, double value, std::string text, Position pos_start, Position pos_end, bool is_none)
	: type_(type_), value(value), text(text), pos_start(pos_start), pos_end(pos_end), is_none(is_none) {
    if (pos_start != Position::none()) {
		pos_start = pos_start.copy();
		pos_end = pos_start.copy();
        pos_end.advance('\0');
	}

    if (pos_end != Position::none()) {
        pos_end = pos_end.copy();
    }
}

bool Token::matches(std::string type_, std::string text) {
    	return ((this->type_ == type_) && (this->text == text));
}
	
std::string Token::print() const {
	if (type_ == TT_INT) return "Token(" + type_ + ", " + std::to_string((int)value) + ")";
	else if (type_ == TT_FLOAT) return "Token(" + type_ + ", " + std::to_string(value) + ")";
    else if (type_ == TT_IDENTIFIER) return "Token(" + type_ + ", " + text + ")";
    else if (type_ == TT_KEYWORD) return "Token(" + type_ + ", " + text + ")";
    else if (type_ == TT_STRING) return "Token(" + type_ + ", " + text + ")";
	else return "Token(" + type_ + ")";
}

bool Token::operator==(const Token& other) const {
    if (type_ == TT_IDENTIFIER) {
        std::cout<< "Comparing identifiers: " << text << " and " << other.text << std::endl;
        return ((type_ == other.type_) && (text == other.text));
    } 
    else {
        std::cout<< "Comparing numbers: " << value << " and " << other.value << std::endl;
        return ((type_ == other.type_) && (value == other.value));
    }
}

void Token::print(std::ostream& os) const {
    //os << "(" << *left_node << "," << op_tok.print() << "," << *right_node << ")";
}

std::string Token::get_class_name() const {
    return "Token";
}



////////////////////////////
////////// LEXER ///////////
////////////////////////////

Lexer::Lexer(std::string fn, std::string text) : fn(fn), text(text) {
    pos = Position(-1, 0, -1, fn, text);
    current_char = '\0';
    advance();
}

void Lexer::advance() {
    pos.advance(current_char);
    current_char = (pos.idx < text.size()) ? text[pos.idx] : '\0';
}

std::pair<std::vector<Token>, Error> Lexer::make_tokens() {
    std::vector<Token> tokens;    

    while (current_char != '\0') {
        Position temp = pos.copy();
        temp.advance('\0');
        if (current_char == ' ' or current_char == '\t') {
            advance();
        }
        if (current_char == ';' or current_char == '\n') {
            tokens.push_back(Token(TT_NEWLINE, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (DIGITS.find(current_char) != std::string::npos) {
            tokens.push_back(make_number());
        }
        else if (LETTERS.find(current_char) != std::string::npos) {
            tokens.push_back(make_identifier());
        }
        else if (current_char == '"') {
            tokens.push_back(make_string());
        }
        else if (current_char == '+') {
            tokens.push_back(Token(TT_PLUS, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == '-') {
            tokens.push_back(make_minus_or_arrow());
        }
        else if (current_char == '*') {
            tokens.push_back(Token(TT_MUL, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == '/') {
            tokens.push_back(Token(TT_DIV, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == '^') {
            tokens.push_back(Token(TT_POW, DBL_MAX, "", pos, temp));
            advance();
        }        
        else if (current_char == '(') {
            tokens.push_back(Token(TT_LPAREN, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == ')') {
            tokens.push_back(Token(TT_RPAREN, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == '[') {
            tokens.push_back(Token(TT_LSQUARE, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == ']') {
            tokens.push_back(Token(TT_RSQUARE, DBL_MAX, "", pos, temp));
            advance();
        }
        else if (current_char == '!') {
            auto result = make_not_equals();
            if (result.second.is_error() != "None") return std::make_pair(std::vector<Token>(), result.second);
            tokens.push_back(result.first);
        }
        else if (current_char == '=') {
            tokens.push_back(make_equals());
        }
        else if (current_char == '<') {
			tokens.push_back(make_less_than());
		}
        else if (current_char == '>') {
			tokens.push_back(make_greater_than());
		}
        else if (current_char == ',') {
            tokens.push_back(Token(TT_COMMA, DBL_MAX, "", pos, temp));
            advance();
        }
        else {
            Position pos_start = pos.copy();
            char c = current_char;
            advance();
            return std::make_pair(std::vector<Token>(), IllegalCharError(pos_start, pos, "'" + std::to_string(c) + "'"));
        }
    }
    Position temp = pos.copy();
    temp.advance('\0');
    tokens.push_back(Token(TT_EOF, DBL_MAX, "", pos, temp));
    return std::make_pair(tokens, IllegalCharError(Position(-1, 0, -1, fn, text), Position(-1, 0, -1, fn, text), "None"));
}


Token Lexer::make_number() {
    std::string num_str = "";
    int dot_count = 0;
    Position pos_start = pos.copy();

    while (current_char != '\0' and ((DIGITS.find(current_char) != std::string::npos) or current_char == '.')) {
        if (current_char == '.') {
            if (dot_count == 1) break;
            dot_count += 1;
            num_str += '.';
        }
        else num_str += current_char;
        advance();
    }

    if (dot_count == 0) {
        return Token(TT_INT, stoi(num_str), "", pos_start, pos);
    }
    else {
        return Token(TT_FLOAT, stof(num_str), "", pos_start, pos);
    }
}

Token Lexer::make_string() {
    std::string string_val = "";
    Position pos_start = pos.copy();
    bool escape_character = false;
    advance();

    std::unordered_map<char, char> escape_characters;
    escape_characters['n'] = '\n';
    escape_characters['t'] = '\t';

    // Maybe a problem here
    while (current_char != '\0' and ((current_char != '"' or escape_character))) {
        if (escape_character) {
            if (escape_characters.find(current_char) != escape_characters.end()) {
                string_val += escape_characters[current_char];
            }
            else string_val += current_char;
        }
        else {
            if (current_char == '\\') {
                escape_character = true;
            }
            else string_val += current_char;
        }        
        advance();
        escape_character = false;
    }

    advance();
    return Token(TT_STRING, DBL_MAX, string_val, pos_start, pos);
}

Token Lexer::make_identifier() {
	std::string id_str = "";
	Position pos_start = pos.copy();

    while (current_char != '\0' and ((LETTERS_DIGITS.find(current_char) != std::string::npos) || current_char=='_')) {
		id_str += current_char;
		advance();
	}

    std:: string tok_type = (std::find(KEYWORDS.begin(), KEYWORDS.end(), id_str) != KEYWORDS.end()) ? TT_KEYWORD : TT_IDENTIFIER;

	return Token(tok_type, DBL_MAX, id_str, pos_start, pos);
}

Token Lexer::make_minus_or_arrow() {     
	Position pos_start = pos.copy();
	advance();

	if (current_char == '>') {
		advance();
		return Token(TT_ARROW, DBL_MAX, "", pos_start, pos);
	}
	return Token(TT_MINUS, DBL_MAX, "", pos_start, pos);

}

std::pair<Token, Error> Lexer::make_not_equals() {
	Position pos_start = pos.copy();
	advance();
    Error error;

    if (current_char == '=') {
		advance();
		return std::make_pair(Token(TT_NE, DBL_MAX, "", pos_start, pos), error);
	}
    advance();
	return std::make_pair(Token(TT_EOF), ExpectedCharError(pos_start, pos, "'=' (after '!')"));
}

Token Lexer::make_equals() {
	Position pos_start = pos.copy();
	advance();

    if (current_char == '=') {
		advance();
		return Token(TT_EE, DBL_MAX, "", pos_start, pos);
	}
	return Token(TT_EQ, DBL_MAX, "", pos_start, pos);
}

Token Lexer::make_less_than() {
	Position pos_start = pos.copy();
	advance();

    if (current_char == '=') {
		advance();
		return Token(TT_LTE, DBL_MAX, "", pos_start, pos);
	}
	return Token(TT_LT, DBL_MAX, "", pos_start, pos);
}

Token Lexer::make_greater_than() {
	Position pos_start = pos.copy();
	advance();

    if (current_char == '=') {
		advance();
		return Token(TT_GTE, DBL_MAX, "", pos_start, pos);
	}
	return Token(TT_GT, DBL_MAX, "", pos_start, pos);
}

////////////////////////////
////////// NODES ///////////
////////////////////////////

NumberNode::NumberNode() {}

NumberNode::NumberNode(Token tok)
	: tok(tok) {
    pos_start = tok.pos_start;
    pos_end = tok.pos_end;
}

void NumberNode::print(std::ostream& os) const {
    os << tok.print();
}

std::string NumberNode::get_class_name() const {
    return "NumberNode";
}

std::ostream& operator<<(std::ostream& os, const NumberNode& obj) {
    os << obj.tok.print();
    return os;
}

StringNode::StringNode() {}

StringNode::StringNode(Token tok)
    : tok(tok) {
    pos_start = tok.pos_start;
    pos_end = tok.pos_end;
}

void StringNode::print(std::ostream& os) const {
    os << tok.print();
}

std::string StringNode::get_class_name() const {
    return "StringNode";
}

std::ostream& operator<<(std::ostream& os, const StringNode& obj) {
    os << obj.tok.print();
    return os;
}

ListNode::ListNode() {}

ListNode::ListNode(std::vector<std::shared_ptr<Node>> element_nodes, Position pos_start, Position pos_end)
    : element_nodes(element_nodes), pos_start(pos_start), pos_end(pos_end) {}

void ListNode::print(std::ostream& os) const {
    os << "[";
    for (auto x : element_nodes) {
        if (x->get_class_name() == "NumberNode") {
            os << std::to_string(std::dynamic_pointer_cast<NumberNode>(x)->tok.value);
        }
        else if (x->get_class_name() == "StringNode") {
            os << std::dynamic_pointer_cast<StringNode>(x)->tok.text;
        }
        else if (x->get_class_name() == "ListNode") {
            std::dynamic_pointer_cast<ListNode>(x)->print(os);
        }
        os << ", ";
    }
    os << ']';
}

std::string ListNode::get_class_name() const {
    return "ListNode";
}

std::ostream& operator<<(std::ostream& os, const ListNode& obj) {
    os << "[";
    for (auto x : obj.element_nodes) {
        std::cout << x->get_class_name() << std::endl;
        if (x->get_class_name() == "NumberNode") {
            os << std::to_string(std::dynamic_pointer_cast<NumberNode>(x)->tok.value);
        }
        else if (x->get_class_name() == "StringNode") {
            os << std::dynamic_pointer_cast<StringNode>(x)->tok.text;
        }
        else if (x->get_class_name() == "ListNode") {
            os << std::dynamic_pointer_cast<ListNode>(x);
        }
        os << ", ";
    }
    os << ']';
    return os;
}

VarAccessNode::VarAccessNode(Token var_name_tok)
    : var_name_tok(var_name_tok) {
	pos_start = var_name_tok.pos_start;
	pos_end = var_name_tok.pos_end;
}

void VarAccessNode::print(std::ostream& os) const {
	os << var_name_tok.print();
}

std::string VarAccessNode::get_class_name() const {
	return "VarAccessNode";
}

std::ostream& operator<<(std::ostream& os, const VarAccessNode& obj) {
	os << obj.var_name_tok.print();
	return os;
}

////////////////////////////
VarAssignNode::VarAssignNode(Token var_name_tok, std::shared_ptr<Node> value_node)
    : var_name_tok(var_name_tok), value_node(value_node) {
	pos_start = var_name_tok.pos_start;
    if (value_node->get_class_name() == "NumberNode") {
        pos_end = std::dynamic_pointer_cast<NumberNode>(value_node)->pos_end;
    }
    else if (value_node->get_class_name() == "BinOpNode") {
        pos_end = std::dynamic_pointer_cast<BinOpNode>(value_node)->pos_end;
    }
    else if (value_node->get_class_name() == "UnaryOpNode") {
        pos_end = std::dynamic_pointer_cast<UnaryOpNode>(value_node)->pos_end;
    }
    else if (value_node->get_class_name() == "VarAccessNode") {
        pos_end = std::dynamic_pointer_cast<VarAccessNode>(value_node)->pos_end;
    }
    else if (value_node->get_class_name() == "VarAssignNode") {
        pos_end = std::dynamic_pointer_cast<VarAssignNode>(value_node)->pos_end;
    }
    else if (value_node->get_class_name() == "IfNode") {
		pos_end = std::dynamic_pointer_cast<IfNode>(value_node)->pos_end;
	}
    else if (value_node->get_class_name() == "ForNode") {
		pos_end = std::dynamic_pointer_cast<ForNode>(value_node)->pos_end;
	}
	else if (value_node->get_class_name() == "WhileNode") {
		pos_end = std::dynamic_pointer_cast<WhileNode>(value_node)->pos_end;
	}
    else if (value_node->get_class_name() == "FuncDefNode") {
        pos_end = std::dynamic_pointer_cast<FuncDefNode>(value_node)->pos_end;
    }   
    else if (value_node->get_class_name() == "CallNode") {
        pos_end = std::dynamic_pointer_cast<CallNode>(value_node)->pos_end;
    }
}

void VarAssignNode::print(std::ostream& os) const {
	os << "(" << var_name_tok.print() << "," << *value_node << ")";
}

std::string VarAssignNode::get_class_name() const {
	return "VarAssignNode";
}

std::ostream& operator<<(std::ostream& os, const VarAssignNode& obj) {
	os << "(" << obj.var_name_tok.print() << "," << *obj.value_node << ")";
	return os;
}

BinOpNode::BinOpNode(){};

////////////////////////////
BinOpNode::BinOpNode(std::shared_ptr<Node> left_node, Token op_tok, std::shared_ptr<Node> right_node)
    : left_node(left_node), op_tok(op_tok), right_node(right_node) {
    if (left_node->get_class_name() == "NumberNode") {
		pos_start = std::dynamic_pointer_cast<NumberNode>(left_node)->pos_start;
	}
    else if (left_node->get_class_name() == "BinOpNode"){
        pos_start = std::dynamic_pointer_cast<BinOpNode>(left_node)->pos_start;
	}
    else if (left_node->get_class_name() == "UnaryOpNode") {
        pos_start = std::dynamic_pointer_cast<UnaryOpNode>(left_node)->pos_start;
    }
    else if (left_node->get_class_name() == "VarAccessNode") {
        pos_start = std::dynamic_pointer_cast<VarAccessNode>(left_node)->pos_start;
    }
    else if (left_node->get_class_name() == "VarAssignNode") {
        pos_start = std::dynamic_pointer_cast<VarAssignNode>(left_node)->pos_start;
    }
    else if (left_node->get_class_name() == "IfNode") {
        pos_start = std::dynamic_pointer_cast<IfNode>(left_node)->pos_start;
    }
    else if (left_node->get_class_name() == "ForNode") {
		pos_start = std::dynamic_pointer_cast<ForNode>(left_node)->pos_start;
	}
	else if (left_node->get_class_name() == "WhileNode") {
		pos_start = std::dynamic_pointer_cast<WhileNode>(left_node)->pos_start;
	}
    else if (left_node->get_class_name() == "FuncDefNode") {
        pos_start = std::dynamic_pointer_cast<FuncDefNode>(left_node)->pos_start;
    }
    else if (left_node->get_class_name() == "CallNode") {
		pos_start = std::dynamic_pointer_cast<CallNode>(left_node)->pos_start;
	}

    if (right_node->get_class_name() == "NumberNode") {
        pos_end = std::dynamic_pointer_cast<NumberNode>(right_node)->pos_end;
    }
    else if (right_node->get_class_name() == "BinOpNode") {
        pos_end = std::dynamic_pointer_cast<BinOpNode>(right_node)->pos_end;
    }
    else if (right_node->get_class_name() == "UnaryOpNode") {
        pos_end = std::dynamic_pointer_cast<UnaryOpNode>(right_node)->pos_end;
    }
	else if (right_node->get_class_name() == "VarAccessNode") {
		pos_end = std::dynamic_pointer_cast<VarAccessNode>(right_node)->pos_end;
	}
	else if (right_node->get_class_name() == "VarAssignNode") {
		pos_end = std::dynamic_pointer_cast<VarAssignNode>(right_node)->pos_end;
	}
    else if (right_node->get_class_name() == "IfNode") {
		pos_end = std::dynamic_pointer_cast<IfNode>(right_node)->pos_end;
	}
    else if (right_node->get_class_name() == "ForNode") {
		pos_end = std::dynamic_pointer_cast<ForNode>(right_node)->pos_end;
	}
	else if (right_node->get_class_name() == "WhileNode") {
		pos_end = std::dynamic_pointer_cast<WhileNode>(right_node)->pos_end;
	}
    else if (right_node->get_class_name() == "FuncDefNode") {
		pos_end = std::dynamic_pointer_cast<FuncDefNode>(right_node)->pos_end;
	}
	else if (right_node->get_class_name() == "CallNode") {
        pos_end = std::dynamic_pointer_cast<CallNode>(right_node)->pos_end;
	}
}

void BinOpNode::print(std::ostream& os) const {
    os << "(" << *left_node << "," << op_tok.print() << "," << *right_node << ")";
}

std::string BinOpNode::get_class_name() const {
    return "BinOpNode";
}

std::ostream& operator<<(std::ostream& os, const BinOpNode& obj) {
    os << "(" << *obj.left_node << "," << obj.op_tok.print() << "," << *obj.right_node << ")";
    return os;
}

////////////////////////////
UnaryOpNode::UnaryOpNode(Token op_tok, std::shared_ptr<Node> node)
	: op_tok(op_tok), node(node) {
    pos_start = op_tok.pos_start;

    if (node->get_class_name() == "NumberNode") {
        pos_end = std::dynamic_pointer_cast<NumberNode>(node)->pos_end;
    }
    else if (node->get_class_name() == "BinOpNode") {
        pos_end = std::dynamic_pointer_cast<BinOpNode>(node)->pos_end;
    }
    else if (node->get_class_name() == "UnaryOpNode") {
        pos_end = std::dynamic_pointer_cast<UnaryOpNode>(node)->pos_end;
    }
    else if (node->get_class_name() == "VarAccessNode") {
        pos_end = std::dynamic_pointer_cast<VarAccessNode>(node)->pos_end;
    }
    else if (node->get_class_name() == "VarAssignNode") {
        pos_end = std::dynamic_pointer_cast<VarAssignNode>(node)->pos_end;
    }
    else if (node->get_class_name() == "IfNode") {
        pos_end = std::dynamic_pointer_cast<IfNode>(node)->pos_end;
    }
    else if (node->get_class_name() == "ForNode") {
		pos_end = std::dynamic_pointer_cast<ForNode>(node)->pos_end;
	}
	else if (node->get_class_name() == "WhileNode") {
		pos_end = std::dynamic_pointer_cast<WhileNode>(node)->pos_end;
	}
    else if (node->get_class_name() == "FuncDefNode") {
		pos_end = std::dynamic_pointer_cast<FuncDefNode>(node)->pos_end;
	}
	else if (node->get_class_name() == "CallNode") {
        pos_end = std::dynamic_pointer_cast<CallNode>(node)->pos_end;
	}
}

void UnaryOpNode::print(std::ostream& os) const {
	os << "(" << op_tok.print() << "," << *node << ")";
}

std::string UnaryOpNode::get_class_name() const {
    return "UnaryOpNode";
}

std::ostream& operator<<(std::ostream& os, const UnaryOpNode& obj) {
	os << "(" << obj.op_tok.print() << "," << *obj.node << ")";
	return os;
}

////////////////////////////
IfNode::IfNode(std::vector<std::vector<std::shared_ptr<Node>>> cases, std::shared_ptr<Node> else_case)
	: cases(cases), else_case(else_case) {
    if (cases[0][0]->get_class_name() == "NumberNode") {
        pos_start = std::dynamic_pointer_cast<NumberNode>(cases[0][0])->pos_start;
    }
    else if (cases[0][0]->get_class_name() == "BinOpNode") {
        pos_start = std::dynamic_pointer_cast<BinOpNode>(cases[0][0])->pos_start;
    }
    else if (cases[0][0]->get_class_name() == "UnaryOpNode") {
        pos_start = std::dynamic_pointer_cast<UnaryOpNode>(cases[0][0])->pos_start;
    }
    else if (cases[0][0]->get_class_name() == "VarAccessNode") {
        pos_start = std::dynamic_pointer_cast<VarAccessNode>(cases[0][0])->pos_start;
    }
    else if (cases[0][0]->get_class_name() == "VarAssignNode") {
        pos_start = std::dynamic_pointer_cast<VarAssignNode>(cases[0][0])->pos_start;
    }
    else if (cases[0][0]->get_class_name() == "IfNode") {
		pos_start = std::dynamic_pointer_cast<IfNode>(cases[0][0])->pos_start;
	}
    else if (cases[0][0]->get_class_name() == "ForNode") {
        pos_start = std::dynamic_pointer_cast<ForNode>(cases[0][0])->pos_start;
    }
    else if (cases[0][0]->get_class_name() == "WhileNode") {
		pos_start = std::dynamic_pointer_cast<WhileNode>(cases[0][0])->pos_start;
	}
    else if (cases[0][0]->get_class_name() == "FuncDefNode") {
		pos_start = std::dynamic_pointer_cast<FuncDefNode>(cases[0][0])->pos_start;
	}
	else if (cases[0][0]->get_class_name() == "CallNode") {
		pos_start = std::dynamic_pointer_cast<CallNode>(cases[0][0])->pos_start;
	}

    std::shared_ptr<Node> last_case = cases[cases.size() - 1][0];
    
    if (else_case != nullptr) last_case = else_case;

    if (last_case->get_class_name() == "NumberNode") {
        pos_end = std::dynamic_pointer_cast<NumberNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "BinOpNode") {
        pos_end = std::dynamic_pointer_cast<BinOpNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "UnaryOpNode") {
        pos_end = std::dynamic_pointer_cast<UnaryOpNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "VarAccessNode") {
        pos_end = std::dynamic_pointer_cast<VarAccessNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "VarAssignNode") {
        pos_end = std::dynamic_pointer_cast<VarAssignNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "IfNode") {
        pos_end = std::dynamic_pointer_cast<IfNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "ForNode") {
		pos_end = std::dynamic_pointer_cast<ForNode>(last_case)->pos_end;
	}
    else if (last_case->get_class_name() == "WhileNode") {
        pos_end = std::dynamic_pointer_cast<WhileNode>(last_case)->pos_end;	
    }
    else if (last_case->get_class_name() == "FuncDefNode") {
        pos_end = std::dynamic_pointer_cast<FuncDefNode>(last_case)->pos_end;
    }
    else if (last_case->get_class_name() == "CallNode") {
		pos_end = std::dynamic_pointer_cast<CallNode>(last_case)->pos_end;
	}
}

void IfNode::print(std::ostream& os) const {
	os << "IfNode(";
    for (auto case_ : cases) {
		os << "(" << *case_[0] << "," << *case_[1] << ")";
	}
    if (else_case != nullptr) {
		os << "Else: " << *else_case;
	}
	os << ")";
}

std::string IfNode::get_class_name() const {
	return "IfNode";
}

std::ostream& operator<<(std::ostream& os, const IfNode& obj) {
	os << "IfNode(";
    for (auto case_ : obj.cases) {
		os << "(" << *case_[0] << "," << *case_[1] << ")";
	}
    if (obj.else_case != nullptr) {
		os << "Else: " << *obj.else_case;
	}
	os << ")";
	return os;
}

////////////////////////////
ForNode::ForNode(Token var_name_tok, std::shared_ptr<Node> start_value_node, std::shared_ptr<Node> end_value_node, std::shared_ptr<Node> step_value_node, std::shared_ptr<Node> body_node)
    : var_name_tok(var_name_tok), start_value_node(start_value_node), end_value_node(end_value_node), step_value_node(step_value_node), body_node(body_node) {
    pos_start = var_name_tok.pos_start;

    if (body_node->get_class_name() == "NumberNode") {
        pos_end = std::dynamic_pointer_cast<NumberNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "BinOpNode") {
        pos_end = std::dynamic_pointer_cast<BinOpNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "UnaryOpNode") {
        pos_end = std::dynamic_pointer_cast<UnaryOpNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "VarAccessNode") {
        pos_end = std::dynamic_pointer_cast<VarAccessNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "VarAssignNode") {
        pos_end = std::dynamic_pointer_cast<VarAssignNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "IfNode") {
        pos_end = std::dynamic_pointer_cast<IfNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "ForNode") {
        pos_end = std::dynamic_pointer_cast<ForNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "WhileNode") {
        pos_end = std::dynamic_pointer_cast<WhileNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "FuncDefNode") {
		pos_end = std::dynamic_pointer_cast<FuncDefNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "CallNode") {
		pos_end = std::dynamic_pointer_cast<CallNode>(body_node)->pos_end;
	}
}

void ForNode::print(std::ostream& os) const {
	os << "ForNode(" << var_name_tok.print() << "," << *start_value_node << "," << *end_value_node;
    if (step_value_node != nullptr) os << "," << *step_value_node;
    os << "," << *body_node << ")";
}

std::string ForNode::get_class_name() const {
	return "ForNode";
}

std::ostream& operator<<(std::ostream& os, const ForNode& obj) {
	os << "ForNode(" << obj.var_name_tok.print() << "," << *obj.start_value_node << "," << *obj.end_value_node << "," << *obj.step_value_node << "," << *obj.body_node << ")";
	return os;
}

////////////////////////////
WhileNode::WhileNode(std::shared_ptr<Node> condition_node, std::shared_ptr<Node> body_node)
    : condition_node(condition_node), body_node(body_node) {
    if (condition_node->get_class_name() == "NumberNode") {
        pos_start = std::dynamic_pointer_cast<NumberNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "BinOpNode") {
        pos_start = std::dynamic_pointer_cast<BinOpNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "UnaryOpNode") {
        pos_start = std::dynamic_pointer_cast<UnaryOpNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "VarAccessNode") {
        pos_start = std::dynamic_pointer_cast<VarAccessNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "VarAssignNode") {
        pos_start = std::dynamic_pointer_cast<VarAssignNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "IfNode") {
        pos_start = std::dynamic_pointer_cast<IfNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "ForNode") {
        pos_start = std::dynamic_pointer_cast<ForNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "WhileNode") {
        pos_start = std::dynamic_pointer_cast<WhileNode>(condition_node)->pos_start;
    }
    else if (condition_node->get_class_name() == "FuncDefNode") {
		pos_start = std::dynamic_pointer_cast<FuncDefNode>(condition_node)->pos_start;
	}
	else if (condition_node->get_class_name() == "CallNode") {
		pos_start = std::dynamic_pointer_cast<CallNode>(condition_node)->pos_start;
	}

    if (body_node->get_class_name() == "NumberNode") {
		pos_end = std::dynamic_pointer_cast<NumberNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "BinOpNode") {
		pos_end = std::dynamic_pointer_cast<BinOpNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "UnaryOpNode") {
		pos_end = std::dynamic_pointer_cast<UnaryOpNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "VarAccessNode") {
		pos_end = std::dynamic_pointer_cast<VarAccessNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "VarAssignNode") {
		pos_end = std::dynamic_pointer_cast<VarAssignNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "IfNode") {
		pos_end = std::dynamic_pointer_cast<IfNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "ForNode") {
		pos_end = std::dynamic_pointer_cast<ForNode>(body_node)->pos_end;
	}
	else if (body_node->get_class_name() == "WhileNode") {
		pos_end = std::dynamic_pointer_cast<WhileNode>(body_node)->pos_end;
	}
    else if (body_node->get_class_name() == "FuncDefNode") {
        pos_end = std::dynamic_pointer_cast<FuncDefNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "CallNode") {
		pos_end = std::dynamic_pointer_cast<CallNode>(body_node)->pos_end;
	}
}

void WhileNode::print(std::ostream& os) const {
	os << "WhileNode(" << *condition_node << "," << *body_node << ")";
}

std::string WhileNode::get_class_name() const {
	return "WhileNode";
}

std::ostream& operator<<(std::ostream& os, const WhileNode& obj) {
	os << "WhileNode(" << *obj.condition_node << "," << *obj.body_node << ")";
	return os;
}

////////////////////////////
FuncDefNode::FuncDefNode(Token var_name_tok, std::vector<Token> arg_name_toks, std::shared_ptr<Node> body_node)
    : var_name_tok(var_name_tok), arg_name_toks(arg_name_toks), body_node(body_node) {
    if (!var_name_tok.is_none) {
        pos_start = var_name_tok.pos_start;
    }
    else if (arg_name_toks.size() > 0) {
        pos_start = arg_name_toks[0].pos_start;
    }
    else {
        if (body_node->get_class_name() == "NumberNode") {
            pos_start = std::dynamic_pointer_cast<NumberNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "BinOpNode") {
            pos_start = std::dynamic_pointer_cast<BinOpNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "UnaryOpNode") {
            pos_start = std::dynamic_pointer_cast<UnaryOpNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "VarAccessNode") {
            pos_start = std::dynamic_pointer_cast<VarAccessNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "VarAssignNode") {
            pos_start = std::dynamic_pointer_cast<VarAssignNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "IfNode") {
            pos_start = std::dynamic_pointer_cast<IfNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "ForNode") {
            pos_start = std::dynamic_pointer_cast<ForNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "WhileNode") {
            pos_start = std::dynamic_pointer_cast<WhileNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "FuncDefNode") {
            pos_start = std::dynamic_pointer_cast<FuncDefNode>(body_node)->pos_start;
        }
        else if (body_node->get_class_name() == "CallNode") {
            pos_start = std::dynamic_pointer_cast<CallNode>(body_node)->pos_start;
        }
    }

    if (body_node->get_class_name() == "NumberNode") {
        pos_end = std::dynamic_pointer_cast<NumberNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "BinOpNode") {
        pos_end = std::dynamic_pointer_cast<BinOpNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "UnaryOpNode") {
        pos_end = std::dynamic_pointer_cast<UnaryOpNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "VarAccessNode") {
        pos_end = std::dynamic_pointer_cast<VarAccessNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "VarAssignNode") {
        pos_end = std::dynamic_pointer_cast<VarAssignNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "IfNode") {
        pos_end = std::dynamic_pointer_cast<IfNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "ForNode") {
        pos_end = std::dynamic_pointer_cast<ForNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "WhileNode") {
        pos_end = std::dynamic_pointer_cast<WhileNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "FuncDefNode") {
        pos_end = std::dynamic_pointer_cast<FuncDefNode>(body_node)->pos_end;
    }
    else if (body_node->get_class_name() == "CallNode") {
        pos_end = std::dynamic_pointer_cast<CallNode>(body_node)->pos_end;
    }

    // temporary
    if (!var_name_tok.is_none) std::cout << "<function " << var_name_tok.text << ">" << std::endl;
    else std::cout << "<function <anonymous>>\n";
}

void FuncDefNode::print(std::ostream& os) const {
	os << "FuncDefNode(" << var_name_tok.print() << ", [";
	for (auto arg_name_tok : arg_name_toks) {
		os << arg_name_tok.print() << ", ";
	}
	os << "], " << *body_node << ")";
}

std::string FuncDefNode::get_class_name() const {
	return "FuncDefNode";
}

std::ostream& operator<<(std::ostream& os, const FuncDefNode& obj) {
	os << "FuncDefNode(" << obj.var_name_tok.print() << ", [";
	for (auto arg_name_tok : obj.arg_name_toks) {
		os << arg_name_tok.print() << ", ";
	}
	os << "], " << *obj.body_node << ")";
	return os;
}

////////////////////////////
CallNode::CallNode(std::shared_ptr<Node> node_to_call, std::vector<std::shared_ptr<Node>> arg_nodes)
    : node_to_call(node_to_call), arg_nodes(arg_nodes) {
    if (node_to_call->get_class_name() == "NumberNode") {
        pos_start = std::dynamic_pointer_cast<NumberNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "BinOpNode") {
        pos_start = std::dynamic_pointer_cast<BinOpNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "UnaryOpNode") {
        pos_start = std::dynamic_pointer_cast<UnaryOpNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "VarAccessNode") {
        pos_start = std::dynamic_pointer_cast<VarAccessNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "VarAssignNode") {
        pos_start = std::dynamic_pointer_cast<VarAssignNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "IfNode") {
        pos_start = std::dynamic_pointer_cast<IfNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "ForNode") {
        pos_start = std::dynamic_pointer_cast<ForNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "WhileNode") {
        pos_start = std::dynamic_pointer_cast<WhileNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "FuncDefNode") {
        pos_start = std::dynamic_pointer_cast<FuncDefNode>(node_to_call)->pos_start;
    }
    else if (node_to_call->get_class_name() == "CallNode") {
        pos_start = std::dynamic_pointer_cast<CallNode>(node_to_call)->pos_start;
    }

    if (arg_nodes.size() > 0) {
        std::shared_ptr<Node> temp = arg_nodes[arg_nodes.size() - 1];
        if (temp->get_class_name() == "NumberNode") {
            pos_end = std::dynamic_pointer_cast<NumberNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "BinOpNode") {
            pos_end = std::dynamic_pointer_cast<BinOpNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "UnaryOpNode") {
            pos_end = std::dynamic_pointer_cast<UnaryOpNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "VarAccessNode") {
            pos_end = std::dynamic_pointer_cast<VarAccessNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "VarAssignNode") {
            pos_end = std::dynamic_pointer_cast<VarAssignNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "IfNode") {
            pos_end = std::dynamic_pointer_cast<IfNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "ForNode") {
            pos_end = std::dynamic_pointer_cast<ForNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "WhileNode") {
            pos_end = std::dynamic_pointer_cast<WhileNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "FuncDefNode") {
            pos_end = std::dynamic_pointer_cast<FuncDefNode>(temp)->pos_end;
        }
        else if (temp->get_class_name() == "CallNode") {
            pos_end = std::dynamic_pointer_cast<CallNode>(temp)->pos_end;
        }
    }
    else {
        if (node_to_call->get_class_name() == "NumberNode") {
            pos_end = std::dynamic_pointer_cast<NumberNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "BinOpNode") {
            pos_end = std::dynamic_pointer_cast<BinOpNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "UnaryOpNode") {
            pos_end = std::dynamic_pointer_cast<UnaryOpNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "VarAccessNode") {
            pos_end = std::dynamic_pointer_cast<VarAccessNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "VarAssignNode") {
            pos_end = std::dynamic_pointer_cast<VarAssignNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "IfNode") {
            pos_end = std::dynamic_pointer_cast<IfNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "ForNode") {
            pos_end = std::dynamic_pointer_cast<ForNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "WhileNode") {
            pos_end = std::dynamic_pointer_cast<WhileNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "FuncDefNode") {
            pos_end = std::dynamic_pointer_cast<FuncDefNode>(node_to_call)->pos_end;
        }
        else if (node_to_call->get_class_name() == "CallNode") {
            pos_end = std::dynamic_pointer_cast<CallNode>(node_to_call)->pos_end;
        }
    }
}

void CallNode::print(std::ostream& os) const {
	os << "CallNode(" << *node_to_call << ", [";
	for (auto arg_node : arg_nodes) {
		os << *arg_node << ", ";
	}
	os << "])";
}

std::string CallNode::get_class_name() const {
	return "CallNode";
}

std::ostream& operator<<(std::ostream& os, const CallNode& obj) {
	os << "CallNode(" << *obj.node_to_call << ", [";
	for (auto arg_node : obj.arg_nodes) {
		os << *arg_node << ", ";
	}
	os << "])";
	return os;
}       

////////////////////////////
/////// PARSE RESULT ///////
////////////////////////////

ParseResult::ParseResult() {
    advance_count = 0;
    to_reverse_count = 0;
    last_registered_advance_count = 0;
}

std::string ParseResult::get_class_name() const {
    return "ParseResult";
}

void ParseResult::register_advancement() {
    last_registered_advance_count = 1;
    advance_count+=1;
}

std::shared_ptr<Node> ParseResult::register_result(std::shared_ptr<Node> res) {
    auto parse_result = std::dynamic_pointer_cast<ParseResult>(res);
    last_registered_advance_count = parse_result->advance_count;
    advance_count += parse_result->advance_count;
    if (parse_result->error.is_error() != "None") {
        error = parse_result->error;
    }
    return parse_result->node;
}

std::shared_ptr<Node> ParseResult::try_register_result(std::shared_ptr<Node> res) {
    if (std::dynamic_pointer_cast<ParseResult>(res)->error.is_error() != "None") {
        to_reverse_count = std::dynamic_pointer_cast<ParseResult>(res)->advance_count;
        return std::make_shared<Number>(Number(0, 1));
    }
    return register_result(res);
}

ParseResult ParseResult::success(std::shared_ptr<Node> node) {
    this->node = node;
    return *this;
}

ParseResult ParseResult::failure(Error error) {
    if (this->error.is_error() == "None" || this->last_registered_advance_count == 0) this->error = error;
    return *this;
}

void ParseResult::print(std::ostream& os) const {
    //os << "(" << *left_node << "," << op_tok.print() << "," << *right_node << ")";
}

////////////////////////////
///// RUNTIME RESULT ///////
////////////////////////////

RTResult::RTResult() {}

std::shared_ptr<Node> RTResult::register_result(RTResult res) {
    if (res.error.is_error() != "None") {
        error = res.error;
    }
    return res.value;
}

RTResult RTResult::success(std::shared_ptr<Node> value) {
    this->value = value;
    return *this;
}

RTResult RTResult::failure(Error error) {
    this->error = error;
    std::cout<<"Context Error in RTResult: "<<this->error.context->display_name<<std::endl;
    return *this;
}

////////////////////////////
////////// PARSER //////////
////////////////////////////

Parser::Parser(){}

Parser::Parser(std::vector<Token> tokens)
	: tokens(tokens) {
    tok_idx = -1;
	advance();
}

Token Parser::advance() {
	tok_idx += 1;
    update_current_tok();
    std::cout<<"current_tok: "<< current_tok.print() << std::endl;
    std::cout<<"current_tok info: "<<current_tok.pos_start.col << ", " << current_tok.pos_end.col << std::endl;
    return current_tok;
}

Token Parser::reverse(int amount) {
    tok_idx -= amount;
    update_current_tok();
    std::cout << "current_tok in reverse: " << current_tok.print() << std::endl;
    std::cout << "current_tok info(reverse): " << current_tok.pos_start.col << ", " << current_tok.pos_end.col << std::endl;
    return current_tok;
}

void Parser::update_current_tok() {
    if (tok_idx >= 0 && tok_idx < tokens.size()) {
        current_tok = tokens[tok_idx];
    }
}

std::shared_ptr<Node> Parser::parse() {
    std::cout << "Pk-entering parse\n";
    std::shared_ptr<Node> res = statements();
    std::cout << "Parse error result: " << std::dynamic_pointer_cast<ParseResult>(res)->error.is_error() << std::endl;

    if (std::dynamic_pointer_cast<ParseResult>(res)->error.is_error() == "None" and current_tok.type_ != TT_EOF) {
        std::cout<< "Parse error result: " << std::dynamic_pointer_cast<ParseResult>(res)->error.is_error() << ", current_tok: "<<current_tok.type_<<std::endl;
        std::cout<< "Pos_start: " << current_tok.pos_start.ln << ", " << current_tok.pos_start.col << std::endl;
        std::cout<< "Pos_end: " << current_tok.pos_end.ln << ", " << current_tok.pos_end.col << std::endl;
		return std::make_shared<ParseResult>(std::dynamic_pointer_cast<ParseResult>(res)->failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '+', '-', '*' or '/'")));
	}

    return res;
}

std::shared_ptr<Node> Parser::statements() {
    ParseResult res = ParseResult();
    std::vector<std::shared_ptr<Node>> statements;
    Position pos_start = current_tok.pos_start.copy();

    while (current_tok.type_ == TT_NEWLINE) {
        res.register_advancement(); advance();
    }

    std::shared_ptr<Node> statement = res.register_result(expr());
    if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
    statements.push_back(statement);

    bool more_statements = true;

    while (true) {
        int newline_count = 0;        
        while (current_tok.type_ == TT_NEWLINE) {
            res.register_advancement(); advance();
            newline_count += 1;
        }
        if (newline_count == 0) more_statements = false;

        if (!more_statements) break;

        statement = res.try_register_result(expr());
        if (!statement || (statement->get_class_name() == "Number" && std::dynamic_pointer_cast<Number>(statement)->is_none)) {
            reverse(res.to_reverse_count);
            more_statements = false;
            continue;
        }
        statements.push_back(statement);
    }

    return std::make_shared<ParseResult>(res.success(std::make_shared<ListNode>(ListNode(statements, pos_start, current_tok.pos_end.copy()))));
}

std::shared_ptr<Node> Parser::list_expr() {
    std::cout << "Pk-entering list_expr\n";
    ParseResult res = ParseResult();
    std::vector<std::shared_ptr<Node>> element_nodes;
    Position pos_start = current_tok.pos_start.copy();

    if (current_tok.type_ != TT_LSQUARE) {
        return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '['")));
    }

    res.register_advancement(); advance();

    if (current_tok.type_ == TT_RSQUARE) {
        res.register_advancement(); advance();
    }
    else {
        element_nodes.push_back(res.register_result(expr()));
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected ']', 'VAR', 'IF', 'FOR', 'WHILE', 'FUN', int, float, identifier, '+', '-', '(', '[' or 'NOT'")));

        while (current_tok.type_ == TT_COMMA) {
            res.register_advancement(); advance();
            element_nodes.push_back(res.register_result(expr()));
            if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        }

        if (current_tok.type_ != TT_RSQUARE) {
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected ',' or ']'")));
        }

        res.register_advancement(); advance();
    }

    return std::make_shared<ParseResult>(res.success(std::make_shared<ListNode>(ListNode(element_nodes, pos_start, current_tok.pos_end.copy()))));
}

std::shared_ptr<Node> Parser::if_expr() {
    ParseResult res;
    auto all_cases = res.register_result(if_expr_cases("IF"));
    if (res.error.is_error() != "None") {
        return std::make_shared<ParseResult>(res);
    }
    auto cases = std::dynamic_pointer_cast<std::vector<std::shared_ptr<Node>>>(all_cases->cases);
    auto else_case = std::dynamic_pointer_cast<std::shared_ptr<Node>>(all_cases->else_case);
    return std::make_shared<ParseResult>(res.success(std::make_shared<IfNode>(IfNode(cases, else_case))));
}

std::shared_ptr<Node> Parser::if_expr_b() {
    return if_expr_cases("ELIF");
}

std::shared_ptr<Node> Parser::if_expr_c() {
    ParseResult res;
    std::shared_ptr<Node> else_case = nullptr;

    if (current_tok.matches(TT_KEYWORD, "ELSE")) {
        res.register_advancement();
        advance();

        if (current_tok.type_ == TT_NEWLINE) {
            res.register_advancement();
            advance();

            auto statements = res.register_result(statements());
            if (res.error.is_error() != "None") {
                return std::make_shared<ParseResult>(res);
            }
            else_case = std::make_shared<TupleNode>(statements, std::make_shared<BoolNode>(true));

            if (current_tok.matches(TT_KEYWORD, "END")) {
                res.register_advancement();
                advance();
            }
            else {
                return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'END'")));
            }
        }
        else {
            auto expr = res.register_result(expr());
            if (res.error.is_error() != "None") {
                return std::make_shared<ParseResult>(res);
            }
            else_case = std::make_shared<TupleNode>(expr, std::make_shared<BoolNode>(false));
        }
    }

    return res.success(else_case);
}

std::shared_ptr<Node> Parser::if_expr_b_or_c() {
    ParseResult res;
    std::vector<std::shared_ptr<Node>> cases;
    std::shared_ptr<Node> else_case = nullptr;

    if (current_tok.matches(TT_KEYWORD, "ELIF")) {
        auto all_cases = res.register_result(if_expr_b());
        if (res.error.is_error() != "None") {
            return std::make_shared<ParseResult>(res);
        }
        cases = std::dynamic_pointer_cast<std::vector<std::shared_ptr<Node>>>(all_cases->cases);
        else_case = std::dynamic_pointer_cast<std::shared_ptr<Node>>(all_cases->else_case);
    }
    else {
        else_case = res.register_result(if_expr_c());
        if (res.error.is_error() != "None") {
            return std::make_shared<ParseResult>(res);
        }
    }

    return res.success(std::make_shared<TupleNode>(cases, else_case));
}

std::shared_ptr<Node> Parser::if_expr_cases(const std::string& case_keyword) {
    ParseResult res;
    std::vector<std::shared_ptr<Node>> cases;
    std::shared_ptr<Node> else_case = nullptr;

    if (!current_tok.matches(TT_KEYWORD, case_keyword)) {
        return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '" + case_keyword + "'")));
    }

    res.register_advancement(); advance();

    std::shared_ptr<Node> condition = res.register_result(expr());
    if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

    if (!current_tok.matches(TT_KEYWORD, "THEN")) {
        return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'THEN'")));
    }

    res.register_advancement(); advance();

    if (current_tok.type_ == TT_NEWLINE) {
        res.register_advancement(); advance();

        std::shared_ptr<Node> statements_result = res.register_result(statements());
        if (res.error.is_error() != "None") {
            return std::make_shared<ParseResult>(res);
        }
        cases.push_back(std::make_shared<TupleNode>(condition, statements_result, std::make_shared<BoolNode>(true)));

        if (current_tok.matches(TT_KEYWORD, "END")) {
            res.register_advancement(); advance();
        }
        else {
            std::shared_ptr<Node> all_cases = res.register_result(if_expr_b_or_c());
            if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
            auto new_cases = std::dynamic_pointer_cast<std::vector<std::shared_ptr<Node>>>(all_cases->cases);
            else_case = all_cases->else_case;
            cases.insert(cases.end(), new_cases->begin(), new_cases->end());
        }
    }
    else {
        auto expr = res.register_result(expr());
        if (res.error.is_error() != "None") {
            return std::make_shared<ParseResult>(res);
        }
        cases.push_back(std::make_shared<TupleNode>(condition, expr, std::make_shared<BoolNode>(false)));

        auto all_cases = res.register_result(if_expr_b_or_c());
        if (res.error.is_error() != "None") {
            return std::make_shared<ParseResult>(res);
        }
        auto new_cases = std::dynamic_pointer_cast<std::vector<std::shared_ptr<Node>>>(all_cases->cases);
        else_case = std::dynamic_pointer_cast<std::shared_ptr<Node>>(all_cases->else_case);
        cases.insert(cases.end(), new_cases->begin(), new_cases->end());
    }

    return res.success(std::make_shared<TupleNode>(cases, else_case));
}






//std::shared_ptr<Node> Parser::if_expr() {
//	std::cout << "Pk-entering if_expr\n";
//	ParseResult res = ParseResult();
//    std::vector<std::vector<std::shared_ptr<Node>>> cases;
//
//    if (!current_tok.matches(TT_KEYWORD, "IF")) {
//		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'IF'")));
//	}
//
//    res.register_advancement(); advance();
//
//    std::shared_ptr<Node> condition = res.register_result(expr());
//    if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
//
//    if (!current_tok.matches(TT_KEYWORD, "THEN")) {
//        return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'THEN'")));
//    }
//
//    res.register_advancement(); advance();
//    
//    std::shared_ptr<Node> expression = res.register_result(expr());
//    if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
//
//    cases.push_back({condition, expression});
//
//    while (current_tok.matches(TT_KEYWORD, "ELIF")) {
//		res.register_advancement(); advance();
//		std::shared_ptr<Node> condition = res.register_result(expr());
//		if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
//
//        if (!current_tok.matches(TT_KEYWORD, "THEN")) {
//			return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'THEN'")));
//		}
//
//		res.register_advancement(); advance();
//		
//		std::shared_ptr<Node> expression = res.register_result(expr());
//		if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
//
//		cases.push_back({condition, expression});
//	}
//
//    std::shared_ptr<Node> else_case = nullptr;
//    if (current_tok.matches(TT_KEYWORD, "ELSE")) {
//		res.register_advancement(); advance();
//        else_case = res.register_result(expr());
//		if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
//	}
//
//	return std::make_shared<ParseResult>(res.success(std::make_shared<IfNode>(IfNode(cases, else_case))));
//}

std::shared_ptr<Node> Parser::for_expr() {
    std::cout << "Pk-entering for_expr\n";
	ParseResult res = ParseResult();	

	if (!current_tok.matches(TT_KEYWORD, "FOR")) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'FOR'")));
	}

	res.register_advancement(); advance();

	if (current_tok.type_ != TT_IDENTIFIER) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected identifier")));
	}

    Token var_name_tok = current_tok;
	res.register_advancement(); advance();

	if (current_tok.type_ != TT_EQ) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '='")));
	}

	res.register_advancement(); advance();

	std::shared_ptr<Node> start_value = res.register_result(expr());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	if (!current_tok.matches(TT_KEYWORD, "TO")) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'TO'")));
	}

	res.register_advancement(); advance();

	std::shared_ptr<Node> end_value = res.register_result(expr());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	std::shared_ptr<Node> step_value = nullptr;
	if (current_tok.matches(TT_KEYWORD, "STEP")) {
		res.register_advancement(); advance();
		step_value = res.register_result(expr());
		if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
	}

	if (!current_tok.matches(TT_KEYWORD, "THEN")) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'THEN'")));
	}

	res.register_advancement(); advance();

	std::shared_ptr<Node> body = res.register_result(expr());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	return std::make_shared<ParseResult>(res.success(std::make_shared<ForNode>(ForNode(var_name_tok, start_value, end_value, step_value, body))));
}

std::shared_ptr<Node> Parser::while_expr() {
	std::cout << "Pk-entering while_expr\n";
	ParseResult res = ParseResult();

	if (!current_tok.matches(TT_KEYWORD, "WHILE")) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'WHILE'")));
	}

	res.register_advancement(); advance();

	std::shared_ptr<Node> condition = res.register_result(expr());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	if (!current_tok.matches(TT_KEYWORD, "THEN")) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'THEN'")));
	}

	res.register_advancement(); advance();

	std::shared_ptr<Node> body = res.register_result(expr());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	return std::make_shared<ParseResult>(res.success(std::make_shared<WhileNode>(WhileNode(condition, body))));
}

std::shared_ptr<Node> Parser::func_def() {
    std::cout << "Pk-entering func_def\n";
	ParseResult res = ParseResult();

	if (!current_tok.matches(TT_KEYWORD, "FUN")) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'FUN'")));
	}

	res.register_advancement(); advance();

    Token var_name_tok;
    if(current_tok.type_ == TT_IDENTIFIER) {
        var_name_tok = current_tok;
        res.register_advancement(); advance();
        if (current_tok.type_ != TT_LPAREN) {
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '('")));
        }
	}
    else {
        var_name_tok.is_none = 1;
        if (current_tok.type_ != TT_LPAREN) {
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected identifier or '('")));
        }
    }	

	res.register_advancement(); advance();
	std::vector<Token> arg_name_toks;

	if (current_tok.type_ == TT_IDENTIFIER) {
		arg_name_toks.push_back(current_tok);
		res.register_advancement(); advance();

        while (current_tok.type_ == TT_COMMA) {
            res.register_advancement(); advance();

            if (current_tok.type_ != TT_IDENTIFIER) {
                return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected identifier")));
            }

            arg_name_toks.push_back(current_tok);
            res.register_advancement(); advance();
        }

        if (current_tok.type_ != TT_RPAREN) {
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected ',' or ')'")));
        }
	}
    else {
        if (current_tok.type_ != TT_RPAREN) {
			return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected identifier or ')'")));
		}
    }	

	res.register_advancement(); advance();

	if (current_tok.type_ != TT_ARROW) {
		return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '->'")));
	}

	res.register_advancement(); advance();

	std::shared_ptr<Node> node_to_return = res.register_result(expr());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	return std::make_shared<ParseResult>(res.success(std::make_shared<FuncDefNode>(FuncDefNode(var_name_tok, arg_name_toks, node_to_return))));
}

std::shared_ptr<Node> Parser::call() {

    std::cout << "Pk-entering call\n";
	ParseResult res = ParseResult();
	std::shared_ptr<Node> atom_result = res.register_result(atom());
	if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);

	if (current_tok.type_ == TT_LPAREN) {
		res.register_advancement(); advance();
		std::vector<std::shared_ptr<Node>> arg_nodes;

		if (current_tok.type_ == TT_RPAREN) {
			res.register_advancement(); advance();
		}
		else {
			arg_nodes.push_back(res.register_result(expr()));
			if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected ')', 'VAR', 'IF', 'FOR', 'WHILE', 'FUN', int, float, identifier, '+', '-', '(', '[' or 'NOT'")));

			while (current_tok.type_ == TT_COMMA) {
				res.register_advancement(); advance();
				arg_nodes.push_back(res.register_result(expr()));
				if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
			}

			if (current_tok.type_ != TT_RPAREN) {
				return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected ',' or ')'")));
			}

			res.register_advancement(); advance();
		}

		return std::make_shared<ParseResult>(res.success(std::make_shared<CallNode>(CallNode(atom_result, arg_nodes))));
	}

	return std::make_shared<ParseResult>(res.success(atom_result));
}

std::shared_ptr<Node> Parser::atom() {
    std::cout << "Pk-entering atom\n";
    ParseResult res = ParseResult();
    Token tok = current_tok;

    if (tok.type_ == TT_INT || tok.type_ == TT_FLOAT) {
        res.register_advancement(); advance();
        return std::make_shared<ParseResult>(res.success(std::make_shared<NumberNode>(tok)));
    }

    else if (tok.type_ == TT_STRING) {
        res.register_advancement(); advance();
        return std::make_shared<ParseResult>(res.success(std::make_shared<StringNode>(tok)));
    }

    else if (tok.type_ == TT_IDENTIFIER) {
		res.register_advancement(); advance();
        std::cout << "VarAccessNode: " << tok.text << std::endl;
		return std::make_shared<ParseResult>(res.success(std::make_shared<VarAccessNode>(VarAccessNode(tok))));
	}

    else if (tok.type_ == TT_LPAREN) {
        res.register_advancement(); advance();
        std::shared_ptr<Node> expr_result = res.register_result(expr());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        if (current_tok.type_ == TT_RPAREN) {
            res.register_advancement(); advance();
            return std::make_shared<ParseResult>(res.success(expr_result));
        }
        else {
            std::cout << "InvalidSyntaxError in factor: " << tok.pos_start.col << ", " << tok.pos_end.col << std::endl;
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(tok.pos_start, tok.pos_end, "Expected ')'")));
        }
     }

    else if (tok.type_ == TT_LSQUARE) {
        std::shared_ptr<Node> list_expr_result = res.register_result(list_expr());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        return std::make_shared<ParseResult>(res.success(list_expr_result));
    }

    else if (tok.matches(TT_KEYWORD, "IF")) {
        std::shared_ptr<Node> if_expr_result = res.register_result(if_expr());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        return std::make_shared<ParseResult>(res.success(if_expr_result));
    }

    else if (tok.matches(TT_KEYWORD, "FOR")) {
        std::shared_ptr<Node> for_expr_result = res.register_result(for_expr());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        return std::make_shared<ParseResult>(res.success(for_expr_result));
    }

    else if (tok.matches(TT_KEYWORD, "WHILE")) {
        std::shared_ptr<Node> while_expr_result = res.register_result(while_expr());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        return std::make_shared<ParseResult>(res.success(while_expr_result));
    }

    else if (tok.matches(TT_KEYWORD, "FUN")) {
        std::shared_ptr<Node> func_def_result = res.register_result(func_def());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        return std::make_shared<ParseResult>(res.success(func_def_result));
    }

    return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(tok.pos_start, tok.pos_end, "Expected int, float, identifier, '+', '-', '(', '[', 'IF', 'FOR', 'WHILE' or 'FUN'")));
}

std::shared_ptr<Node> Parser::power() {
    std::cout << "Pk-entering power\n";
	return bin_op(bind(&Parser::call, this), {TT_POW}, bind(&Parser::factor, this));
}

std::shared_ptr<Node> Parser::factor() {
    std::cout << "Pk-entering factor\n";
    ParseResult res = ParseResult();
    Token tok = current_tok;

    std::cout << "Entered factor..."<<tok.type_<<std::endl;

    if (tok.type_ == TT_PLUS || tok.type_ == TT_MINUS) {
		res.register_advancement(); advance();
		std::shared_ptr<Node> factor_result = res.register_result(factor());
		if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
		return std::make_shared<ParseResult>(res.success(std::make_shared<UnaryOpNode>(tok, factor_result)));
	} 

    return power();

    //std::cout<< "InvalidSyntaxError in factor: " << tok.pos_start.col << ", " << tok.pos_end.col << std::endl;
    //return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(tok.pos_start, tok.pos_end, "Expected int or float")));
}

std::shared_ptr<Node> Parser::term() {
    std::cout << "Pk-entering term\n";
    return bin_op(bind(&Parser::factor, this), {TT_MUL, TT_DIV});
}

std::shared_ptr<Node> Parser::arith_expr() {
	std::cout << "Pk-entering arith_expr\n";
	return bin_op(bind(&Parser::term, this), {TT_PLUS, TT_MINUS});
}

std::shared_ptr<Node> Parser::comp_expr() {
	std::cout << "Pk-entering comp_expr\n";
	ParseResult res = ParseResult();
    if (current_tok.matches(TT_KEYWORD, "NOT")) {
		Token op_tok = current_tok;
		res.register_advancement(); advance();
		std::shared_ptr<Node> node = res.register_result(comp_expr());
		if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
		return std::make_shared<ParseResult>(res.success(std::make_shared<UnaryOpNode>(op_tok, node)));
	}
	std::shared_ptr<Node> node = res.register_result(bin_op(bind(&Parser::arith_expr, this), { TT_EE, TT_NE, TT_LT, TT_GT, TT_LTE, TT_GTE}));
    if (res.error.is_error() != "None") {
        return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected int, float, identifier, '+', '-', '(', '[' or 'NOT'")));
    }
	return std::make_shared<ParseResult>(res.success(node));
}   

std::shared_ptr<Node> Parser::expr() {
    std::cout << "Pk-entering expr\n";
    ParseResult res = ParseResult();
    if (current_tok.matches(TT_KEYWORD, "VAR")) {
        std::cout << "Variable assignment detected\n";
        res.register_advancement(); advance();
        if (current_tok.type_ != TT_IDENTIFIER) {
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected identifier")));
        }
        Token var_name = current_tok;
        res.register_advancement(); advance();
        if (current_tok.type_ != TT_EQ) {
            return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected '='")));
        }
        res.register_advancement(); advance();
        std::shared_ptr<Node> expression = res.register_result(expr());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        std::cout << "Variable assignment successful\n";
        return std::make_shared<ParseResult>(res.success(std::make_shared<VarAssignNode>(VarAssignNode(var_name, expression))));
    }
    std::cout << "Parsing expression\n";
    std::shared_ptr<Node> node = res.register_result(bin_op(bind(&Parser::comp_expr, this), { "AND", "OR" }));
    

    if (res.error.is_error() != "None") {
        return std::make_shared<ParseResult>(res.failure(InvalidSyntaxError(current_tok.pos_start, current_tok.pos_end, "Expected 'VAR', 'IF', 'FOR', 'WHILE', 'FUN', int, float, identifier, '+', '-', '(', '[' or 'NOT'")));
    }
    return std::make_shared<ParseResult>(res.success(node));
}


std::shared_ptr<Node> Parser::bin_op(std::function<std::shared_ptr<Node>()> func_a, std::vector<std::string> ops, std::function<std::shared_ptr<Node>()> func_b) {
    std::cout << "Pk-entering bin_op\n";
    ParseResult res = ParseResult();
    std::shared_ptr<Node> left = res.register_result(func_a());



    if(func_b==nullptr) func_b = func_a;


    if (res.error.is_error() != "None") {
        return std::make_shared<ParseResult>(res);
    }

    while ((find(ops.begin(), ops.end(), current_tok.type_) != ops.end()) || (find(ops.begin(), ops.end(), current_tok.text) != ops.end())) {
        Token op_tok = current_tok;
        res.register_advancement(); advance();
        std::shared_ptr<Node> right = res.register_result(func_b());
        if (res.error.is_error() != "None") return std::make_shared<ParseResult>(res);
        left = std::make_shared<BinOpNode>(left, op_tok, right);
    }

    std::cout<< "Returning BinOpNode " << std::endl;
    return std::make_shared<ParseResult>(res.success(left));
}



////////////////////////////
///////// VALUES ///////////
////////////////////////////

Number::Number() {}

Number::Number(double value, bool is_none) 
    : value(value), is_none(is_none) {
    set_pos();
    set_context();
}

//Number::Number(std::shared_ptr<Node> node) {
//    if (auto numberNode = std::dynamic_pointer_cast<NumberNode>(node)) {
//        this->value = numberNode->tok.value; // Assuming Token has a value field
//        this->is_none = false;
//        set_pos(numberNode->pos_start, numberNode->pos_end);
//        set_context(nullptr);
//    }
//    else {
//        throw std::runtime_error("Node is not a NumberNode");
//    }
//}

Number Number::set_pos(Position pos_start, Position pos_end) {
	this->pos_start = pos_start;
	this->pos_end = pos_end;
	return *this;
}

Number Number::set_context(Context* context) {
	this->context = context;
	return *this;
}

std::pair<Number, Error> Number::added_to(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
		return std::make_pair(Number(value + std::dynamic_pointer_cast<Number>(other)->value).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::subbed_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
		return std::make_pair(Number(value - std::dynamic_pointer_cast<Number>(other)->value).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::multed_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
		return std::make_pair(Number(value * std::dynamic_pointer_cast<Number>(other)->value).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::dived_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        if (std::dynamic_pointer_cast<Number>(other)->value == 0) {
			return std::make_pair(Number(0), RTError(std::dynamic_pointer_cast<Number>(other)->pos_start, std::dynamic_pointer_cast<Number>(other)->pos_end, "Division by zero", this->context));
		}
		return std::make_pair(Number(value / std::dynamic_pointer_cast<Number>(other)->value).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::powed_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(std::pow(value, std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
    }
}

std::pair<Number, Error> Number::get_comparison_eq(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(double(value == std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
    }
}

std::pair<Number, Error> Number::get_comparison_ne(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(double(value != std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
    }
}

std::pair<Number, Error> Number::get_comparison_lt(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(double(value < std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
    }
}

std::pair<Number, Error> Number::get_comparison_gt(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(double(value > std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
    }
}

std::pair<Number, Error> Number::get_comparison_lte(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(double(value <= std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
    }
}

std::pair<Number, Error> Number::get_comparison_gte(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        return std::make_pair(Number(double(value >= std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::anded_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
		return std::make_pair(Number(double(value && std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::ored_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
		return std::make_pair(Number(double(value || std::dynamic_pointer_cast<Number>(other)->value)).set_context(this->context), Error());
	}
}

std::pair<Number, Error> Number::notted() {
	return std::make_pair(Number(value==0?1:0).set_context(this->context), Error());
}

Number Number::copy() {
	Number copy = Number(value);
	copy.set_pos(pos_start, pos_end);
	copy.set_context(context);
	return copy;
}

bool Number::is_true() {
	return value == 0 ? 0 : 1;
}

std::ostream& operator<<(std::ostream& os, const Number& obj) {
	os << obj.value;
	return os;
}

void Number::print(std::ostream& os) const {
	os << value;
}

std::string Number::get_class_name() const {
    return "Number";
}

String::String(std::string value)
    : value(value) {
    set_pos();
    set_context();
}

String String::set_pos(Position pos_start, Position pos_end) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
    return *this;
}

String String::set_context(Context* context) {
    this->context = context;
    return *this;
}

std::pair<String, Error> String::added_to(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "String") {
        return std::make_pair(String(value + std::dynamic_pointer_cast<String>(other)->value).set_context(this->context), Error());
    }
}

std::pair<String, Error> String::multed_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        std::string final_value = "";
        for (double i = 0; i < std::dynamic_pointer_cast<Number>(other)->value; ++i) {
            final_value += value;
        }        
        return std::make_pair(String(final_value).set_context(this->context), Error());
    }
}

String String::copy() {
    String copy = String(value);
    copy.set_pos(pos_start, pos_end);
    copy.set_context(context);
    return copy;
}

bool String::is_true() {
    return value.size()>0;
}

std::ostream& operator<<(std::ostream& os, const String& obj) {
    os << obj.value;
    return os;
}

void String::print(std::ostream& os) const {
    os << value;
}

std::string String::get_class_name() const {
    return "String";
}

List::List() {}

List::List(std::vector<std::shared_ptr<Node>> elements)
    : elements(elements) {
    set_pos();
    set_context();
}

List List::set_pos(Position pos_start, Position pos_end) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
    return *this;
}

List List::set_context(Context* context) {
    this->context = context;
    return *this;
}

std::pair<List, Error> List::added_to(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        List new_list = this->copy();
        new_list.elements.push_back(other);
        return std::make_pair(new_list, Error());
    }
}

std::pair<List, Error> List::subbed_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        List new_list = this->copy();
        if ((int)(std::dynamic_pointer_cast<Number>(other)->value) < elements.size()) {
            new_list.elements.erase(new_list.elements.begin() + std::dynamic_pointer_cast<Number>(other)->value);
        }
        else {
            return std::make_pair(new_list, RTError(std::dynamic_pointer_cast<Number>(other)->pos_start, std::dynamic_pointer_cast<Number>(other)->pos_end, "Element at this index could not be removed from list because index is out of bounds", context));
        }
        return std::make_pair(new_list, Error());
    }
}

std::pair<List, Error> List::multed_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "List") {
        List new_list = this->copy();
        new_list.elements.insert(new_list.elements.end(), std::dynamic_pointer_cast<List>(other)->elements.begin(), std::dynamic_pointer_cast<List>(other)->elements.end());
        return std::make_pair(new_list, Error());
    }
}

std::pair<std::shared_ptr<Node>, Error> List::dived_by(std::shared_ptr<Node> other) {
    if (other->get_class_name() == "Number") {
        std::shared_ptr<Node> res;
        if((int)(std::dynamic_pointer_cast<Number>(other)->value) < elements.size()) {
            return std::make_pair(elements[(int)(std::dynamic_pointer_cast<Number>(other)->value)], Error());
        }
        else {
            return std::make_pair(res, RTError(std::dynamic_pointer_cast<Number>(other)->pos_start, std::dynamic_pointer_cast<Number>(other)->pos_end, "Element at this index could not be retrieved from list because index is out of bounds", context));
        }
    }
}

List List::copy() {
    List copy = List(elements);
    copy.set_pos(pos_start, pos_end);
    copy.set_context(context);
    return copy;
}

std::ostream& operator<<(std::ostream& os, const List& obj) {
    os << "[";
    for (auto x : obj.elements) {
        if (x->get_class_name() == "Number") {
            os << std::to_string(std::dynamic_pointer_cast<Number>(x)->value);
        }
        else if (x->get_class_name() == "String") {
            os << std::dynamic_pointer_cast<String>(x)->value;
        }
        else if (x->get_class_name() == "List") {
            std::dynamic_pointer_cast<List>(x)->print(os);
        }
        os << ", ";
    }
    os << ']';
    return os;
}

void List::print(std::ostream& os) const {
    os << "[";
    for (auto x : elements) {
        if (x->get_class_name() == "Number") {
            os<<std::to_string(std::dynamic_pointer_cast<Number>(x)->value);
        }
        else if (x->get_class_name() == "String") {
            os<< std::dynamic_pointer_cast<String>(x)->value;
        }
        else if (x->get_class_name() == "List") {
            std::dynamic_pointer_cast<List>(x)->print(os);
        }
        os<< ", ";
    }
    os << ']';
}

std::string List::get_class_name() const {
    return "List";
}

BaseFunction::BaseFunction() {}

BaseFunction::BaseFunction(std::string name) {
    if (name.empty()) this->name = "<anonymous>";
    else this->name = name;
    set_pos();
    set_context();
}

BaseFunction BaseFunction::set_pos(Position pos_start, Position pos_end) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
    return *this;
}

BaseFunction BaseFunction::set_context(Context* context) {
    this->context = context;
    return *this;
}

Context* BaseFunction::generate_new_context() {
    Context* new_context = new Context(name, context, pos_start);
    if (new_context->parent != nullptr) new_context->symbol_table = new SymbolTable(new_context->parent->symbol_table);
    else new_context->symbol_table = new SymbolTable();
    return new_context;
}

RTResult BaseFunction::check_args(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Node>> args) {
    RTResult res = RTResult();

    if (args.size() > arg_names.size()) {
        std::string temp = std::to_string(args.size() - arg_names.size()) + " too many arguments passed into " + name;
        return res.failure(RTError(pos_start, pos_end, temp, context));
    }

    if (args.size() < arg_names.size()) {
        std::string temp = std::to_string(arg_names.size() - args.size()) + " too few arguments passed into " + name;
        return res.failure(RTError(pos_start, pos_end, temp, context));
    }

    return res.success(std::make_shared<Number>(Number(0,1)));
}

void BaseFunction::populate_args(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Node>> args, Context* exec_ctx) {
    for (int i = 0; i < args.size(); i++) {
        std::string arg_name = arg_names[i];
        std::shared_ptr<Node> arg_value = args[i];
        if (arg_value->get_class_name() == "Number") 
            std::dynamic_pointer_cast<Number>(arg_value)->set_context(exec_ctx);
        else if (arg_value->get_class_name() == "String") 
            std::dynamic_pointer_cast<String>(arg_value)->set_context(exec_ctx);
        else if (arg_value->get_class_name() == "List") 
            std::dynamic_pointer_cast<List>(arg_value)->set_context(exec_ctx);
        exec_ctx->symbol_table->set(arg_name, arg_value);
    }
}

RTResult BaseFunction::check_and_populate_args(std::vector<std::string> arg_names, std::vector<std::shared_ptr<Node>> args, Context* exec_ctx) {
    RTResult res = RTResult();

    res.register_result(check_args(arg_names, args));
    if (res.error.is_error() != "None") return res;

    populate_args(arg_names, args, exec_ctx);
    return res.success(std::make_shared<Number>(Number(0, 1)));
}

std::ostream& operator<<(std::ostream& os, const BaseFunction& obj) {
    //os << obj.value;
    return os;
}

void BaseFunction::print(std::ostream& os) const {
    os << "";
}

std::string BaseFunction::get_class_name() const {
    return "BaseFunction";
}

Function::Function(std::string name, std::shared_ptr<Node> body_node, std::vector<std::string> arg_names)
    : BaseFunction(name), body_node(body_node), arg_names(arg_names) {}

RTResult Function::execute_result(std::vector<std::shared_ptr<Node>> args) {
    RTResult res = RTResult();
    Interpreter interpreter = Interpreter();
    Context* exec_ctx = generate_new_context();
    
    res.register_result(check_and_populate_args(arg_names, args, exec_ctx));
    if (res.error.is_error() != "None") return res;

    std::shared_ptr<Node> value = res.register_result(interpreter.visit(body_node, exec_ctx));
	if (res.error.is_error() != "None") return res;
	return res.success(value);
}

Function Function::copy() {
	Function copy = Function(name, body_node, arg_names);
	copy.set_pos(pos_start, pos_end);
	copy.set_context(context);
	return copy;
}

std::ostream& operator<<(std::ostream& os, const Function& obj) {
	os << "<function " << obj.name << ">";
	return os;
}

void Function::print(std::ostream& os) const {
	os << "<function " << name << ">";
}

std::string Function::get_class_name() const {
	return "Function";
}

BuiltInFunction::BuiltInFunction(std::string name)
    : BaseFunction(name) {}

BuiltInFunction BuiltInFunction::copy() {
    BuiltInFunction copy = BuiltInFunction(name);
    copy.set_pos(pos_start, pos_end);
    copy.set_context(context);
    return copy;
}

RTResult BuiltInFunction::execute_result(std::vector<std::shared_ptr<Node>> args) {
    RTResult res = RTResult();
    Context* exec_ctx = generate_new_context();

    std::string method_name = "execute_" + name;
    std::shared_ptr<Node> return_value;
    std::cout << "Method name in Builtin execute function: " << method_name << std::endl;


    if (method_name == "execute_print") {
        res.register_result(check_and_populate_args(execute_print_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_print(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_print_ret") {
        res.register_result(check_and_populate_args(execute_print_ret_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_print_ret(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_input") {
        res.register_result(check_and_populate_args(execute_input_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_input(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_input_int") {
        res.register_result(check_and_populate_args(execute_input_int_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_input_int(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_clear") {
        res.register_result(check_and_populate_args(execute_clear_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_clear(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_is_number") {
        res.register_result(check_and_populate_args(execute_is_number_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_is_number(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_is_string") {
        res.register_result(check_and_populate_args(execute_is_string_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_is_string(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_is_list") {
        res.register_result(check_and_populate_args(execute_is_list_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_is_list(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_is_function") {
        res.register_result(check_and_populate_args(execute_is_function_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_is_function(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_append") {
        res.register_result(check_and_populate_args(execute_append_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_append(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_pop") {
        res.register_result(check_and_populate_args(execute_pop_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_pop(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else if (method_name == "execute_extend") {
        res.register_result(check_and_populate_args(execute_extend_arg_names_, args, exec_ctx));
        if (res.error.is_error() != "None") return res;

        return_value = res.register_result(execute_extend(exec_ctx));
        if (res.error.is_error() != "None") return res;
    }
    else {
        return no_visit_method(context);
    }
    return res.success(return_value);
}

RTResult BuiltInFunction::execute_print(Context* exec_ctx) {
    if (exec_ctx->symbol_table->get("value")->get_class_name() == "Number") 
        std::cout << *std::dynamic_pointer_cast<Number>(exec_ctx->symbol_table->get("value")) << std::endl;
    else if (exec_ctx->symbol_table->get("value")->get_class_name() == "String") 
        std::cout << *std::dynamic_pointer_cast<String>(exec_ctx->symbol_table->get("value")) << std::endl;
    else if (exec_ctx->symbol_table->get("value")->get_class_name() == "List") 
        std::cout << *std::dynamic_pointer_cast<List>(exec_ctx->symbol_table->get("value")) << std::endl;

    return RTResult().success(std::make_shared<Number>(Number(0, 1)));
}

RTResult BuiltInFunction::execute_print_ret(Context* exec_ctx) {
    return RTResult().success(exec_ctx->symbol_table->get("value"));
}

RTResult BuiltInFunction::execute_input(Context* exec_ctx) {
    std::string input;
    std::cin >> input;
    return RTResult().success(std::make_shared<String>(String(input)));
}

RTResult BuiltInFunction::execute_input_int(Context* exec_ctx) {    
    int res;    
    while (1) {
        std::string input;
        std::cin >> input;
        if (std::all_of(input.begin(), input.end(), ::isdigit)) {
            res = std::stoi(input);
            break;
        }
        else {
            std::cout << "'" + input + "' must be an integer. Try again!\n";
        }
    }    
    return RTResult().success(std::make_shared<Number>(Number((double)res)));
}

RTResult BuiltInFunction::execute_clear(Context* exec_ctx) {
    std::cout << "\033[2J\033[1;1H";
    return RTResult().success(std::make_shared<Number>(Number(0,1)));
}

RTResult BuiltInFunction::execute_is_number(Context* exec_ctx) {
    bool is_number;
    exec_ctx->symbol_table->get("value")->get_class_name() == "Number" ? is_number = 1 : is_number = 0;
    return RTResult().success(std::make_shared<Number>(is_number ? Number::true_: Number::false_));
}

RTResult BuiltInFunction::execute_is_string(Context* exec_ctx) {
    bool is_string;
    exec_ctx->symbol_table->get("value")->get_class_name() == "String" ? is_string = 1 : is_string = 0;
    return RTResult().success(std::make_shared<Number>(is_string ? Number::true_ : Number::false_));
}

RTResult BuiltInFunction::execute_is_list(Context* exec_ctx) {
    bool is_list;
    exec_ctx->symbol_table->get("value")->get_class_name() == "List" ? is_list = 1 : is_list = 0;
    return RTResult().success(std::make_shared<Number>(is_list ? Number::true_ : Number::false_));
}

RTResult BuiltInFunction::execute_is_function(Context* exec_ctx) {
    bool is_function=0;
    std::string temp = exec_ctx->symbol_table->get("value")->get_class_name();
    if (temp == "Function" || temp == "BaseFunction" || temp == "BuiltInFunction") is_function = 1;
    return RTResult().success(std::make_shared<Number>(is_function ? Number::true_ : Number::false_));
}

RTResult BuiltInFunction::execute_append(Context* exec_ctx) {
    std::shared_ptr<Node> list_ = exec_ctx->symbol_table->get("list");
    std::shared_ptr<Node> value = exec_ctx->symbol_table->get("value");

    if (list_->get_class_name() != "List") {
        return RTResult().failure(RTError(pos_start, pos_end, "First argument must be a list", exec_ctx));
    }

    std::dynamic_pointer_cast<List>(list_)->elements.push_back(value);

    return RTResult().success(std::make_shared<Number>(Number(0, 1)));
}

RTResult BuiltInFunction::execute_pop(Context* exec_ctx) {
    std::shared_ptr<Node> list_ = exec_ctx->symbol_table->get("list");
    std::shared_ptr<Node> index = exec_ctx->symbol_table->get("index");

    if (list_->get_class_name() != "List") {
        return RTResult().failure(RTError(pos_start, pos_end, "First argument must be a list", exec_ctx));
    }

    if (index->get_class_name() != "Number") {
        return RTResult().failure(RTError(pos_start, pos_end, "Second argument must be a number", exec_ctx));
    }

    std::shared_ptr<Node> element;

    if ((int)(std::dynamic_pointer_cast<Number>(index)->value) < std::dynamic_pointer_cast<List>(list_)->elements.size()) {
        element = std::dynamic_pointer_cast<List>(list_)->elements[std::dynamic_pointer_cast<Number>(index)->value];
        std::dynamic_pointer_cast<List>(list_)->elements.erase(std::dynamic_pointer_cast<List>(list_)->elements.begin() + std::dynamic_pointer_cast<Number>(index)->value);
    }
    else {
        return RTResult().failure(RTError(pos_start, pos_end, "Element at this index could not be removed from list because index is out of bounds", exec_ctx));
    }
    return RTResult().success(element);
}

RTResult BuiltInFunction::execute_extend(Context* exec_ctx) {
    std::shared_ptr<Node> listA = exec_ctx->symbol_table->get("listA");
    std::shared_ptr<Node> listB = exec_ctx->symbol_table->get("listB");

    if (listA->get_class_name() != "List") {
        return RTResult().failure(RTError(pos_start, pos_end, "First argument must be a list", exec_ctx));
    }

    if (listB->get_class_name() != "List") {
        return RTResult().failure(RTError(pos_start, pos_end, "Second argument must be a list", exec_ctx));
    }

    std::dynamic_pointer_cast<List>(listA)->elements.insert(std::dynamic_pointer_cast<List>(listA)->elements.end(), std::dynamic_pointer_cast<List>(listB)->elements.begin(), std::dynamic_pointer_cast<List>(listB)->elements.end());

    return RTResult().success(std::make_shared<Number>(Number(0, 1)));
}

RTResult BuiltInFunction::no_visit_method(Context* context) {
    throw std::runtime_error("No execute_" + name + " method defined");
    RTResult temp = RTResult(); // To avoid compilation error
    return temp;
}

std::ostream& operator<<(std::ostream& os, const BuiltInFunction& obj) {
    os << "<built-in function " << obj.name << ">";
    return os;
}

void BuiltInFunction::print(std::ostream& os) const {
    os << "<built-in function " << name << ">";
}

std::string BuiltInFunction::get_class_name() const {
    return "BuiltInFunction";
}

////////////////////////////
///////// CONTEXT //////////
////////////////////////////

Context::Context() {}

Context::Context(std::string display_name, Context* parent, Position parent_entry_pos)
    : display_name(display_name), parent(parent), parent_entry_pos(parent_entry_pos) {
    symbol_table = nullptr;
}

////////////////////////////
/////// SYMBOL TABLE ///////
////////////////////////////

SymbolTable::SymbolTable(SymbolTable* parent)
    : parent(parent) {
    symbols = std::unordered_map<std::string, std::shared_ptr<Node>>();
}

std::shared_ptr<Node> SymbolTable::get(std::string name) {
    std::shared_ptr<Node> value=nullptr;

    if (this->symbols.find(name) != this->symbols.end()) {        
        value = symbols[name];
    }
    else if (this->parent != nullptr) {
        value = this->parent->get(name);
    }
    return value;
}

void SymbolTable::set(std::string name, std::shared_ptr<Node> value) {
    symbols[name] = value;
}

void SymbolTable::remove(std::string name) {
	symbols.erase(name);
}

////////////////////////////
/////// INTERPRETER ////////
////////////////////////////

Interpreter::Interpreter() {}

RTResult Interpreter::visit(std::shared_ptr<Node> node, Context* context) {
    //std::cout<<"Context in interpreter visit function: "<<context->display_name<<std::endl;
    std::string method_name = "visit_" + node->get_class_name(); 
    std::cout<< "Method name in interpreter visit function: " << method_name << std::endl;
    if (method_name == "visit_NumberNode") {
        return visit_NumberNode(node, context);
    }
    else if (method_name == "visit_StringNode") {
        return visit_StringNode(node, context);
    }
    else if (method_name == "visit_BinOpNode") {
        return visit_BinOpNode(node, context);
	}
    else if (method_name == "visit_UnaryOpNode") {
		return visit_UnaryOpNode(node, context);
	}
    else if (method_name == "visit_VarAccessNode") {
        return visit_VarAccessNode(node, context);
    }
    else if (method_name == "visit_VarAssignNode") {
        return visit_VarAssignNode(node, context);
    }
    else if (method_name == "visit_IfNode") {
		return visit_IfNode(node, context);
	}
    else if (method_name == "visit_ForNode") {
        return visit_ForNode(node, context);
    }
    else if (method_name == "visit_WhileNode") {
		return visit_WhileNode(node, context);
	}
	else if (method_name == "visit_FuncDefNode") {
		return visit_FuncDefNode(node, context);
	}
	else if (method_name == "visit_CallNode") {
		return visit_CallNode(node, context);
	} 
    else if (method_name == "visit_ListNode") {
        return visit_ListNode(node, context);
    }
    else {
        return no_visit_method(node, context);
    }
}

RTResult Interpreter::no_visit_method(std::shared_ptr<Node> node, Context* context) {
    throw std::runtime_error("No visit_" + node->get_class_name() + " method defined");
    RTResult temp = RTResult(); // To avoid compilation error
    return temp;
}

RTResult Interpreter::visit_NumberNode(std::shared_ptr<Node> node, Context* context) {
    std::cout << "Visiting NumberNode" << std::endl;
    //std::cout << "Context in NumberNode: " << context->display_name << std::endl;
	Number number = Number(std::dynamic_pointer_cast<NumberNode>(node)->tok.value);
    number.set_context(context);
    return RTResult().success(std::make_shared<Number>(number.set_pos(std::dynamic_pointer_cast<NumberNode>(node)->pos_start, std::dynamic_pointer_cast<NumberNode>(node)->pos_end)));
}

RTResult Interpreter::visit_StringNode(std::shared_ptr<Node> node, Context* context) {
    std::cout << "Visiting StringNode" << std::endl;
    //std::cout << "Context in StringNode: " << context->display_name << std::endl;
    String string_val = String(std::dynamic_pointer_cast<StringNode>(node)->tok.text);
    string_val.set_context(context);
    return RTResult().success(std::make_shared<String>(string_val.set_pos(std::dynamic_pointer_cast<StringNode>(node)->pos_start, std::dynamic_pointer_cast<StringNode>(node)->pos_end)));
}

RTResult Interpreter::visit_ListNode(std::shared_ptr<Node> node, Context* context) {
    std::cout << "Visiting ListNode" << std::endl;
    //std::cout << "Context in ListNode: " << context->display_name << std::endl;
    RTResult res = RTResult();
    std::vector<std::shared_ptr<Node>> elements;

    if (!std::dynamic_pointer_cast<ListNode>(node)->element_nodes.empty()) {
        for (auto element_node : std::dynamic_pointer_cast<ListNode>(node)->element_nodes) {
            elements.push_back(res.register_result(visit(element_node, context)));
            if (res.error.is_error() != "None") return res;
        }
    }    

    List result_list = List(elements);
    result_list.set_context(context);
    return res.success(std::make_shared<List>(result_list.set_pos(std::dynamic_pointer_cast<ListNode>(node)->pos_start, std::dynamic_pointer_cast<ListNode>(node)->pos_end)));
}

RTResult Interpreter::visit_VarAccessNode(std::shared_ptr<Node> node, Context* context) {
	std::cout << "Visiting VarAccessNode" << std::endl;
	//std::cout << "Context in VarAccessNode: " << context->display_name << std::endl;
    RTResult res = RTResult();
	std::string var_name = std::dynamic_pointer_cast<VarAccessNode>(node)->var_name_tok.text;
    std::cout << "Variable name is: " << var_name << std::endl;

	std::shared_ptr<Node> value = context->symbol_table->get(var_name);

    std::cout << "Value found" << std::endl;
     
    //std::cout << "Value is: " << *value << std::endl;
    if (value == nullptr ){
        std::string error = var_name + " is not defined";
		return res.failure(RTError(std::dynamic_pointer_cast<VarAccessNode>(node)->pos_start, std::dynamic_pointer_cast<VarAccessNode>(node)->pos_end, error, context));
	}

    /*Number final_value = (*std::dynamic_pointer_cast<Number>(value)).copy();
    final_value.set_pos(std::dynamic_pointer_cast<VarAccessNode>(node)->pos_start, std::dynamic_pointer_cast<VarAccessNode>(node)->pos_end);*/

    if (value->get_class_name() == "Number") {
        std::dynamic_pointer_cast<Number>(value)->set_context(context);
        std::dynamic_pointer_cast<Number>(value)->set_pos(std::dynamic_pointer_cast<VarAccessNode>(node)->pos_start, std::dynamic_pointer_cast<VarAccessNode>(node)->pos_end);
    }
    else if (value->get_class_name() == "String") {
        std::dynamic_pointer_cast<String>(value)->set_context(context);
        std::dynamic_pointer_cast<String>(value)->set_pos(std::dynamic_pointer_cast<VarAccessNode>(node)->pos_start, std::dynamic_pointer_cast<VarAccessNode>(node)->pos_end);
    }
    else if (value->get_class_name() == "Function") {
        std::dynamic_pointer_cast<Function>(value)->set_context(context);
        std::dynamic_pointer_cast<Function>(value)->set_pos(std::dynamic_pointer_cast<VarAccessNode>(node)->pos_start, std::dynamic_pointer_cast<VarAccessNode>(node)->pos_end);
    }

	return res.success(value);
}

RTResult Interpreter::visit_VarAssignNode(std::shared_ptr<Node> node, Context* context) {
	std::cout << "Visiting VarAssignNode" << std::endl;
	//std::cout << "Context in VarAssignNode: " << context->display_name << std::endl;
	RTResult res = RTResult();
	std::string var_name = std::dynamic_pointer_cast<VarAssignNode>(node)->var_name_tok.text;
	std::shared_ptr<Node> value = res.register_result(visit(std::dynamic_pointer_cast<VarAssignNode>(node)->value_node, context));
	if (res.error.is_error() != "None") return res;

	context->symbol_table->set(var_name, value);
    std::cout << "Value set as-pk: " << context->symbol_table->symbols[var_name]<<std::endl;
	return res.success(value);
}

RTResult Interpreter::visit_BinOpNode(std::shared_ptr<Node> node, Context* context) {
    std::cout << "Visiting BinOpNode" << std::endl;
    //std::cout << "Context in BinOpNode: " << context->display_name << std::endl;
    RTResult res = RTResult();
    std::shared_ptr<Node> left = res.register_result(visit(std::dynamic_pointer_cast<BinOpNode>(node)->left_node, context));
    if (res.error.is_error() != "None") return res;
    std::shared_ptr<Node> right = res.register_result(visit(std::dynamic_pointer_cast<BinOpNode>(node)->right_node, context));
    if (res.error.is_error() != "None") return res;

    std::cout<< "Left: " << left << ", Right: " << right << std::endl;

    Number result = Number(0);
    Error error = Error();

    if (left->get_class_name() == "String") {
        String result = String("");
        Error error = Error();
        if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_PLUS) {
            auto output = std::dynamic_pointer_cast<String>(left)->added_to(right);
            result = output.first;
            error = output.second;
        }
        else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_MUL) {
            auto output = std::dynamic_pointer_cast<String>(left)->multed_by(right);
            result = output.first;
            error = output.second;
        }
        if (error.is_error() != "None") {
            std::cout << "Error in BinOpNode: " << error.is_error() << std::endl;
            return res.failure(error);
        }
        else return res.success(std::make_shared<String>(result.set_pos(std::dynamic_pointer_cast<BinOpNode>(node)->pos_start, std::dynamic_pointer_cast<BinOpNode>(node)->pos_end)));
    }

    if (left->get_class_name() == "List") {
        List result;
        Error error = Error();

        if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_PLUS) {
            auto output = std::dynamic_pointer_cast<List>(left)->added_to(right);
            result = output.first;
            error = output.second;
        }
        if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_MINUS) {
            auto output = std::dynamic_pointer_cast<List>(left)->subbed_by(right);
            result = output.first;
            error = output.second;
        }
        if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_MUL) {
            auto output = std::dynamic_pointer_cast<List>(left)->multed_by(right);
            result = output.first;
            error = output.second;
        }
        if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_DIV) {
            auto output = std::dynamic_pointer_cast<List>(left)->dived_by(right);
            std::shared_ptr<Node> result_temp = output.first;
            error = output.second;
            if (error.is_error() != "None") {
                return res.failure(error);
            }
            if (result_temp->get_class_name() == "Number") {
                std::dynamic_pointer_cast<Number>(result_temp)->set_pos(std::dynamic_pointer_cast<BinOpNode>(node)->pos_start, std::dynamic_pointer_cast<BinOpNode>(node)->pos_end);
            }
            else if (result_temp->get_class_name() == "String") {
                std::dynamic_pointer_cast<String>(result_temp)->set_pos(std::dynamic_pointer_cast<BinOpNode>(node)->pos_start, std::dynamic_pointer_cast<BinOpNode>(node)->pos_end);
            }
            if (result_temp->get_class_name() == "List") {
                std::dynamic_pointer_cast<List>(result_temp)->set_pos(std::dynamic_pointer_cast<BinOpNode>(node)->pos_start, std::dynamic_pointer_cast<BinOpNode>(node)->pos_end);
            }
            return res.success(result_temp);
        }
        if (error.is_error() != "None") {
            return res.failure(error);
        }
        else return res.success(std::make_shared<List>(result.set_pos(std::dynamic_pointer_cast<BinOpNode>(node)->pos_start, std::dynamic_pointer_cast<BinOpNode>(node)->pos_end)));
    }

    if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_PLUS) {
		auto output = std::dynamic_pointer_cast<Number>(left)->added_to(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_MINUS) {
        auto output = std::dynamic_pointer_cast<Number>(left)->subbed_by(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_MUL) {
        auto output = std::dynamic_pointer_cast<Number>(left)->multed_by(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_DIV) {
        auto output = std::dynamic_pointer_cast<Number>(left)->dived_by(right);
        result = output.first;
        error = output.second;
	}  
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_POW) {
        auto output = std::dynamic_pointer_cast<Number>(left)->powed_by(right);
        result = output.first;
        error = output.second;
    }
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_EE) {
        auto output = std::dynamic_pointer_cast<Number>(left)->get_comparison_eq(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_NE) {
        auto output = std::dynamic_pointer_cast<Number>(left)->get_comparison_ne(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_LT) {
        auto output = std::dynamic_pointer_cast<Number>(left)->get_comparison_lt(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_GT) {
        auto output = std::dynamic_pointer_cast<Number>(left)->get_comparison_gt(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_LTE) {
        auto output = std::dynamic_pointer_cast<Number>(left)->get_comparison_lte(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.type_ == TT_GTE) {
        auto output = std::dynamic_pointer_cast<Number>(left)->get_comparison_gte(right);
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.matches(TT_KEYWORD, "AND")) {
        auto output = std::dynamic_pointer_cast<Number>(left)->anded_by(right);
        result = output.first;
        error = output.second;
	} 
    else if (std::dynamic_pointer_cast<BinOpNode>(node)->op_tok.matches(TT_KEYWORD, "OR")) {
        auto output = std::dynamic_pointer_cast<Number>(left)->ored_by(right);
        result = output.first;
        error = output.second;
    }

    //std::cout<<"Context in error: "<<error.context->display_name<<std::endl;

    //std::cout << "Result: " << result << std::endl;
    if (error.is_error() != "None") {
        std::cout<< "Error in BinOpNode: " << error.is_error() << std::endl;
        return res.failure(error);
    }
    else return res.success(std::make_shared<Number>(result.set_pos(std::dynamic_pointer_cast<BinOpNode>(node)->pos_start, std::dynamic_pointer_cast<BinOpNode>(node)->pos_end)));
}

RTResult Interpreter::visit_UnaryOpNode(std::shared_ptr<Node> node, Context* context) {
    std::cout << "Visiting UnaryOpNode" << std::endl;
    //std::cout << "Context in UnaryOpNode: " << context->display_name << std::endl;
    RTResult res = RTResult();
    std::shared_ptr<Node> number = res.register_result(visit(std::dynamic_pointer_cast<UnaryOpNode>(node)->node, context));
    if (res.error.is_error() != "None") return res;

    Number result = Number(0);
    Error error = Error();

    if (std::dynamic_pointer_cast<UnaryOpNode>(node)->op_tok.type_ == TT_MINUS) {
        auto output = std::dynamic_pointer_cast<Number>(number)->multed_by(std::make_shared<Number>(Number(-1)));
        result = output.first;
        error = output.second;
	}
    else if (std::dynamic_pointer_cast<UnaryOpNode>(node)->op_tok.matches(TT_KEYWORD, "NOT")) {
        auto output = std::dynamic_pointer_cast<Number>(number)->notted();
		result = output.first;
		error = output.second;
	}

    if (error.is_error() != "None") return res.failure(error);
    else return res.success(std::make_shared<Number>(result.set_pos(std::dynamic_pointer_cast<UnaryOpNode>(node)->pos_start, std::dynamic_pointer_cast<UnaryOpNode>(node)->pos_end)));
}

RTResult Interpreter::visit_IfNode(std::shared_ptr<Node> node, Context* context) {
	std::cout << "Visiting IfNode" << std::endl;
	//std::cout << "Context in IfNode: " << context->display_name << std::endl;
	RTResult res = RTResult();
	std::vector<std::vector<std::shared_ptr<Node>>> cases = std::dynamic_pointer_cast<IfNode>(node)->cases;
	std::shared_ptr<Node> else_case = std::dynamic_pointer_cast<IfNode>(node)->else_case;    

    for (auto case_ : cases) {
		std::shared_ptr<Node> condition = case_[0];
		std::shared_ptr<Node> expression = case_[1];

		std::shared_ptr<Node> condition_value = res.register_result(visit(condition, context));
		if (res.error.is_error() != "None") return res;

        if (std::dynamic_pointer_cast<Number>(condition_value)->is_true()) {
            std::shared_ptr<Node> expr_value = res.register_result(visit(expression, context));
			if (res.error.is_error() != "None") return res;
			return res.success(expr_value);
		}
	}

    if (else_case != nullptr) {
        std::shared_ptr<Node> else_value = res.register_result(visit(else_case, context));
		if (res.error.is_error() != "None") return res;
		return res.success(else_value);
	}

	return res.success(std::make_shared<Number>(Number(0, 1)));
}   

RTResult Interpreter::visit_ForNode(std::shared_ptr<Node> node, Context* context) {
	std::cout << "Visiting ForNode" << std::endl;
	//std::cout << "Context in ForNode: " << context->display_name << std::endl;
	RTResult res = RTResult();
    std::vector<std::shared_ptr<Node>> elements;

    std::shared_ptr<Node> start_value = res.register_result(visit(std::dynamic_pointer_cast<ForNode>(node)->start_value_node, context));
	if (res.error.is_error() != "None") return res;

    std::shared_ptr<Node> end_value = res.register_result(visit(std::dynamic_pointer_cast<ForNode>(node)->end_value_node, context));
	if (res.error.is_error() != "None") return res;

    std::shared_ptr<Node> step_value = std::make_shared<Number>(Number(1));
	if (std::dynamic_pointer_cast<ForNode>(node)->step_value_node != nullptr) {
		step_value = res.register_result(visit(std::dynamic_pointer_cast<ForNode>(node)->step_value_node, context));
		if (res.error.is_error() != "None") return res;
	}

	double i = std::dynamic_pointer_cast<Number>(start_value)->value;

    if (std::dynamic_pointer_cast<Number>(step_value)->value >= 0) {
        while (i < std::dynamic_pointer_cast<Number>(end_value)->value) {
            context->symbol_table->set(std::dynamic_pointer_cast<ForNode>(node)->var_name_tok.text, std::make_shared<Number>(Number(i)));
			i += std::dynamic_pointer_cast<Number>(step_value)->value;
            elements.push_back(res.register_result(visit(std::dynamic_pointer_cast<ForNode>(node)->body_node, context)));
			if (res.error.is_error() != "None") return res;
        }
    }
    else {
        while (i > std::dynamic_pointer_cast<Number>(end_value)->value) {
            context->symbol_table->set(std::dynamic_pointer_cast<ForNode>(node)->var_name_tok.text, std::make_shared<Number>(Number(i)));
            i += std::dynamic_pointer_cast<Number>(step_value)->value;
            elements.push_back(res.register_result(visit(std::dynamic_pointer_cast<ForNode>(node)->body_node, context)));
            if (res.error.is_error() != "None") return res;
        }
    }

    
	return res.success(std::make_shared<List>(List(elements).set_context(context).set_pos(std::dynamic_pointer_cast<ForNode>(node)->pos_start, std::dynamic_pointer_cast<ForNode>(node)->pos_end)));
}

RTResult Interpreter::visit_WhileNode(std::shared_ptr<Node> node, Context* context) {
	std::cout << "Visiting WhileNode" << std::endl;
	//std::cout << "Context in WhileNode: " << context->display_name << std::endl;
	RTResult res = RTResult();
    std::vector<std::shared_ptr<Node>> elements;

	while (true) {
		std::shared_ptr<Node> condition = res.register_result(visit(std::dynamic_pointer_cast<WhileNode>(node)->condition_node, context));
		if (res.error.is_error() != "None") return res;

		if (!std::dynamic_pointer_cast<Number>(condition)->is_true()) break;

        elements.push_back(res.register_result(visit(std::dynamic_pointer_cast<WhileNode>(node)->body_node, context)));
		if (res.error.is_error() != "None") return res;
	}

    return res.success(std::make_shared<List>(List(elements).set_context(context).set_pos(std::dynamic_pointer_cast<ListNode>(node)->pos_start, std::dynamic_pointer_cast<ListNode>(node)->pos_end)));
}

RTResult Interpreter::visit_FuncDefNode(std::shared_ptr<Node> node, Context* context) {
	std::cout << "Visiting FuncDefNode" << std::endl;
	//std::cout << "Context in FuncDefNode: " << context->display_name << std::endl;
	RTResult res = RTResult();
    std::string func_name = "None";
    if(!(std::dynamic_pointer_cast<FuncDefNode>(node)->var_name_tok.is_none)) func_name = std::dynamic_pointer_cast<FuncDefNode>(node)->var_name_tok.text;
	std::shared_ptr<Node> body_node = std::dynamic_pointer_cast<FuncDefNode>(node)->body_node;
    std::vector<Token> arg_name_toks = std::dynamic_pointer_cast<FuncDefNode>(node)->arg_name_toks;
	std::vector<std::string> arg_names;
    for(auto x: arg_name_toks) {
		arg_names.push_back(x.text);
	}

	Function func_value = Function(func_name, body_node, arg_names);
    func_value.set_context(context);
    func_value.set_pos(std::dynamic_pointer_cast<FuncDefNode>(node)->pos_start, std::dynamic_pointer_cast<FuncDefNode>(node)->pos_end);
	
    if (std::dynamic_pointer_cast<FuncDefNode>(node)->var_name_tok.text != "") {
        context->symbol_table->set(func_name, std::make_shared<Function>(func_value));
    }

	return res.success(std::make_shared<Function>(func_value));
}

RTResult Interpreter::visit_CallNode(std::shared_ptr<Node> node, Context* context) {
    std::cout << "Visiting CallNode" << std::endl;
    //std::cout << "Context in CallNode: " << context->display_name << std::endl;
    RTResult res = RTResult();

    std::vector<std::shared_ptr<Node>> args;

    std::shared_ptr<Node> value = res.register_result(visit(std::dynamic_pointer_cast<CallNode>(node)->node_to_call, context));
    if (res.error.is_error() != "None") return res;
    std::shared_ptr<Node> value_to_call;
    if (value->get_class_name() == "Function") {
        value_to_call = std::make_shared<Function>(std::dynamic_pointer_cast<Function>(value)->copy());
        std::dynamic_pointer_cast<Function>(value_to_call)->set_pos(std::dynamic_pointer_cast<CallNode>(node)->pos_start, std::dynamic_pointer_cast<CallNode>(node)->pos_end);
    }
    else if (value->get_class_name() == "BuiltInFunction") {
        value_to_call = std::make_shared<BuiltInFunction>(std::dynamic_pointer_cast<BuiltInFunction>(value)->copy());
        std::dynamic_pointer_cast<BuiltInFunction>(value_to_call)->set_pos(std::dynamic_pointer_cast<CallNode>(node)->pos_start, std::dynamic_pointer_cast<CallNode>(node)->pos_end);
    }
    
    for (auto x : std::dynamic_pointer_cast<CallNode>(node)->arg_nodes) {
        args.push_back(res.register_result(visit(x, context)));
        if (res.error.is_error() != "None") return res;        
    }

    std::shared_ptr<Node> return_value;
    if (value_to_call->get_class_name() == "Function") {
        return_value = res.register_result(std::dynamic_pointer_cast<Function>(value_to_call)->execute_result(args));
        if (res.error.is_error() != "None") return res;
    }
    else if (value_to_call->get_class_name() == "BuiltInFunction") {
        return_value = res.register_result(std::dynamic_pointer_cast<BuiltInFunction>(value_to_call)->execute_result(args));
        if (res.error.is_error() != "None") return res;
    }

    if (return_value->get_class_name() == "Number") {
        std::dynamic_pointer_cast<Number>(return_value)->set_context(context);
        std::dynamic_pointer_cast<Number>(return_value)->set_pos(std::dynamic_pointer_cast<CallNode>(node)->pos_start, std::dynamic_pointer_cast<CallNode>(node)->pos_end);
    }
    else if (return_value->get_class_name() == "String") {
        std::dynamic_pointer_cast<String>(return_value)->set_context(context);
        std::dynamic_pointer_cast<String>(return_value)->set_pos(std::dynamic_pointer_cast<CallNode>(node)->pos_start, std::dynamic_pointer_cast<CallNode>(node)->pos_end);
    }
    else if (return_value->get_class_name() == "List") {
        std::dynamic_pointer_cast<List>(return_value)->set_context(context);
        std::dynamic_pointer_cast<List>(return_value)->set_pos(std::dynamic_pointer_cast<CallNode>(node)->pos_start, std::dynamic_pointer_cast<CallNode>(node)->pos_end);
    }
    return res.success(return_value);
}

////////////////////////////
/////////// RUN ////////////
////////////////////////////

Number Number::null_ = Number(0);
Number Number::false_ = Number(0);
Number Number::true_ = Number(1);
Number Number::math_PI_ = Number(3.14159265358979323846);

std::vector<std::string> BuiltInFunction::execute_print_arg_names_{ "value" };
std::vector<std::string> BuiltInFunction::execute_print_ret_arg_names_{ "value" };
std::vector<std::string> BuiltInFunction::execute_input_arg_names_{};
std::vector<std::string> BuiltInFunction::execute_input_int_arg_names_{};
std::vector<std::string> BuiltInFunction::execute_clear_arg_names_{};
std::vector<std::string> BuiltInFunction::execute_is_number_arg_names_{ "value" };
std::vector<std::string> BuiltInFunction::execute_is_string_arg_names_{ "value" };
std::vector<std::string> BuiltInFunction::execute_is_list_arg_names_{ "value" };
std::vector<std::string> BuiltInFunction::execute_is_function_arg_names_{ "value" };
std::vector<std::string> BuiltInFunction::execute_append_arg_names_{ "list", "value" };
std::vector<std::string> BuiltInFunction::execute_pop_arg_names_{ "list", "index" };
std::vector<std::string> BuiltInFunction::execute_extend_arg_names_{ "listA", "listB" };

BuiltInFunction BuiltInFunction::BuiltInFunction_print = BuiltInFunction("print");
BuiltInFunction BuiltInFunction::BuiltInFunction_print_ret = BuiltInFunction("print_ret");
BuiltInFunction BuiltInFunction::BuiltInFunction_input = BuiltInFunction("input");
BuiltInFunction BuiltInFunction::BuiltInFunction_input_int = BuiltInFunction("input_int");
BuiltInFunction BuiltInFunction::BuiltInFunction_clear = BuiltInFunction("clear");
BuiltInFunction BuiltInFunction::BuiltInFunction_is_number = BuiltInFunction("is_number");
BuiltInFunction BuiltInFunction::BuiltInFunction_is_string = BuiltInFunction("is_string");
BuiltInFunction BuiltInFunction::BuiltInFunction_is_list = BuiltInFunction("is_list");
BuiltInFunction BuiltInFunction::BuiltInFunction_is_function = BuiltInFunction("is_function");
BuiltInFunction BuiltInFunction::BuiltInFunction_append = BuiltInFunction("append");
BuiltInFunction BuiltInFunction::BuiltInFunction_pop = BuiltInFunction("pop");
BuiltInFunction BuiltInFunction::BuiltInFunction_extend = BuiltInFunction("extend");

std::pair<std::shared_ptr<Node>, Error> run(std::string fn, std::string text) {
    global_symbol_table.set("NULL", std::make_shared<Number>(Number::null_));
    global_symbol_table.set("TRUE", std::make_shared<Number>(Number::true_));
    global_symbol_table.set("FALSE", std::make_shared<Number>(Number::false_));
    global_symbol_table.set("MATH_PI", std::make_shared<Number>(Number::math_PI_));
    global_symbol_table.set("PRINT", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_print));
    global_symbol_table.set("PRINT_RET", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_print_ret));
    global_symbol_table.set("INPUT", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_input));
    global_symbol_table.set("INPUT_INT", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_input_int));
    global_symbol_table.set("CLEAR", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_clear));
    global_symbol_table.set("CLS", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_clear));
    global_symbol_table.set("IS_NUM", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_is_number));
    global_symbol_table.set("IS_STR", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_is_string));
    global_symbol_table.set("IS_LIST", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_is_list));
    global_symbol_table.set("IS_FUN", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_is_function));
    global_symbol_table.set("APPEND", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_append));
    global_symbol_table.set("POP", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_pop));
    global_symbol_table.set("EXTEND", std::make_shared<BuiltInFunction>(BuiltInFunction::BuiltInFunction_extend));
    // Debug: Starting the run function
    //std::cout << "Starting run function with fn: " << fn << " and text: " << text << std::endl;

    // Generate Tokens
    Lexer lexer(fn, text);
    //std::cout << "Lexer initialized." << std::endl;

    std::pair<std::vector<Token>, Error> result = lexer.make_tokens();
    //std::cout << "Tokens generated." << std::endl;

    std::vector<Token> tokens = result.first;
    Error error = result.second;

    std::cout << "Tokens generated: " << std::endl;

    for (auto x : tokens) {
        std::cout << x.print() << ", ";
	}
    std::cout << std::endl;

    std::shared_ptr<Node> temp;

    if (error.is_error() != "None") {
        std::cout << "Error detected: " << error.is_error() << std::endl;
        return std::make_pair(temp, error);
    }

    // Generate AST
    Parser parser(tokens);

    std::shared_ptr<Node> ast = parser.parse();
    std::cout << "AST generated." << std::endl;

    if(std::dynamic_pointer_cast<ParseResult>(ast)->error.is_error() != "None") return std::make_pair(temp, std::dynamic_pointer_cast<ParseResult>(ast)->error);

    // Assuming std::dynamic_pointer_cast<ParseResult> is valid and has node and error members
    auto parseResult = std::dynamic_pointer_cast<ParseResult>(ast);
   /* if (parseResult) {
        std::cout << "ParseResult node and error extracted." << std::endl;
    }
    else {
        std::cout << "ParseResult cast failed." << std::endl;
    }*/

    // Print AST
    std::cout << "Abstract tree is: " << *(parseResult->node) << std::endl;

    // Run program
    Interpreter interpreter = Interpreter();
    Context contextObj("<program>");
    Context* context = &contextObj;
    context->symbol_table = &global_symbol_table;
    std::cout<<"Pk - Main context: "<<context->display_name<<std::endl;
    RTResult result_runtime = interpreter.visit(std::dynamic_pointer_cast<ParseResult>(ast)->node, context);

    std::shared_ptr<Node> resultNumber = result_runtime.value;

    //std::cout<< "Final context: "<< result_runtime.error.context->display_name << std::endl;

    if(result_runtime.error.is_error() != "None") {
        std::cout << "hi\n";
        std::cout << result_runtime.error.as_string() << std::endl;
        return std::make_pair(temp, Error());
    }

    if(resultNumber->get_class_name()=="Number"){
        if (std::dynamic_pointer_cast<Number>(resultNumber)->is_none) {
            std::cout << "Result is None" << std::endl;
        }
        else {
            std::cout << "Result is: " << *std::dynamic_pointer_cast<Number>(resultNumber) << std::endl;
        }
    }
    else  if (resultNumber->get_class_name() == "String") {
        std::cout << "Result is: " << *std::dynamic_pointer_cast<String>(resultNumber) << std::endl;
    }
    else  if (resultNumber->get_class_name() == "List") {
        std::cout << "Result is: " << *std::dynamic_pointer_cast<List>(resultNumber) << std::endl;
    }

    /*if (result_runtime.error.is_error() != "None") {
        std::cout << result_runtime.error.as_string() << std::endl;
    }
    else {
        std::cout << *ast << std::endl;
    }*/
    //std::cout << "haha\n";

    return std::make_pair(parseResult->node, parseResult->error);
}