#!/usr/local/bin/python3
import cgi
import cgitb

# print("Location: /es.png")
# print()

# input_data = cgi.FieldStorage()
print("Status: 511 Not Found")
print("Status: 511 Not Found")
print("Status: 511 Not Found")
print("Content-type: text/html;charset=utf-8")
print()
# # print('hello')
# # for i in range(10):
# # 	print(i**2)
# # for k in input_data:
# # 	print(k, input_data[k])
# print("<html>")
# print("<body>")
# cgi.print_environ()
# print("</body>")
# print("</html>")



'''
#@ Server specific variables:
- SERVER_SOFTWARE: name/version of HTTP server.
- SERVER_NAME: host name of the server, may be dot-decimal IP address.
- GATEWAY_INTERFACE: CGI/version.
#@ Request specific variables:
- SERVER_PROTOCOL: HTTP/version.
- SERVER_PORT: TCP port (decimal).
- REQUEST_METHOD: name of HTTP method (see above).
- PATH_INFO: path suffix, if appended to URL after program name and a slash.
- PATH_TRANSLATED: corresponding full path as supposed by server, if PATH_INFO is present.
- SCRIPT_NAME: relative path to the program, like /cgi-bin/script.cgi.
- QUERY_STRING: the part of URL after ? character. The query string may be composed of *name=value pairs separated with ampersands (such as var1=val1&var2=val2...) when used to submit form data transferred via GET method as defined by HTML application/x-www-form-urlencoded.
- REMOTE_HOST: host name of the client, unset if server did not perform such lookup.
- REMOTE_ADDR: IP address of the client (dot-decimal).
- AUTH_TYPE: identification type, if applicable.
- REMOTE_USER used for certain AUTH_TYPEs.
- REMOTE_IDENT: see ident, only if server performed such lookup.
- CONTENT_TYPE: Internet media type of input data if PUT or POST method are used, as provided via HTTP header.
- CONTENT_LENGTH: similarly, size of input data (decimal, in octets) if provided via HTTP header.

#@ Variables passed by user agent
#  (HTTP_ACCEPT, HTTP_ACCEPT_LANGUAGE, HTTP_USER_AGENT, HTTP_COOKIE and possibly others) contain values of corresponding HTTP headers and therefore have the same sense.

'''