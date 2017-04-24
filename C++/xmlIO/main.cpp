#include "pugixml.hpp"

#include <string.h>
#include <iostream>
#include <fstream>

void loadFile() {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("tree.xml");
	if (!result) {
		std::cerr << "load file failed.\n";
		return;
	}
	std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;
}

void loadString() {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string("<mesh name='sphere'><bounds>0 0 1 1</bounds></mesh>");	
	if (!result) {
		std::cerr << "load string failed.\n";
		return;
	}
	std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;
}

void loadStream() {
	pugi::xml_document doc;

	{
		std::ifstream stream("weekly-utf-8.xml");
		pugi::xml_parse_result result = doc.load(stream);
		if (!result) {
			std::cerr << "load stream failed.\n";
			return;
		}
		std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;
	}

	{
		std::ifstream stream("weekly-utf-16.xml");
		pugi::xml_parse_result result = doc.load(stream);
		if (!result) {
			std::cerr << "load stream failed.\n";
			return;
		}
		std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;
	}
}

void add() {
	pugi::xml_document doc;

	// add node with some name
	pugi::xml_node node = doc.append_child("node");

	// add description node with text child
	pugi::xml_node descr = node.append_child("description");
	descr.append_child(pugi::node_pcdata).set_value("Simple node");

	// add param node before the description
	pugi::xml_node param = node.insert_child_before("param", descr);

	// add attributes to param node
	param.append_attribute("name") = "version";
	param.append_attribute("value") = 1.1;
	param.insert_attribute_after("type", param.attribute("name")) = "float";	

	doc.print(std::cout);
}

void modify() {
	pugi::xml_document doc;
	if (!doc.load_string("<node id='123'>text</node><!-- comment -->", pugi::parse_default | pugi::parse_comments)) return;
	
	pugi::xml_node node = doc.child("node");

	// change node name
	std::cout << node.set_name("notnode");
	std::cout << ", new node name: " << node.name() << std::endl;

	// change comment text
	std::cout << doc.last_child().set_value("useless comment");
	std::cout << ", new comment text: " << doc.last_child().value() << std::endl;

	// we can't change value of the element or name of the comment
	std::cout << node.set_value("1") << ", " << doc.last_child().set_name("2") << std::endl;
	// end::node[]

	// tag::attr[]
	pugi::xml_attribute attr = node.attribute("id");

	// change attribute name/value
	std::cout << attr.set_name("key") << ", " << attr.set_value("345");
	std::cout << ", new attribute: " << attr.name() << "=" << attr.value() << std::endl;

	// we can use numbers or booleans
	attr.set_value(1.234);
	std::cout << "new attribute value: " << attr.value() << std::endl;

	// we can also use assignment operators for more concise code
	attr = true;
	std::cout << "final attribute value: " << attr.value() << std::endl;

	doc.print(std::cout);
}

void remove() {
	pugi::xml_document doc;
	if (!doc.load_string("<node><description>Simple node</description><param name='id' value='123'/></node>")) return;
	
	// remove description node with the whole subtree
	pugi::xml_node node = doc.child("node");
	node.remove_child("description");

	// remove id attribute
	pugi::xml_node param = node.child("param");
	param.remove_attribute("value");

	// we can also remove nodes/attributes by handles
	pugi::xml_attribute id = param.attribute("name");
	param.remove_attribute(id);	

	doc.print(std::cout);
}

void access() {
	/*< ? xml version = "1.0" encoding = "UTF-8" standalone = "no" ? >
		<Profile FormatVersion = "1">
			<Tools>
			<Tool Filename = "jam" AllowIntercept = "true">
				<Description>Jamplus build system< / Description>
			< / Tool>
			<Tool Filename = "mayabatch.exe" AllowRemote = "true" OutputFileMasks = "*.dae" DeriveCaptionFrom = "lastparam" Timeout = "40" / >
			<Tool Filename = "meshbuilder_*.exe" AllowRemote = "false" OutputFileMasks = "*.mesh" DeriveCaptionFrom = "lastparam" Timeout = "10" / >
			<Tool Filename = "texbuilder_*.exe" AllowRemote = "true" OutputFileMasks = "*.tex" DeriveCaptionFrom = "lastparam" / >
			<Tool Filename = "shaderbuilder_*.exe" AllowRemote = "true" DeriveCaptionFrom = "lastparam" / >
			< / Tools>
		< / Profile>
	*/

	pugi::xml_document doc;
	if (!doc.load_file("xgconsole.xml")) return;

	pugi::xml_node tools = doc.child("Profile").child("Tools");

	// tag::basic[]
	for (pugi::xml_node tool = tools.first_child(); tool; tool = tool.next_sibling())
	{
		std::cout << "Tool:";

		for (pugi::xml_attribute attr = tool.first_attribute(); attr; attr = attr.next_attribute())
		{
			std::cout << " " << attr.name() << "=" << attr.value();
		}

		std::cout << std::endl;
	}
	// end::basic[]

	std::cout << std::endl;

	// tag::data[]
	for (pugi::xml_node tool = tools.child("Tool"); tool; tool = tool.next_sibling("Tool"))
	{
		std::cout << "Tool " << tool.attribute("Filename").value();
		std::cout << ": AllowRemote " << tool.attribute("AllowRemote").as_bool();
		std::cout << ", Timeout " << tool.attribute("Timeout").as_int();
		std::cout << ", Description '" << tool.child_value("Description") << "'\n";
	}
	// end::data[]

	std::cout << std::endl;

	// tag::contents[]
	std::cout << "Tool for *.dae generation: " << tools.find_child_by_attribute("Tool", "OutputFileMasks", "*.dae").attribute("Filename").value() << "\n";

	for (pugi::xml_node tool = tools.child("Tool"); tool; tool = tool.next_sibling("Tool"))
	{
		std::cout << "Tool " << tool.attribute("Filename").value() << "\n";
	}
	// end::contents[]

	std::cout << std::endl;

	// tag::iterator[]
	for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it)
	{
		std::cout << "Tool:";

		for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
		{
			std::cout << " " << ait->name() << "=" << ait->value();
		}

		std::cout << std::endl;
	}
	// end::iterator[]
}

void save() {
	// get a test document
	pugi::xml_document doc;
	doc.load_string("<foo bar='baz'>hey</foo>");
	
	// save document to file
	std::cout << "Saving result: " << doc.save_file("save_file_output.xml") << std::endl;
}

int main()
{
	access();
	return 0;
}

// vim:et
