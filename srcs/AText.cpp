#include "AText.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

	bool			AText::empty() const					{ return this->content.empty(); }
	void			AText::setContent(const string& c)		{ this->content = c; }
	string			AText::getContent() const				{ return this->content; }	// return copied string, not source.
	void			AText::append(const string& s)			{ this->content.append(s); }
