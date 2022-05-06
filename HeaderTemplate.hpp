/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderTemplate.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 14:58:24 by mishin            #+#    #+#             */
/*   Updated: 2022/05/06 17:43:33 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HEADERTEMPLATE_HPP
# define HEADERTEMPLATE_HPP

# include <string>
# include <cstring>
# include <map>
#include <sys/fcntl.h>
using namespace std;


// * example header
string headerTemplate = "#HTTP-VERSION #STATUS #REASON-PHARSE\n"
    					"Date: Fri, 01 Feb 2002 01:34:56 GMT\n"
    					"Server: webserv42\n"
    					"Connection: #CONNECTION\n"
    					"Content-Type: #MIME-TYPE\n\n";

    			// "X-Powered-By: PHP/4.0.6\n"





#endif
