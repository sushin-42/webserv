#ifndef COLOR_HPP
# define COLOR_HPP

# include <iomanip>
// # include <iostream>

# define RED(s) 				"\033[1;31m"s"\033[0m"
# define BLUE(s) 				"\033[1;34m"s"\033[0m"
# define GRAY(s) 				"\033[0;90m"s"\033[0m"
# define CYAN(s) 				"\033[1;36m"s"\033[0m"
# define GREEN(s)				"\033[0;32m"s"\033[0m"
# define YELLOW(s)				"\033[0;33m"s"\033[0m"
# define PURPLE(s)				"\033[0;35m"s"\033[0m"
// # define NC(s)					"\033[0m"
# define UL(s)					"\033[4m"s"\033[0m"

# define TAG(CLASS, METHOD)		cout << "[ " << PURPLE(#CLASS) << "#"  << UL(#METHOD) <<"() ] "
# define _NOTE(s)				"\033[0;33m" << "NOTE: " #s << " \033[0m"
# define _FAIL(s)				"\033[1;31m" << "FAIL: " #s << " \033[0m"
# define _GOOD(s)				"\033[0;36m" << "GOOD: " #s << " \033[0m"


#endif

