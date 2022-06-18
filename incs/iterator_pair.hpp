#ifndef ITERATOR_PAIR_HPP
# define ITERATOR_PAIR_HPP

template <class T1, class T2, class Diff = typename T1::difference_type> struct iterator_pair
{
	typedef T1		first_type;
	typedef T2		second_type;
	typedef Diff	difference_type;

	T1			first;
	T2			second;

/**========================================================================
* @                           Constructors
*========================================================================**/

	iterator_pair()									: first(), second() {}
    iterator_pair(const T1& t1, const T2& t2)		: first(t1), second(t2)	{}

	template <class U1, class U2>
    iterator_pair(const iterator_pair<U1, U2>& src): first(src.first), second(src.second) {}

/**========================================================================
* *                            operators
*========================================================================**/

	iterator_pair& operator=(iterator_pair const& src)
	{
		if (this == &src)	return (*this);

		first	= src.first;
        second	= src.second;

		return (*this);
    }

    iterator_pair&	operator++()							{ ++first; ++second; return *this; }
    iterator_pair	operator++(int)							{ iterator_pair tmp(*this); ++(*this); return tmp; }
    iterator_pair&	operator--()							{ --first; --second; return *this; }
    iterator_pair	operator--(int)							{ iterator_pair tmp(*this); --(*this); return tmp; }
    iterator_pair&	operator+=(difference_type n)			{ first += n; second += n; return *this; }
    iterator_pair	operator+ (difference_type n) const		{ iterator_pair tmp(*this); tmp += n; return tmp; }
    iterator_pair&	operator-=(difference_type n)			{ *this += -n; return *this; }
    iterator_pair	operator- (difference_type n) const		{ return (*this + (-n)); }
};

/**========================================================================
* *                          non-member operators
*========================================================================**/
template <class T1, class T2, class Diff> bool operator== (const iterator_pair<T1,T2, Diff>& lhs, const iterator_pair<T1,T2, Diff>& rhs) { return lhs.first==rhs.first && lhs.second==rhs.second; }
template <class T1, class T2, class Diff> bool operator!= (const iterator_pair<T1,T2, Diff>& lhs, const iterator_pair<T1,T2, Diff>& rhs) { return !(lhs==rhs); }
template <class T1, class T2, class Diff> bool operator<  (const iterator_pair<T1,T2, Diff>& lhs, const iterator_pair<T1,T2, Diff>& rhs) { return lhs.first<rhs.first || (!(rhs.first<lhs.first) && lhs.second<rhs.second); }	// lhs.first == rhs.first && lhs.second < rhs.second
template <class T1, class T2, class Diff> bool operator<= (const iterator_pair<T1,T2, Diff>& lhs, const iterator_pair<T1,T2, Diff>& rhs) { return !(rhs<lhs); }
template <class T1, class T2, class Diff> bool operator>  (const iterator_pair<T1,T2, Diff>& lhs, const iterator_pair<T1,T2, Diff>& rhs) { return rhs<lhs; }
template <class T1, class T2, class Diff> bool operator>= (const iterator_pair<T1,T2, Diff>& lhs, const iterator_pair<T1,T2, Diff>& rhs) { return !(lhs<rhs); }

template <class T1, class T2, class Diff>
typename iterator_pair<T1, T2, Diff>::difference_type
operator-(const iterator_pair<T1, T2, Diff>& x, const iterator_pair<T1, T2, Diff>& y)       { return x.base() - y.base(); }

template <class T1, class T2, class Diff>
iterator_pair<T1, T2, Diff>
operator+(typename iterator_pair<T1, T2, Diff>::difference_type n, iterator_pair<T1, T2, Diff> x)
{
    x += n;
    return x;
}

/**========================================================================
* #                         non-member functions
*========================================================================**/

template <class T1, class T2>
iterator_pair<T1,T2,typename T1::difference_type> make_iterator_pair (T1 x, T2 y)	{ return ( iterator_pair<T1,T2>(x,y) ); }
template <class T1, class T2, class Diff>
iterator_pair<T1,T2,Diff> make_iterator_pair (T1 x, T2 y)	{ return ( iterator_pair<T1,T2, Diff>(x,y) ); }



// C++98 : cannot infer template arguments from the constructor.
#endif


