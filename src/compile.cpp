#include <Command.h>
#include <Parser/Parser.h>
#include <Parser/Compile.h>

Command compile(L"compile", [](Command::Arguments arguments) {
	assert(arguments.size() == 1);
	
	Parser::Program pp = Parser::compile(Parser::parseFile(arguments[0]));
	Parser::write(std::wcerr, pp);
	Compile::Program cp = Compile::compile(pp);
	Compile::write(std::wcerr, cp);
	
	return Command::success;
},
L"parse a source file",
L"<source file>\n\n"
"The source file is parsed and the resulting structure printed to\n"
"the standard output.\n"
);
