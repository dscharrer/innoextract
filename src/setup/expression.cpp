/*
 * Copyright (C) 2012-2019 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "setup/expression.hpp"

#include <stddef.h>
#include <cstring>
#include <vector>
#include <stdexcept>

#include "util/log.hpp"

namespace setup {

namespace {

bool is_identifier_start(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-';
}

bool is_identifier(char c) {
	return is_identifier_start(c) || (c >= '0' && c <= '9') || c == '\\';
}

struct evaluator {
	
	const std::string & test;
	const char * expr;
	
	enum token_type {
		end,
		op_or,
		op_and,
		op_not,
		paren_left,
		paren_right,
		identifier
	} token;
	const char * token_start;
	size_t token_length;
	
	evaluator(const std::string & expression, const std::string & variable)
		: test(variable), expr(expression.c_str()), token(end) { }
	
	token_type next() {
		
		// Ignore whitespace
		while(*expr > 0 && *expr <= 32) {
			expr++;
		}
		
		if(!*expr) {
			return (token = end);
			
		} else if(*expr == '(') {
			return (expr++, token = paren_left);
			
		} else if(*expr == ')') {
			return (expr++, token = paren_right);
			
		} else if(is_identifier_start(*expr)) {
			
			const char * start = expr++;
			while(is_identifier(*expr)) {
				expr++;
			}
			
			if(expr - start == 3 && !memcmp(start, "not", 3)) {
				return (token = op_not);
			} else if(expr - start == 3 && !memcmp(start, "and", 3)) {
				return (token = op_and);
			} else if(expr - start == 2 && !memcmp(start, "or", 2)) {
				return (token = op_or);
			}
			
			token_start = start;
			token_length = size_t(expr - start);
			return (token = identifier);
			
		} else {
			throw std::runtime_error(std::string("unexpected symbol: ") + *expr);
		}
	}
	
	bool eval_identifier(bool lazy) {
		bool result = lazy || test.compare(0, std::string::npos, token_start, token_length) == 0;
		next();
		return result;
	}
	
	bool eval_factor(bool lazy) {
		if(token == paren_left) {
			next();
			bool result = eval_expression(lazy);
			if(token != paren_right) {
				throw std::runtime_error("expected closing parenthesis");
			}
			next();
			return result;
		} else if(token == op_not) {
			next();
			return !eval_factor(lazy);
		} else if(token == identifier) {
			return eval_identifier(lazy);
		} else {
			throw std::runtime_error("unexpected token");
		}
	}
	
	bool eval_term(bool lazy) {
		bool result = eval_factor(lazy);
		while(token == op_and) {
			next();
			result = eval_factor(lazy || !result) && result;
		}
		return result;
	}
	
	bool eval_expression(bool lazy, bool inner = true) {
		bool result = eval_term(lazy);
		if(result && !inner) {
			return result;
		}
		while(token == op_or || token == identifier) {
			if(token == op_or) {
				next();
			}
			result = eval_term(lazy || result) || result;
			if(result && !inner) {
				return result;
			}
		}
		return result;
	}
	
	bool eval() {
		next();
		return eval_expression(false, false);
	}
	
};

} // anonymous namespace

bool expression_match(const std::string & test, const std::string & expression) {
	try {
		return evaluator(expression, test).eval();
	} catch(const std::runtime_error & error) {
		log_warning << "Error evaluating \"" << expression << "\": " << error.what();
		return true;
	}
}

bool is_simple_expression(const std::string & expression) {
	if(expression.empty()) {
		return true;
	}
	const char * c = expression.c_str();
	if(!is_identifier_start(*c)) {
		return false;
	}
	while(*c) {
		if(!is_identifier(*c)) {
			return false;
		}
		c++;
	}
	return true;
}

} // namespace setup
