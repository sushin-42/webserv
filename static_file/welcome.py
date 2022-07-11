#!/usr/local/bin/python3
print("Content-Type: text/html")
print()

import cgi
import cgitb

cgitb.enable()

form = cgi.FieldStorage()
username = form.getvalue("name")

print('''<!DOCTYPE html>
<html>
<head>
<title>Welcome, {username}!</title>
</head>
<body>
<h1>Welcome, {username}!</h1>
</body>
</html>
'''.format(username = username))
