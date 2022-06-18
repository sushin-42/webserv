#include "IText.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

	bool			IText::empty() const					{ return this->content.empty(); }
	void			IText::setContent(const string& c)		{ this->content = c; }
	string			IText::getContent() const				{ return this->content; }	// return copied string, not source.
	void			IText::append(const string& s)			{ this->content.append(s); }
