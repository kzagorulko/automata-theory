//
//  Parser.cpp
//  cpp
//
//  Created by Константин on 29.10.2019.
//  Copyright © 2019 Константин. All rights reserved.
//

#include "Parser.hpp"

Parser::Parser(std::list<Token> tokens): pos(0) {
    for (Token t:tokens) {
        if (t.type != SPACE) {
            this->tokens.push_back(Token(t.type, t.text, t.pos));
        }
    }
}

Token* Parser::match(std::set<size_t> types) {
    if (pos < tokens.size()) {
        std::list<Token>::iterator it = tokens.begin();
        std::advance(it, pos);
        if (types.count(it->type) > 0) {
            pos++;
            return &*it;
        }
    }
    return nullptr;
}

Token* Parser::match(size_t type) {
    if (pos < tokens.size()) {
        std::list<Token>::iterator it = tokens.begin();
        std::advance(it, pos);
        if (it->type == type) {
            pos++;
            return &*it;
        }
    }
    return nullptr;
}

Token Parser::require(size_t type) {
    Token* t = match(type);
    if (t == nullptr) {
        throw std::make_tuple("Expected " + std::string(TOKEN_NAME[type]), "Parser::require", pos);
    }
    return *t;
}

ExprNode* Parser::parse_elem() {
    Token* num = match(NUMBER);
    if (num != nullptr) {
        return new NumberNode(num);
    }
    Token* id = match(ID);
    if (id != nullptr) {
        return new VarNode(id);
    }
    throw std::make_tuple("Expected number or variable", "Parser::parse_elem", pos);
    return nullptr;
}

ExprNode* Parser::parse_bracketed() {
    if (match(LPAR) != nullptr) {
        ExprNode* e = parse_expression();
        require(RPAR);
        return e;
    }
    return parse_elem();
}

ExprNode* Parser::parse_summand() {
    ExprNode* e1 = parse_bracketed();
    Token* op;
    std::set<size_t> types {MUL, DIV};
    
    while ((op = match(types)) != nullptr) {
        ExprNode* e2 = parse_bracketed();
        e1 = new BinOpNode(op, e1, e2);
    }
    
    return e1;
}

ExprNode* Parser::parse_logical() {
    ExprNode* e1 = parse_summand();
    Token* op;
    std::set<size_t> types {ADD, SUB};
    
    while ((op = match(types)) != nullptr) {
        ExprNode* e2 = parse_summand();
        e1 = new BinOpNode(op, e1, e2);
    }
    
    return e1;
}

ExprNode* Parser::parse_expression() {
    ExprNode* e1 = parse_logical();
    Token* op;
    std::set<size_t> types {LESS, MORE, EQUAL};
    std::set<size_t> wrong_types {NUMBER, FLOAT_NUMBER, LPAR};
    
    while ((op = match(types)) != nullptr) {
        ExprNode* e2 = parse_logical();
        e1 = new BinOpNode(op, e1, e2);
    }
    
    std::list<Token>::iterator next_token = tokens.begin();
    std::advance(next_token, pos);
    
    if ((pos < tokens.size()) && (op == nullptr) && (match(wrong_types) != nullptr)) {
        throw std::make_tuple("Expected a opertor", "Parser::parce_expression", pos);
    }
    
    return e1;
}

int Parser::eval(ExprNode* node) {
    switch (node->type) {
        case NUMBER_NODE:
            return std::stoi((((NumberNode*) node)->number)->text);
            break;
        case BIN_OP_NODE:
        {
                BinOpNode bon = *((BinOpNode*) node);
                int l = eval((ExprNode *) bon.left);
                int r = eval((ExprNode *) bon.right);
                switch (bon.op->type) {
                    case ADD:   return l + r;
                    case SUB:   return l - r;
                    case MUL:   return l * r;
                    case DIV:   return l / r;
                    case LESS:  return l < r;
                    case MORE:  return l > r;
                    case EQUAL: return l == r;
                }
        }
            break;
            
        case VAR_NODE:
        {
            std::string value;
            
            VarNode vn = *((VarNode*) node);
            std::cout << "Enter value " << vn.var->text << ": ";
            std::cin >> value;
            if (!Utils::is_number(value)) {
                throw std::make_pair("Expected a number", "Parser::eval");
            }
            return std::stof(value);
        }
            break;
            
        default:
            throw std::make_pair("IllegalStateException", "Parser::eval");
            break;
    }
    
    return 1;
}
