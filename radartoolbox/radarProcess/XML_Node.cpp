#include "XML_Node.h"
#include "ParamIO.h"

XML_Node::XML_Node() :
_parent(0)
{
}

XML_Node::XML_Node(const std::string &name, const std::string &attributes, XML_Node *parent) :
_name(name),
_attributes(attributes),
_parent(parent)
{
}

void XML_Node::set(const std::string &name, const std::string &attributes, XML_Node *parent)
{
	_name = name;
	_attributes = attributes;
	_parent = parent;
}


void XML_Node::addElement(const std::string &name, const std::string &value, const std::string &attributes)
{
   Element el(value, attributes);
   elements_iterator it = std::find_if(_elements.begin(), _elements.end(), FindElement(name));

   if(it == _elements.end())
   {
      _elements.push_back(std::make_pair(name, el));
   }
   else
   {
      it->second = el;
   }
}

XML_Node *XML_Node::addNode(const std::string &name, const std::string &attributes)
{
	XML_Node newNode(name, attributes, this);
	_nodes.push_back(newNode);
	return &(_nodes.back());
}

bool XML_Node::getElement(const std::string &name, std::string &value, std::string &attributes)
{
   elements_iterator it = std::find_if(_elements.begin(), _elements.end(), FindElement(name));
	
	if(it != _elements.end())
	{
		value      = it->second.value;
		attributes = it->second.attributes;
		return true;
	}

	return false;
}

bool XML_Node::setElementValue(const std::string &name, const std::string &value)
{
   elements_iterator it = std::find_if(_elements.begin(), _elements.end(), FindElement(name));
	
	if(it != _elements.end())
	{
   	it->second.value = value;
		return true;
	}

	return false;
}

XML_Node::elements_const_iterator XML_Node::beginElements() const
{
	return _elements.begin();
}

XML_Node::elements_const_iterator XML_Node::endElements() const
{
	return _elements.end();
}

XML_Node::nodes_const_iterator XML_Node::beginNodes() const
{
	return _nodes.begin();
}

XML_Node::nodes_const_iterator XML_Node::endNodes() const
{
	return _nodes.end();
}

XML_Node::nodes_iterator XML_Node::beginNodes()
{
	return _nodes.begin();
}

XML_Node::nodes_iterator XML_Node::endNodes()
{
	return _nodes.end();
}

const std::string &XML_Node::getName() const
{
	return _name;
}

bool XML_Node::extractValue(const std::string &str, std::string &value) const
{
   elements_const_iterator it = std::find_if(_elements.begin(), _elements.end(), FindElement(str));

	if(it == endElements())
	{
		return false;
	}

	value = it->second.value;

	return true;
}

XML_Node *XML_Node::getParent()
{
	return _parent;
}

void XML_Node::print(std::ostream &os) const
{
   static int nTabCount = 1;
   int i;

   for(i=1; i < nTabCount; i++)
   {
      os << "\t";
   }

   nTabCount++;

   os << "<" << _name << ">" << std::endl;

   for(elements_const_iterator it = beginElements(); it != endElements(); it++)
   {
      for(i=1; i < nTabCount; i++)
      {
         os << "\t";
      }

      os << "<" << it->first << ">";
      os << it->second.value;
      os << "</" << it->first << ">" << std::endl;
   }

   for(nodes_const_iterator it2 = beginNodes(); it2 != endNodes(); it2++)
   {
      it2->print(os);
   }

   for(i=1; i < (nTabCount-1); i++)
   {
      os << "\t";
   }

   os << "</" << _name << ">" << std::endl;

   nTabCount--;
}

void XML_Node::clear()
{
	_name = std::string();
	_attributes = std::string();
	_parent = 0;
	_elements.clear();
	_nodes.clear();
}


XML_Node::~XML_Node()
{
	clear();
}

XML_Node::nodes_const_iterator XML_Node::findNode(const std::string &name) const
{
   return std::find_if(beginNodes(), endNodes(), FindNode(name));
}

XML_Node::nodes_iterator XML_Node::findNode(const std::string &name)
{
   return std::find_if(beginNodes(), endNodes(), FindNode(name));
}


// XML_Param_Notify
XML_Param_Notify::XML_Param_Notify():
	_currentNode(0)
{
}

XML_Param_Notify::~XML_Param_Notify()
{
	clear();
}

void XML_Param_Notify::clear()
{
	_node.clear();
}

XML_Node::nodes_iterator XML_Param_Notify::top()
{
	return static_cast<XML_Node::nodes_iterator>(&_node);
}

XML_Node::nodes_const_iterator XML_Param_Notify::getNode(std::vector<std::string> &strs) const
{
	// First we need to find the final node
	XML_Node::nodes_const_iterator res, end, begin;

	begin = static_cast<XML_Node::nodes_const_iterator>(&_node);
	end   = static_cast<XML_Node::nodes_const_iterator>(&(_node) + 1);
   res = begin;
	for(int i=0; i<strs.size(); i++)
	{
		res = std::find_if(begin, end, FindNode(strs[i]));
		if(res == end)
		{
			// Couldn't find the proper parameter
			return static_cast<XML_Node::nodes_const_iterator>(0); 
		}
		begin = res->beginNodes();
		end   = res->endNodes();
	}

	return res;
}

XML_Node::nodes_iterator XML_Param_Notify::getNode(std::vector<std::string> &strs)
{
	// First we need to find the final node
	XML_Node::nodes_iterator res, end, begin;

	begin = static_cast<XML_Node::nodes_iterator>(&_node);
	end   = static_cast<XML_Node::nodes_iterator>(&(_node) + 1);
   res = begin;
	for(int i=0; i<strs.size(); i++)
	{
		res = std::find_if(begin, end, FindNode(strs[i]));
		if(res == end)
		{
			// Couldn't find the proper parameter
			return static_cast<XML_Node::nodes_iterator>(0); 
		}
		begin = res->beginNodes();
		end   = res->endNodes();
	}

	return res;
}

void XML_Param_Notify::print(std::ostream &os) const
{
	_node.print(os);
}

// notify methods
void XML_Param_Notify::foundNode( std::string & name, std::string & attributes )
{
	if(_currentNode != 0)
	{
		_currentNode = _currentNode->addNode(name, attributes);
	}
	else
	{
		_node.set(name, attributes, 0);
		_currentNode = &_node;
	}
}

void XML_Param_Notify::endNode( std::string & name, std::string & attributes )
{
	_currentNode = _currentNode->getParent();
}

void XML_Param_Notify::foundElement( std::string & name, std::string & value, std::string & attributes )
{
	_currentNode->addElement(name, value, attributes);
}

bool XML_Param_Notify::compare(const ParamIO &old, std::vector<std::string> &strs) const
{
	// First we need to check if it's a subtree or an element
	XML_Node::nodes_const_iterator  res = getNode(strs);

	if(res == static_cast<XML_Node::nodes_const_iterator>(0))
	{
      // Not an existing node, it's either an element, either a wrong access
		return compareElement(old, strs);
	}
   else
   {
		return compareSubTree(old, strs);
   }

   // Useless
	return false;
}

bool XML_Param_Notify::compareElement(const ParamIO &old, std::vector<std::string> &strs) const
{
	// First we need to find the final node
	std::string paramName = strs.back();
	strs.pop_back();
	XML_Node::nodes_const_iterator  it    = getNode(strs);
	XML_Node::nodes_const_iterator  itOld = old.getTree().getNode(strs);

	if(it    == static_cast<XML_Node::nodes_const_iterator>(0) && 
      itOld == static_cast<XML_Node::nodes_const_iterator>(0))
	{
      // The element doesn't exist in both trees = >they are identical
		return true;
	}

   if(it    == static_cast<XML_Node::nodes_const_iterator>(0) || 
      itOld == static_cast<XML_Node::nodes_const_iterator>(0))
   {
      // The element exists only in one tree => they are different
      return false;
   }

   std::string value, oldValue;
	bool ret = it->extractValue(paramName, value);
	bool retOld = itOld->extractValue(paramName, oldValue);

   if(ret == false && retOld == false)
   {
      // The node doesn't exist in both cases
      return true;
   }

   if(ret == false || retOld == false)
   {
      // Only one of them doesn't have this node
      return false;
   }

   if(value.compare(oldValue) == 0)
   {
      return true;
   }

   return false;
}

// TODO
bool XML_Param_Notify::compareSubTree(const ParamIO &old, std::vector<std::string> &strs) const
{
   XML_Node::nodes_const_iterator  it = getNode(strs);
	XML_Node::nodes_const_iterator  itOld = old.getTree().getNode(strs);

	if(it    == static_cast<XML_Node::nodes_const_iterator>(0) && 
      itOld == static_cast<XML_Node::nodes_const_iterator>(0))
	{
      // The subtree doesn't exist in both trees = >they are identical
		return true;
	}

   if(it    == static_cast<XML_Node::nodes_const_iterator>(0) || 
      itOld == static_cast<XML_Node::nodes_const_iterator>(0))
   {
      // The subtree exists only in one tree => they are different
      return false;
   }

   // Now compare subtrees

   // 1st compare the elements
   if(compareAllElements(it, itOld) == false)
   {
      return false;
   }
   
   // 2nd compare the children nodes
   if(compareAllChildren(it, itOld) == false)
   {
      return false;
   }

   return true;
}

bool XML_Param_Notify::compareAllElements(XML_Node::nodes_const_iterator  it, XML_Node::nodes_const_iterator  itOld) const
{
   // Check if they have the same number of elements
   int nbElem    = std::distance(it->beginElements(),    it->endElements());
   int nbElemOld = std::distance(itOld->beginElements(), itOld->endElements());

   if(nbElem != nbElemOld)
   {
      return false;
   }

   if(nbElem == 0)
   {
      // They both have no element
      return true;
   }

   // Compare the values of the elements
	for(XML_Node::elements_const_iterator elemIt = it->beginElements(); elemIt != it->endElements(); elemIt++)
   {
      std::string value = elemIt->second.value;
      std::string valueOld;

		bool res = itOld->extractValue(elemIt->first, valueOld);

      if(res == false || value.compare(valueOld)!=0)
      {
         return false;
      }
   }

   return true;
}

bool XML_Param_Notify::compareAllChildren(XML_Node::nodes_const_iterator  it, XML_Node::nodes_const_iterator  itOld) const
{
   // Check if they have the same number of elements
   int nb    = std::distance(it->beginNodes(),    it->endNodes());
   int nbOld = std::distance(itOld->beginNodes(), itOld->endNodes());

   if(nb != nbOld)
   {
      return false;
   }

   if(nb == 0)
   {
      // They both have no element
      return true;
   }

   // Compare the children
	for(XML_Node::nodes_const_iterator nodeIt = it->beginNodes(); nodeIt != it->endNodes(); nodeIt++)
   {
      XML_Node::nodes_const_iterator nodeItOld = itOld->findNode(nodeIt->getName());

      if(nodeItOld == itOld->endNodes())
      {
         // One node exists in it and not itOld
         return false;
      }

      if(compareAllElements(nodeIt, nodeItOld) == false)
      {
         // Subtree are not identical
         return false;
      }

      if(compareAllChildren(nodeIt, nodeItOld) == false)
      {
         // Subtree are not identical
         return false;
      }
   }

   return true;
}

////////// Extract 
bool XML_Param_Notify::extractSubTree(std::vector<std::string> &strs, ParamIO &subtree) const
{
	// First we need to check if it's a subtree or an element
	XML_Node::nodes_const_iterator  res = getNode(strs);

	if(res != static_cast<XML_Node::nodes_const_iterator>(0))
	{
      *(subtree.getTree().top()) = (*res);
      return true;
   }

   // We didn't get a node (either element or wrong access)
	return false;
}

////////// Erase 
bool XML_Param_Notify::eraseSubTree(std::vector<std::string> &strs)
{
    std::string lastAccess = strs.back();
    strs.pop_back();


    // First we need to check if it's a subtree or an element
    XML_Node::nodes_iterator	res = getNode(strs);

    if(res != static_cast<XML_Node::nodes_const_iterator>(0))
    {
        if(res->eraseElement(lastAccess) == true)
        {
            return true;
        }

        if(res->eraseNode(lastAccess) == true)
        {
            return true;
        }
    }

    return false;
}

bool XML_Node::eraseElement(const std::string &name)
{
   elements_iterator it = std::find_if(_elements.begin(), _elements.end(), FindElement(name));
	
	if(it != _elements.end())
	{
        _elements.erase(it);
		return true;
	}

	return false;    
}

bool XML_Node::eraseNode(const std::string &name)
{
    // Find if we want to delete a node or an element
    XML_Node::nodes_iterator it = findNode(name);

    if(it != endNodes())
    {
        _nodes.erase(it);
        return true;
    }

    return false;
}



