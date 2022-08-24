#include "AHeader.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

void						AHeader::clear()									{ content.clear(); headerField.clear(); }
void						AHeader::setHTTPversion(const string& v)			{ this->HTTPversion = v; }
const string&				AHeader::getHTTPversion() const						{ return this->HTTPversion; }
void						AHeader::setHeaderField(const _Map& hf)				{ this->headerField = hf; }
const map<string, string>&	AHeader::getHeaderField() const						{ return this->headerField; }
void						AHeader::removeKey(const string& key)				{ this->headerField.erase(lowerize(key)); }
void						AHeader::insertRange(iterator from, iterator to)	{ this->headerField.insert(from, to); }
bool						AHeader::exist(const string& key)					{ return this->headerField.find(lowerize(key)) != this->headerField.end(); }

void	AHeader::append(const string& key, const string& value)
{
	if (this->exist(key))
	{
		if ((*this)[key] != value)
			(*this)[key] += (", " + value);
	}
	else
		(*this)[key] = value;
}

void	AHeader::integrate()
{
	map<string, string>::iterator it;
	map<string, string>::iterator ite = headerField.end();
	for (it = headerField.begin(); it != ite; it++ )
		content.append(capitalize(it->first, '-') + ": " + it->second + "\r\n");
	content.append("\r\n");
}

