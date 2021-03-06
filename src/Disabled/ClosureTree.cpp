#include "Passes/ClosureTree.h"
#include <set>

/*

ClosureTree::ClosureTree(DataFlowGraph* program)
: _program(program)
, _parents(program->nodes().size())
{
	for(uint i =0; i < _program->nodes().size(); ++i)
		_parents[i] = -1;
}

ClosureTree::~ClosureTree()
{
}

void ClosureTree::calculate()
{
	for(uint i = 0; i < _program->nodes().size(); ++i) {
		if(_parents[i] != -1)
			continue;
		std::shared_ptr<Node> node = _program->nodes()[i];
		if(node->type() != NodeType::Closure)
			continue;
		_parent = i;
		
		std::set<std::shared_ptr<Node>> out;
		
		recurse(node);
	}
}

void ClosureTree::recurse(Node* node)
{
	//for(int o = 0; o < node->outArity();
}

*/
