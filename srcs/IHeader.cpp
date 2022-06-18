#include "IHeader.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

void						IHeader::clear()									{ content.clear(); headerField.clear(); }
void						IHeader::setHTTPversion(const string& v)			{ this->HTTPversion = v; }
const string&				IHeader::getHTTPversion() const						{ return this->HTTPversion; }
void						IHeader::setHeaderField(const _Map& hf)				{ this->headerField = hf; }
const map<string, string>&	IHeader::getHeaderField() const						{ return this->headerField; }
void						IHeader::removeKey(const string& key)				{ this->headerField.erase(lowerize(key)); }
void						IHeader::insertRange(iterator from, iterator to)	{ this->headerField.insert(from, to); }
bool						IHeader::exist(const string& key)					{ return this->headerField.find(lowerize(key)) != this->headerField.end(); }

void	IHeader::append(const string& key, const string& value)
{
	if (this->exist(key))
	{
		if ((*this)[key] != value)
			(*this)[key] += (", " + value);
	}
	else
		(*this)[key] = value;
}

void	IHeader::integrate()
{
	map<string, string>::iterator it;
	map<string, string>::iterator ite = headerField.end();
	for (it = headerField.begin(); it != ite; it++ )
		content.append(capitalize(it->first, '-') + ": " + it->second + "\r\n");
	content.append("\r\n");
}

