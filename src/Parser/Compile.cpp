#include "Compile.h"
#include <Parser/Parser.h>
namespace Compile {

template<class T>
std::vector<T> set_to_vector(const std::set<T>& set)
{
	std::vector<T> vec;
	vec.reserve(set.size());
	std::copy(set.begin(), set.end(), std::back_inserter(vec));
	std::sort(vec.begin(), vec.end());
	return vec;
}

std::vector<std::set<uint>> calculate_dependencies(const Parser::Program& p)
{
	std::vector<std::set<uint>> hierarchy(p.calls.size());
	
	// Initialize
	for(uint i = 0; i < p.calls.size(); ++i) {
		auto call = p.calls[i];
		for(uint j = 0; j <  call.size(); ++j) {
			auto ref = call[j];
			if(ref.first != 0 && ref.first != 1 && ref.second != 0)
				hierarchy[i].insert(ref.first);
		}
	}
	
	return hierarchy;
}

void print(std::wostream& out, const Parser::Program& p,
	const std::vector<Symbol>& call)
{
	std::vector<std::wstring> vec;
	for(const auto c: call)
		if(p.symbols.find(c) != p.symbols.end())
			vec.push_back(p.symbols.at(c));
		else
			vec.push_back(L"-");
	out << vec << "\n";
}

std::vector<std::vector<Symbol>> calculate_closures(
	const Parser::Program& p)
{
	const auto hierarchy = calculate_dependencies(p);
	
	std::vector<std::set<Symbol>> closures(p.closures.size());
	for(uint closure = 0; closure < p.closures.size(); ++closure) {
		std::vector<Symbol> call = p.calls[p.closure_call[closure]];
		for(auto bind: call) {
			if(bind.first == 0 || bind.first == 1)
				continue;
			if(bind.first == closure + 2)
				continue;
			if(bind.second == 0)
				continue;
			closures[closure].insert(bind);
		}
	}
	// std::wcerr << closures << "\n";
	
	// Itteratively add closures
	bool done = false;
	while(!done) {
		done = true;
		for(uint closure = 0; closure < p.closures.size(); ++closure) {
			std::vector<Symbol> call = p.calls[p.closure_call[closure]];
			for(auto bind: call) {
				if(bind.first == 0 || bind.first == 1)
					continue;
				if(bind.first == closure + 2)
					continue;
				if(bind.second != 0)
					continue;
				for(auto dep: closures[bind.first - 2]) {
					if(dep.first == closure + 2)
						continue;
					if(closures[closure].insert(dep).second)
						done = false;
				}
			}
		}
	}
	// std::wcerr << closures << "\n";
	
	std::vector<std::vector<Symbol>> sorted;
	for(auto set: closures)
		sorted.push_back(set_to_vector(set));
	// std::wcerr << sorted << "\n";
	
	return sorted;
}

std::vector<std::vector<uint>> calculate_allocs(const Parser::Program& p)
{
	const auto closures = calculate_closures(p);
	
	std::vector<std::vector<uint>> allocs;
	for(uint closure = 0; closure < p.closures.size(); ++closure) {
		const auto call = p.calls[p.closure_call[closure]];
		std::set<uint> alloc;
		for(auto arg: call) {
			if(arg.first == 0 || arg.first == 1 || arg.first == closure + 2)
				continue;
			if(arg.second != 0)
				continue;
			//std::wcerr << arg << " <- " << closures[arg.first - 2] << "\n";
			alloc.insert(arg.first);
		}
		allocs.push_back(set_to_vector(alloc));
	}
	return allocs;
}

Program compile(const Parser::Program& p)
{
	const auto closures = calculate_closures(p);
	const auto allocs = calculate_allocs(p);
	Program program;
	for(uint closure = 0; closure < p.closures.size(); ++closure) {
		Function function;
		function.id = closure + 2;
		function.arity = p.closures[closure];
		if(closure < p.symbols_export.size())
			function.name = p.symbols_export[closure];
		function.closure = closures[closure];
		function.allocations = allocs[closure];
		function.call = p.calls[p.closure_call[closure]];
		for(uint i = 0; i < p.closures[closure]; ++i)
			function.arguments.push_back(std::make_pair(closure + 2, i + 1));
		for(auto arg: function.call) {
			if(arg.first == 0)
				function.imports[arg] = p.symbols_import[arg.second];
			else if(arg.first == 1)
				function.constants[arg] = p.constants[arg.second];
		}
		program.push_back(function);
	}
	return program;
}

void write(std::wostream& out, const Compile::Program& p)
{
	for(const Function& f: p) {
		out << L"λ           " << "(" << f.id << L", 0) " << f.name << "\n";
		out << L"Imports     " << f.imports << "\n";
		out << L"Constants   " << f.constants << "\n";
		out << L"Closure     " << f.closure << "\n";
		out << L"Arguments   " << f.arguments << "\n";
		out << L"Allocations " << f.allocations << "\n";
		out << L"Call        " << f.call << "\n";
		out << L"\n";
	}
}

} // namespace Compile
