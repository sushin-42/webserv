#!/usr/local/bin/python3
import cgi
import cgitb
import time
print("Status: 200 OK", flush=True)
print("Content-type: text/htmlss", flush=True)

print("Accept: text/abcd", flush=True)

print("Location: /loop.py", flush=True)
print(flush=True)