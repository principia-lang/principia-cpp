#include "DFG/PropertyType.h"

string PropertyType::toString() const
{
	switch(value)
	{
		case PropertyType::Identifier: return L"Identifier";
		case PropertyType::Source: return L"Source";
		case PropertyType::Constant: return L"Constant";
		case PropertyType::Closure: return L"Closure";
		case PropertyType::Order: return L"Order";
		case PropertyType::Stack: return L"Stack";
		default: return L"Illegal value";
	}
}
