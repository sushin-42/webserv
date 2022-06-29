#ifndef COLOR_HPP
# define COLOR_HPP

# include <iomanip>
// # include <iostream>

# define _NC				"\033[0m"
# define _UL				"\033[4m"
# define _RED 				"\033[1;31m"
# define _BLUE 				"\033[1;34m"
# define _GRAY 				"\033[0;90m"
# define _CYAN 				"\033[1;36m"
# define _GREEN				"\033[0;32m"
# define _YELLOW			"\033[0;33m"
# define _PURPLE			"\033[0;35m"
# define UL(s)				_UL s _NC
# define RED(s) 			_RED s _NC
# define BLUE(s) 			_BLUE s _NC
# define GRAY(s) 			_GRAY s _NC
# define CYAN(s) 			_CYAN s _NC
# define GREEN(s)			_GREEN s _NC
# define YELLOW(s)			_YELLOW s _NC
# define PURPLE(s)			_PURPLE s _NC

# define TAG(CLASS)				cerr << "[ " << PURPLE(#CLASS) << "#"  << _UL << __func__ << _NC <<"() ] "
# define _NOTE(s)				"\033[0;33m"  "NOTE: " #s " \033[0m"
# define _FAIL(s)				"\033[1;31m"  "FAIL: " #s " \033[0m"
# define _GOOD(s)				"\033[0;36m"  "GOOD: " #s " \033[0m"

# ifdef DEBUG
# define LOGGING(CLASS, ...)		TAG(CLASS); fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
# else
# define LOGGING(CLASS, ...)	(void)0
# endif

#endif
