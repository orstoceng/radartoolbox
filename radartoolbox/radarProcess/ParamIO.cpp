#include "ParamIO.h"

void ParamIO::parseAccess(const char *str, std::vector<std::string> &access)
{
	access.clear();

   if(str == 0 || strlen(str) == 0)
   {
      return;
   }

	std::istringstream stream(str);

	char *char_str = new char[strlen(str)];

	while(stream.getline(char_str, strlen(str), ':'))
	{
		access.push_back(std::string(char_str));
	}

	delete[] char_str;
}

void ParamIO::readStream(std::istream &is)
{
	std::string str;

	// Copy file content to string
	char c;

	is.get(c);
	while(is.eof() == false)
	{
		str += c;
		is.get(c);
	}

	_treeBuilder.clear();
	_xml.setSubscriber(_treeBuilder);

	// Parse XML
	_xml.parse(str.c_str(), str.size());
}

void ParamIO::writeStream(std::ostream &os) const
{
	_treeBuilder.print(os);
}

void ParamIO::readFile(const char *filename)
{
	std::ifstream fileIn;
	fileIn.open(filename);
	if(fileIn)
	{
		readStream(fileIn);
		fileIn.close();
	}
}

void ParamIO::writeFile(const char *filename) const
{
	std::ofstream os(filename);
	writeStream(os);
	os.close();
}

bool ParamIO::compare(const ParamIO &old, const char *str) const
{
	std::vector<std::string> strs;
   parseAccess(str, strs);

	return _treeBuilder.compare(old, strs);
}

bool ParamIO::extract(const char *str, ParamIO &subtree) const
{
	std::vector<std::string> strs;
   parseAccess(str, strs);

   if(strs.size() == 0)
   {
       return false;
   }

   subtree.clear();

   return _treeBuilder.extractSubTree(strs, subtree);
}

bool ParamIO::erase(const char *str)
{
    std::vector<std::string> strs;
    parseAccess(str, strs);

   if(strs.size() == 0)
   {
       return false;
   }

   return _treeBuilder.eraseSubTree(strs);
}
