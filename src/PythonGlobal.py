import io
import traceback

class NestedDict(dict):
    def __getitem__(self,item):
        try:
            return dict.__getitem__(self,item)
        except KeyError:
            self[item] = type(self)()
            return self[item]

line = ''
command = ''
atcp = {}
channel102 = {}
gmcp=NestedDict()

def onConnect():
    "Run when connection established"
    
def onDisconnect():
    "Run on disconnect"
    
def handleWindowResizeEvent():
    "Run when window resizes"

def printFixedStackTrace(trace,funcName):
    """Need to modify the stack trace to correspond
     to the lines numbers in the mudlet editor."""
    lines = trace.splitlines()
    lines[0] = 'Python ' + lines[0]
    replace = lines[1].split(',')
    lineno = replace[1].split()[1]
    scripttype = replace[2].split()[1]
    index = -1
    while (scripttype[index].isdigit()):
        index -= 1
    scripttype = scripttype[:index+1]
    replace[1] = ' line ' + str(int(lineno)-3)
    replace[2] = ' in ' + scripttype + ': ' + funcName
    lines[1] = ','.join(replace)
    print '\n'.join(lines)

