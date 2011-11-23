#ifndef XML_NODE_H
#define XML_NODE_H

#pragma warning (disable : 4786)

#include <string>
#include <vector>
#include <algorithm>

#include "XmlNotify.h"
class ParamIO;

class Element
{
   public:

      Element()
      {
      }

      Element(const std::string &v, const std::string &a):
         value(v),
         attributes(a)
      {
      }

      std::string value;
      std::string attributes;
};

class FindElement
{
public:

   FindElement(const std::string &str) : _searchStr(str)
   {
   }

   bool operator()(const std::pair<std::string, Element> &p) const
   {
      return p.first == _searchStr;
   }

   std::string _searchStr;
};

class FindNode;

class XML_Node
{
	public:

		typedef std::vector<XML_Node>::const_iterator nodes_const_iterator;
		typedef std::vector<XML_Node>::iterator nodes_iterator;

		typedef std::vector< std::pair<std::string, Element > >::iterator elements_iterator;
		typedef std::vector< std::pair<std::string, Element > >::const_iterator elements_const_iterator;

		XML_Node();

		XML_Node(const std::string &name, const std::string &attributes, XML_Node *parent);

		void set(const std::string &name, const std::string &attributes, XML_Node *parent);

		void addElement(const std::string &name, const std::string &value, const std::string &attributes);
		XML_Node *addNode(const std::string &name, const std::string &attributes);

		bool getElement(const std::string &name, std::string &value, std::string &attributes);
		bool setElementValue(const std::string &name, const std::string &value);

		elements_const_iterator beginElements() const;
		elements_const_iterator endElements() const;

		nodes_const_iterator beginNodes() const;
		nodes_const_iterator endNodes() const;

		nodes_iterator beginNodes();
		nodes_iterator endNodes();

		nodes_const_iterator findNode(const std::string &name) const;
		nodes_iterator findNode(const std::string &name);

		const std::string &getName() const;

		template<class T>
		bool extractValue(const std::string &str, T &value) const
		{
         elements_const_iterator it = std::find_if(_elements.begin(), _elements.end(), FindElement(str));
			if(it == endElements())
			{
				return false;
			}

			std::stringstream ist(it->second.value);
			ist >> value;

			return ist.fail()==0;
		}

		// We need a specific method in std::string case so that
		// we can get space-separated words
		bool extractValue(const std::string &str, std::string &value) const;

		XML_Node *getParent();

		void print(std::ostream &os) const;

		void clear();

        bool eraseElement(const std::string &name);
        bool eraseNode(const std::string &name);


		~XML_Node();


	private:

		// In case we have no child node
        std::vector< std::pair<std::string, Element > > _elements;
		std::string _name, _attributes;

		// In case we have child node
		std::vector<XML_Node> _nodes;

		// Parent
		XML_Node* _parent;
};

class XML_Param_Notify : public XmlNotify
{
public:

	XML_Param_Notify();

	virtual ~XML_Param_Notify();

	void clear();

	XML_Node::nodes_iterator top();

	template<class T>
		bool extractValue(std::vector<std::string> strs, T &value) const
	{
		// First we need to find the final node
		std::string paramName = strs.back();
		strs.pop_back();
		XML_Node::nodes_const_iterator res = getNode(strs);
		if(res == static_cast<XML_Node::nodes_const_iterator>(0))
		{
			return false;
		}

		// We found the last node, find the element
		return res->extractValue(paramName, value);
	}

	XML_Node::nodes_const_iterator  getNode(std::vector<std::string> &strs) const;
	XML_Node::nodes_iterator  getNode(std::vector<std::string> &strs);

	template<class T>
	bool	addElement(std::vector<std::string> &strs, T &value)
	{
		// First we need to find the final node
		XML_Node::nodes_iterator end, begin;
		XML_Node::nodes_iterator res, prevRes;
		unsigned int i;

		begin = static_cast<XML_Node::nodes_iterator>(&_node);
		end   = static_cast<XML_Node::nodes_iterator>(&(_node) + 1);
		bool go_out = false;

		res = prevRes = static_cast<XML_Node::nodes_iterator>(0);
		for(i=0; i<strs.size()-1 && go_out==false; i++)
		{
			prevRes = res;

			res = std::find_if(begin, end, FindNode(strs[i]));
			if(res == end)
			{
				// Couldn't find the proper parameter
				go_out = true;
			}
			else
			{
				begin = res->beginNodes();
				end   = res->endNodes();
			}
		}

		// Do we need to create any node?
		if(go_out == true)
		{
			i--;
			// We need to create new nodes
			for(;i<strs.size()-1; i++)
			{
				if(prevRes == static_cast<XML_Node::nodes_iterator>(0))
				{
					// It means the tree is empty
					_node.set(strs[i], std::string(), 0);
					prevRes = static_cast<XML_Node::nodes_iterator>(&(_node));
				}
				else
				{
					prevRes = static_cast<XML_Node::nodes_iterator>(prevRes->addNode(strs[i], std::string()));
				}
			}
		}
		else
		{
			prevRes = res;
		}

		std::stringstream stream;
		stream << value;/* << std::ends; => adds an useless extra character*/

		prevRes->addElement(strs.back(), stream.str(), std::string());

		return true;
	}

   bool compare(const ParamIO &old, std::vector<std::string> &strs) const;

   bool extractSubTree(std::vector<std::string> &strs, ParamIO &subTree) const;

   bool eraseSubTree(std::vector<std::string> &strs);

	void print(std::ostream &os) const;

	// notify methods
	virtual void foundNode( std::string & name, std::string & attributes );
	virtual void endNode( std::string & name, std::string & attributes );

	virtual void foundElement( std::string & name, std::string & value, std::string & attributes );

private:

   bool compareElement(const ParamIO &old, std::vector<std::string> &strs) const;
   bool compareSubTree(const ParamIO &old, std::vector<std::string> &strs) const;

   bool compareAllElements(XML_Node::nodes_const_iterator  it, XML_Node::nodes_const_iterator  itOld) const;
   bool compareAllChildren(XML_Node::nodes_const_iterator  it, XML_Node::nodes_const_iterator  itOld) const;
	
	XML_Node _node;

	XML_Node *_currentNode;
};

// class used by find_if in findSameName method
// Note : we could have avoided the FindSameName class using a std::string
// converter operator for the XML_Node class. But adding such an operator
// for this purpose didn't seem right.

class FindNode
{
public:
   FindNode(const std::string &name):
      _name(name)
   {
   }

   bool operator()(const XML_Node &node)
   {
      return _name.compare(node.getName()) == 0?true:false;
   }

   std::string _name;
};


#endif
