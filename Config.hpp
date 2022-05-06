/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 14:53:32 by mishin            #+#    #+#             */
/*   Updated: 2022/05/06 21:02:45 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <unistd.h>
# include "utils.hpp"
using namespace std;

string root = getcwd(NULL, 0);

map<string, string>	MIME= getMIME();

class Config
{
private:

public:
	Config(  );
	Config( const Config& src );
	~Config();

	Config&	operator=( const Config& src );


};

#endif
