#include "Parser.h"
#include <Unicode/convert.h>
#include <Unicode/exceptions.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <functional>
#include <limits>
#include <algorithm>

namespace Parser {

typedef std::vector<std::shared_ptr<Node>> Stack;

std::shared_ptr<Node> lexer(std::wistream& stream)
{
	Stack stack;
	uint indentation_depth = 0;
	uint statement_depth = 0;
	
	// TOKEN: BEGIN MODULE
	//std::wcerr << "BEGIN MODULE\n";
	stack.push_back(std::make_shared<Node>(Module));
	
	while(stream.good()) {
		wchar_t c = stream.get();
		
		// Statements separator and indentation
		if(c == statement_separator) {
			// All sub statements must be closed
			if(statement_depth > 1) {
				throw runtime_error(L"Missing )");
			} else if(statement_depth == 1) {
				
				// TOKEN: END STATEMENT
				//std::wcerr << "END STATEMENT\n";
				assert(stack.back()->kind == Statement);
				stack.pop_back();
				statement_depth = 0;
				
			}
			
			// Indentation mode:
			uint new_identation_depth = 0;
			while(stream.peek() == indentation) {
				++new_identation_depth;
				stream.get();
			}
			// wcerr << "INDENTATION " << new_identation_depth << "\n";
			if(new_identation_depth > indentation_depth + 1) {
				throw runtime_error(L"Can not open more than one scope at a time.");
			} else if(new_identation_depth == indentation_depth + 1) {
				// TOKEN: BEGIN SCOPE
				//std::wcerr << "BEGIN SCOPE\n";
				assert(stack.back()->kind == Module || stack.back()->kind == Scope);
				stack.back()->children.push_back(std::make_shared<Node>(Scope));
				stack.push_back(stack.back()->children.back());
				++indentation_depth;
			} else if(new_identation_depth == indentation_depth) {
				// Same scope
			} else {
				while(new_identation_depth < indentation_depth) {
					// TOKEN: END SCOPE
					//std::wcerr << "END SCOPE\n";
					assert(stack.back()->kind == Scope);
					stack.pop_back();
					--indentation_depth;
				}
			}
		} else if(c == indentation) {
			throw runtime_error(L"Can not have indentation here.");
		
		} else if(c == identifier_separator) {
			// Nothing
		
		// Substatements
		} else if(c == substatement_begin) {
			
			if(statement_depth == 0) {
				// TOKEN: BEGIN STATEMENT
				//std::wcerr << "BEGIN STATEMENT\n";
				assert(stack.back()->kind == Module || stack.back()->kind == Scope);
				assert(statement_depth == 0);
				stack.back()->children.push_back(std::make_shared<Node>(Statement));
				stack.push_back(stack.back()->children.back());
				statement_depth = 1;
			}
			
			// TOKEN: BEGIN SUBSTATEMENT
			//std::wcerr << "BEGIN SUB-STATEMENT\n";
			assert(stack.back()->kind == Statement || stack.back()->kind == SubStatement);
			assert(statement_depth >= 1);
			stack.back()->children.push_back(std::make_shared<Node>(SubStatement));
			stack.push_back(stack.back()->children.back());
			++statement_depth;
			
		} else if(c == substatement_end) {
			if(statement_depth <= 1) {
				throw runtime_error(L"Unmatched ) found.");
			}
			
			// TOKEN: END SUBSTATEMENT
			//std::wcerr << "END SUB-STATEMENT\n";
			assert(stack.back()->kind == SubStatement);
			assert(statement_depth >= 2);
			stack.pop_back();
			--statement_depth;
		
		// Quote mode
		} else if(c == quote_begin) {
			auto node = std::make_shared<Node>(Quote);
			uint nesting = 1;
			while(true) {
				if(!stream.good())
					throw runtime_error(L"Found unmatched “.");
				c = stream.get();
				if(c == quote_begin)
					++nesting;
				if(c == quote_end)
					--nesting;
				if(nesting == 0)
					break;
				node->quote += c;
			}
			
			if(statement_depth == 0) {
				// TOKEN: BEGIN STATEMENT
				//std::wcerr << "BEGIN STATEMENT\n";
				assert(stack.back()->kind == Module || stack.back()->kind == Scope);
				assert(statement_depth == 0);
				stack.back()->children.push_back(std::make_shared<Node>(Statement));
				stack.push_back(stack.back()->children.back());
				statement_depth = 1;
			}
			
			// TOKEN: QUOTE(content)
			//std::wcerr << "QUOTE \"" << node->identifier << "\"\n";
			assert(stack.back()->kind == Statement || stack.back()->kind == SubStatement);
			stack.back()->children.push_back(node);
			
		} else if(c == quote_end) {
			throw runtime_error(L"Found unmatched ”.");
		
		// Identifier mode
		} else {
			auto node = std::make_shared<Node>(Identifier);
			node->identifier += c;
			while(true) {
				if(!stream.good())
					break;
				c = stream.peek();
				if(c == statement_separator || c == identifier_separator || c == substatement_end)
					break;
				c = stream.get();
				if(c == quote_begin || c == quote_end || c == indentation)
					throw runtime_error(L"Unexpected character.");
				node->identifier += c;
			}
			
			if(statement_depth == 0) {
				// TOKEN: BEGIN STATEMENT
				//std::wcerr << "BEGIN STATEMENT\n";
				assert(stack.back()->kind == Module || stack.back()->kind == Scope);
				assert(statement_depth == 0);
				stack.back()->children.push_back(std::make_shared<Node>(Statement));
				stack.push_back(stack.back()->children.back());
				statement_depth = 1;
			}
			
			// TOKEN: IDENTIFIER(content)
			//std::wcerr << "IDENTIFIER " << node->identifier << "\n";
			assert(stack.back()->kind == Statement || stack.back()->kind == SubStatement);
			stack.back()->children.push_back(node);
		}
	}
	if(statement_depth == 1) {
		// TOKEN: END STATEMENT
		//std::wcerr << "END STATEMENT\n";
		assert(stack.back()->kind == Statement);
		stack.pop_back();
	}
	while(indentation_depth--) {
		// TOKEN: END SCOPE
		//std::wcerr << "END SCOPE\n";
		assert(stack.back()->kind == Scope);
		stack.pop_back();
	}
	
	// TOKEN: END MODULE
	//std::wcerr << "END MODULE\n";
	assert(stack.back()->kind == Module);
	return stack.back();
}

std::shared_ptr<Node> parseFile(const std::wstring& filename)
{
	std::wifstream stream(encodeLocal(filename));
	std::shared_ptr<Node> n = lexer(stream);
	n->filename = filename;
	parse(n);
	return n;
}

std::shared_ptr<Node> parseString(const std::wstring& contents)
{
	std::wstringstream stream(contents);
	std::shared_ptr<Node> n = lexer(stream);
	n->filename = L"<string>";
	parse(n);
	return n;
}

std::shared_ptr<Node> paserStream(std::wistream& stream)
{
	std::shared_ptr<Node> n = lexer(stream);
	n->filename = L"<stream>";
	parse(n);
	return n;
}

void visit(std::shared_ptr<Node> node, std::function<void(Node&)> visitor)
{
	visitor(*node);
	for(const std::shared_ptr<Node>& child: node->children)
		visit(child, visitor);
}

void visit(Stack& stack, std::function<void(const Stack&)> visitor)
{
	visitor(stack);
	const std::shared_ptr<Node>& node = stack.back();
	for(const std::shared_ptr<Node>& child: node->children) {
		stack.push_back(child);
		visit(stack, visitor);
		stack.pop_back();
	}
}

void visit(std::shared_ptr<Node>node, std::function<void(const Stack&)> visitor)
{
	Stack stack;
	stack.push_back(node);
	visit(stack, visitor);
}

void tag(std::shared_ptr<Node> module)
{
	visit(module, [](Node& node) {
		if((node.kind == Statement || node.kind == SubStatement)
			&& node.children.size() >= 1
			&& node.children[0]->kind == Identifier
			&& node.children[0]->identifier.size() == 1
			&& node.children[0]->identifier[0] == closure) {
			node.is_closure = true;
			node.children[0]->is_closure = true;
			for(uint i = 1; i < node.children.size(); ++i) {
				node.children[i]->is_binding_site = true;
			}
		}
	});
}

void deanonymize(std::shared_ptr<Node> module)
{
	// Give all substatements a unique identifier.
	uint counter = 1;
	const std::wstring base(L"anon-");
	visit(module, [&](Node& node) {
		if(node.kind == SubStatement) {
			std::wstringstream name;
			name << base;
			name << counter;
			node.identifier = name.str();
			++counter;
		}
	});
}

void undebruijn(std::shared_ptr<Node> module)
{
	constexpr uint none = std::numeric_limits<uint>().max();
	visit(module, [](const Stack& stack) {
		const std::shared_ptr<Node>& node = stack.back();
		if(node->kind == Identifier
			&& node->identifier.size() == 1) {
			uint number = none;
			switch(node->identifier[0]) {
				case de_bruijn_1: number = 1; break;
				case de_bruijn_2: number = 2; break;
				default: break;
			}
			if(number != none) {
				assert(number < stack.size());
				const uint index = stack.size() - number - 1;
				assert(stack[index]->kind = SubStatement);
				
				if(node->is_binding_site) {
					assert(!stack[index]->is_binding_site);
					stack[index]->binding_site = node;
				} else {
					assert(stack[index]->is_binding_site);
					node->binding_site = stack[index];
				}
			}
		}
	});
}

std::shared_ptr<Node> find_symbol(const std::wstring& identifier, std::shared_ptr<Node> node)
{
	if(node->kind == Identifier
		&& node->is_binding_site
		&& node->identifier == identifier)
		return node;
	if(node->kind == Statement || node->kind == SubStatement) {
		for(std::shared_ptr<Node> child: node->children) {
			std::shared_ptr<Node> binding_site = find_symbol(identifier, child);
			if(binding_site)
				return binding_site;
		}
	}
	return std::shared_ptr<Node>();
}

void bind(std::shared_ptr<Node> module)
{
	// Bind identifiers
	visit(module, [](const Stack& stack) {
		const std::shared_ptr<Node>& node = stack.back();
		if(node->kind != Identifier || node->is_binding_site || node->is_closure
			|| node->binding_site.lock())
			return;
		
		// Id node
		const std::shared_ptr<Node> id_node = node;
		const std::wstring id = id_node->identifier;
		
		// Binding node
		std::shared_ptr<Node> binding_node;
		
		// Go up the stack
		assert(stack.size() >= 2);
		uint stack_index = stack.size() - 2;
		
		while(stack_index < stack.size()) {
			
			// Find out position in the stack
			const std::shared_ptr<Node> child = stack[stack_index + 1];
			const std::shared_ptr<Node> parent = stack[stack_index];
			uint child_index = std::distance(parent->children.begin(), std::find(
				parent->children.begin(), parent->children.end(), child));
			assert(child_index < parent->children.size());
			
			// Scan backward
			for(uint i = child_index - 1; i < parent->children.size(); --i) {
				binding_node = find_symbol(id, parent->children[i]);
				if(binding_node)
					break;
			}
			if(binding_node)
				break;
			
			// Scan forward
			for(uint i = child_index + 1; i < parent->children.size(); ++i) {
				binding_node = find_symbol(id, parent->children[i]);
				if(binding_node)
					break;
			}
			if(binding_node)
				break;
				
			// Go up and try again
			--stack_index;
		}
		
		// Upsert a global symbol
		if(!binding_node) {
			std::shared_ptr<Node> module = stack[0];
			assert(module->kind == Module);
			
			for(std::shared_ptr<Node> global: module->globals) {
				if(global->identifier == id) {
					binding_node = global;
					break;
				}
			}
			if(!binding_node) {
				binding_node = std::make_shared<Node>(Identifier);
				binding_node->identifier = id;
				binding_node->is_binding_site = true;
				module->globals.push_back(binding_node);
			}
		}
		
		// Bind  the node
		assert(binding_node);
		id_node->binding_site = binding_node;
		
	});
}

void parse(std::shared_ptr<Node> module)
{
	tag(module);
	deanonymize(module);
	undebruijn(module);
	bind(module);
}



} // namespace Parser
