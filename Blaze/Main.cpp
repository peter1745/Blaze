#include "HTML/Parser.hpp"

#include <fstream>
#include <sstream>
#include <print>

using namespace Blaze;
using namespace CSTM;

int main()
{
	std::ifstream stream("Tests/HTML/Basic.html");
	std::stringstream ss;
	ss << stream.rdbuf();
	stream.close();

	const auto fileData = String::create(ss.view());

	SharedRef doc = HTML::Node::create<HTML::Document>(null).as<HTML::Document>();
	HTML::Parser parser(doc);
	parser.parse(fileData);

	return 0;
}
