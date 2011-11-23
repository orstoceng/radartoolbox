#if !defined(XmlUtil_H)
#define XmlUtil_H


/////////////////////////////////////////////////////////////////////////
// define tag markers

#define idTagLeft	"<"
#define idTagRight	">"
#define idTagEnd	"</"
#define idTagNoData "/>"


#define idTagLeftLength		1
#define idTagRightLength	1
#define idTagEndLength		2


#include <string>

//////////////////////////////////////////////////////////////////////////////////
// XmlUtil
//
// Purpose:		provides xml utility methods

class XmlUtil
{

	// tag helper methods
	static std::string getStartTag ( std::string & text )
	{
		std::string tag = idTagLeft;
		tag += text;
		tag += idTagRight;

		return std::string(tag);
	}

	// static helper methods
	static std::string getEndTag ( std::string & text )
	{
		std::string tag = idTagEnd;
		tag += text;
		tag += idTagRight;

		return std::string(tag);
	}

	static std::string getStartTag ( const char *text )
	{
		std::string tag = idTagLeft;
		tag += text;
		tag += idTagRight;

		return std::string(tag);
	}

	static std::string getEndTag ( const char *text )
	{
		std::string tag = idTagEnd;
		tag += text;
		tag += idTagRight;

		return std::string(tag);
	}
};


#endif