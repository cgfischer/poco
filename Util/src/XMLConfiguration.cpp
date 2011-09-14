//
// XMLConfiguration.cpp
//
// $Id: //poco/Main/Util/src/XMLConfiguration.cpp#8 $
//
// Library: Util
// Package: Configuration
// Module:  XMLConfiguration
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Poco/Util/XMLConfiguration.h"
#include "Poco/SAX/InputSource.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Attr.h"
#include "Poco/DOM/Text.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/Exception.h"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include <set>


namespace Poco {
namespace Util {


XMLConfiguration::XMLConfiguration()
{
}


XMLConfiguration::XMLConfiguration(Poco::XML::InputSource* pInputSource)
{
	load(pInputSource);
}


XMLConfiguration::XMLConfiguration(std::istream& istr)
{
	load(istr);
}


XMLConfiguration::XMLConfiguration(const std::string& path)
{
	load(path);
}


XMLConfiguration::XMLConfiguration(const Poco::XML::Document* pDocument)
{
	load(pDocument);
}

	
XMLConfiguration::XMLConfiguration(const Poco::XML::Node* pNode)
{
	load(pNode);
}


XMLConfiguration::~XMLConfiguration()
{
}


void XMLConfiguration::load(Poco::XML::InputSource* pInputSource)
{
	poco_check_ptr (pInputSource);
	
	Poco::XML::DOMParser parser;
	parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, false);
	parser.setFeature(Poco::XML::DOMParser::FEATURE_FILTER_WHITESPACE, true);
	Poco::XML::AutoPtr<Poco::XML::Document> pDoc = parser.parse(pInputSource);
	load(pDoc);
}


void XMLConfiguration::load(std::istream& istr)
{
	Poco::XML::InputSource src(istr);
	load(&src);	
}


void XMLConfiguration::load(const std::string& path)
{
	Poco::XML::InputSource src(path);
	load(&src);	
}

	
void XMLConfiguration::load(const Poco::XML::Document* pDocument)
{
	poco_check_ptr (pDocument);
	
	_pDocument = Poco::XML::AutoPtr<Poco::XML::Document>(const_cast<Poco::XML::Document*>(pDocument), true);
	_pRoot     = Poco::XML::AutoPtr<Poco::XML::Node>(pDocument->documentElement(), true);
}


void XMLConfiguration::load(const Poco::XML::Node* pNode)
{
	poco_check_ptr (pNode);

	if (pNode->nodeType() == Poco::XML::Node::DOCUMENT_NODE)
	{
		load(static_cast<const Poco::XML::Document*>(pNode));
	}
	else
	{
		_pDocument = Poco::XML::AutoPtr<Poco::XML::Document>(pNode->ownerDocument(), true);
		_pRoot     = Poco::XML::AutoPtr<Poco::XML::Node>(const_cast<Poco::XML::Node*>(pNode), true);
	}
}


void XMLConfiguration::loadEmpty(const std::string& rootElementName)
{
	_pDocument = new Poco::XML::Document;
	_pRoot     = _pDocument->createElement(rootElementName);
	_pDocument->appendChild(_pRoot);
}


void XMLConfiguration::save(const std::string& path) const
{
	Poco::XML::DOMWriter writer;
	writer.setNewLine("\n");
	writer.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
	writer.writeNode(path, _pDocument);
}


void XMLConfiguration::save(std::ostream& ostr) const
{
	Poco::XML::DOMWriter writer;
	writer.setNewLine("\n");
	writer.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
	writer.writeNode(ostr, _pDocument);
}


void XMLConfiguration::save(Poco::XML::DOMWriter& writer, const std::string& path) const
{
	writer.writeNode(path, _pDocument);
}


void XMLConfiguration::save(Poco::XML::DOMWriter& writer, std::ostream& ostr) const
{
	writer.writeNode(ostr, _pDocument);
}


bool XMLConfiguration::getRaw(const std::string& key, std::string& value) const
{
	const Poco::XML::Node* pNode = findNode(key);
	if (pNode)
	{
		value = pNode->innerText();
		return true;
	}
	else return false;
}


void XMLConfiguration::setRaw(const std::string& key, const std::string& value)
{
	std::string::const_iterator it = key.begin();
	Poco::XML::Node* pNode = findNode(it, key.end(), _pRoot, true);
	if (pNode)
	{
        unsigned short nodeType = pNode->nodeType();
        if (Poco::XML::Node::ATTRIBUTE_NODE == nodeType)
        {
            pNode->setNodeValue(value);
        }
        else if (Poco::XML::Node::ELEMENT_NODE == nodeType)
        {
            Poco::XML::Node* pChildNode = pNode->firstChild();
            if (pChildNode)
            {
                if (Poco::XML::Node::TEXT_NODE == pChildNode->nodeType())
                {
                    pChildNode->setNodeValue(value);
                }
            }
            else
            {
				Poco::AutoPtr<Poco::XML::Node> pText = _pDocument->createTextNode(value);
				pNode->appendChild(pText);
            }
        }
	}
    else throw NotFoundException("Node not found in XMLConfiguration", key);
}


void XMLConfiguration::enumerate(const std::string& key, Keys& range) const
{
	using Poco::NumberFormatter;
	
	std::multiset<std::string> keys;
	const Poco::XML::Node* pNode = findNode(key);
	if (pNode)
	{
		const Poco::XML::Node* pChild = pNode->firstChild();
		while (pChild)
		{
			if (pChild->nodeType() == Poco::XML::Node::ELEMENT_NODE)
			{
				const std::string& nodeName = pChild->nodeName();
				int n = (int) keys.count(nodeName);
				if (n)
					range.push_back(nodeName + "[" + NumberFormatter::format(n) + "]");
				else
					range.push_back(nodeName);
				keys.insert(nodeName);
			}
			pChild = pChild->nextSibling();
		}
	}
}


void XMLConfiguration::removeRaw(const std::string& key)
{
	Poco::XML::Node* pNode = findNode(key);

	if (pNode)
	{
		if (pNode->nodeType() == Poco::XML::Node::ELEMENT_NODE)
		{
			Poco::XML::Node* pParent = pNode->parentNode();
			if (pParent)
			{
				pParent->removeChild(pNode);
			}
		}
		else if (pNode->nodeType() == Poco::XML::Node::ATTRIBUTE_NODE)
		{
			Poco::XML::Attr* pAttr = dynamic_cast<Poco::XML::Attr*>(pNode);
			Poco::XML::Element* pOwner = pAttr->ownerElement();
			if (pOwner)
			{
				pOwner->removeAttributeNode(pAttr);
			}
		}
	}
}


const Poco::XML::Node* XMLConfiguration::findNode(const std::string& key) const
{
	std::string::const_iterator it = key.begin();
	Poco::XML::Node* pRoot = const_cast<Poco::XML::Node*>(_pRoot.get());
	return findNode(it, key.end(), pRoot);
}


Poco::XML::Node* XMLConfiguration::findNode(const std::string& key)
{
	std::string::const_iterator it = key.begin();
	Poco::XML::Node* pRoot = const_cast<Poco::XML::Node*>(_pRoot.get());
	return findNode(it, key.end(), pRoot);
}


Poco::XML::Node* XMLConfiguration::findNode(std::string::const_iterator& it, const std::string::const_iterator& end, Poco::XML::Node* pNode, bool create)
{
	if (pNode && it != end)
	{
		if (*it == '[')
		{
			++it;
			if (it != end && *it == '@')
			{
				++it;
				std::string attr;
				while (it != end && *it != ']' && *it != '=') attr += *it++;
				if (it != end && *it == '=')
				{
					++it;
					std::string value;
					if (it != end && *it == '\'')
					{
						++it;
						while (it != end && *it != '\'') value += *it++;
						if (it != end) ++it;
					}
					else
					{
						while (it != end && *it != ']') value += *it++;
					}
					if (it != end) ++it;
					return findNode(it, end, findElement(attr, value, pNode), create);
				}
				else
				{
					if (it != end) ++it;
					return findAttribute(attr, pNode, create);
				}
			}
			else
			{
				std::string index;
				while (it != end && *it != ']') index += *it++;
				if (it != end) ++it;
				return findNode(it, end, findElement(Poco::NumberParser::parse(index), pNode, create), create);
			}
		}
		else
		{
			while (it != end && *it == '.') ++it;
			std::string key;
			while (it != end && *it != '.' && *it != '[') key += *it++;
			return findNode(it, end, findElement(key, pNode, create), create);
		}
	}
	else return pNode;
}


Poco::XML::Node* XMLConfiguration::findElement(const std::string& name, Poco::XML::Node* pNode, bool create)
{
	Poco::XML::Node* pChild = pNode->firstChild();
	while (pChild)
	{
		if (pChild->nodeType() == Poco::XML::Node::ELEMENT_NODE && pChild->nodeName() == name)
			return pChild;
		pChild = pChild->nextSibling();
	}
	if (create)
	{
		Poco::AutoPtr<Poco::XML::Element> pElem = pNode->ownerDocument()->createElement(name);
		pNode->appendChild(pElem);
		return pElem;
	}
	else return 0;
}


Poco::XML::Node* XMLConfiguration::findElement(int index, Poco::XML::Node* pNode, bool create)
{
	Poco::XML::Node* pRefNode = pNode;
	if (index > 0)
	{
		pNode = pNode->nextSibling();
		while (pNode)
		{
			if (pNode->nodeName() == pRefNode->nodeName())
			{
				if (--index == 0) break;
			}
			pNode = pNode->nextSibling();
		}
	}
	if (!pNode && create)
	{
		if (index == 1)
		{
			Poco::AutoPtr<Poco::XML::Element> pElem = pRefNode->ownerDocument()->createElement(pRefNode->nodeName());
			pRefNode->parentNode()->appendChild(pElem);
			return pElem;
		}
		else throw Poco::InvalidArgumentException("Element index out of range.");
	}
	return pNode;
}


Poco::XML::Node* XMLConfiguration::findElement(const std::string& attr, const std::string& value, Poco::XML::Node* pNode)
{
	Poco::XML::Node* pRefNode = pNode;
	Poco::XML::Element* pElem = dynamic_cast<Poco::XML::Element*>(pNode);
	if (!(pElem && pElem->getAttribute(attr) == value))
	{
		pNode = pNode->nextSibling();
		while (pNode)
		{
			if (pNode->nodeName() == pRefNode->nodeName())
			{
				pElem = dynamic_cast<Poco::XML::Element*>(pNode);
				if (pElem && pElem->getAttribute(attr) == value) break;
			}
			pNode = pNode->nextSibling();
		}
	}
	return pNode;
}


Poco::XML::Node* XMLConfiguration::findAttribute(const std::string& name, Poco::XML::Node* pNode, bool create)
{
	Poco::XML::Node* pResult(0);
	Poco::XML::Element* pElem = dynamic_cast<Poco::XML::Element*>(pNode);
	if (pElem)
	{
		pResult = pElem->getAttributeNode(name);
		if (!pResult && create)
		{
			Poco::AutoPtr<Poco::XML::Attr> pAttr = pNode->ownerDocument()->createAttribute(name);
			pElem->setAttributeNode(pAttr);
			return pAttr;
		}
	}
	return pResult;
}


} } // namespace Poco::Util
