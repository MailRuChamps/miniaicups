import sys
import json
data = sys.stdin.readlines()
strings = []
for s in data:
    try:
        string = unicode(s)
    except UnicodeDecodeError:
        string = "Can't parse string\n"
    strings.append(string)
print json.dumps(''.join(strings)[:100000])