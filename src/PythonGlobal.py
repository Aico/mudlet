import io
import site
import traceback
import PythonQt
from PythonQt.mudlet import *
from PythonQt.QtGui import QColor,QPixmap
from PythonQt.QtCore import QPointF,QSizeF
import re
import pprint
from sys import *
import webbrowser

mudlet = MudletObject(HOST_HASH)

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

class NestedDict(dict):
    def __getitem__(self,item):
        try:
            return dict.__getitem__(self,item)
        except KeyError:
            self[item] = type(self)()
            return self[item]
            
class Mapper:
    def __init__(self):
        self.envColors = Mapper.EnvColors()
        self.rooms = Mapper.Rooms()
        self.areaNamesMap = Mapper.AreaNamesMap()
        self.customEnvColors = Mapper.CustomEnvColors()
        #self.hashTable = Mapper.HashTable()
        self.mapLabels = Mapper.MapLabels()
        
    class EnvColors(dict):
        def __init__(self):
            super(Mapper.EnvColors,self).__init__()
            for key,value in HOST_ENV_COLORS.iteritems():
                super(Mapper.EnvColors,self).__setitem__(ord(key),value)
                
        def __setitem__(self,key,value):
            raise Exception('envColors dict is immutable.')
            
        def __delitem__(self, key):
            raise Exception('envColors dict is immutable.')
            
    class Rooms(dict):
        def __init__(self):
            super(Mapper.Rooms,self).__init__()
            for key,value in HOST_ROOMS.iteritems():
                room = {}
                for room_key,room_value in value.iteritems():
                    room[str(room_key)]=room_value
                super(Mapper.Rooms,self).__setitem__(ord(key),Mapper.Room(room,self))                
                
        def __setitem__(self,key,value):
            if key > 65000:
                #TODO create support for keys > 65000.
                raise Exception('key cannot be higher than 65000')
            if type(value) == Mapper.Room:
                super(Mapper.Room,value).__setitem__('id',key)
                mudlet.updateRoom(value)
                super(Mapper.Rooms,self).__setitem__(key,value)
                value.setContainer(self)
            elif type(value) == dict:
                value['id']=int(key)
                mudlet.updateRoom(value)
                super(Mapper.Rooms,self).__setitem__(key,Mapper.Room(value,self))
            else:
                raise Exception('Value must be a Room or a dict')
            
        def __delitem__(self,key):
            mudlet.deleteRoom(key)
            super(Mapper.Rooms,self).__delitem__(key)            
            for k,v in self.iteritems():
                for rk,rv in v.iteritems():
                    if type(rv) == type(1):
                        if rv == key and rk in ('north','northeast','east','southeast','south','southwest','west','northwest','up','down','in','out'):
                            v[rk]=-1
                            super(Mapper.Rooms,self).__setitem__(k,v)
                            
    class Room(dict):
        def __init__(self,data,c=None):
            self.rooms = c
            for k,v in data.iteritems():
                super(Mapper.Room,self).__setitem__(k,v)
                
        def __setitem__(self,key,value):
            super(Mapper.Room,self).__setitem__(key,value)
            if key == 'userData':
                for k,v in value.iteritems():
                    mudlet.setRoomUserData(self['id'],k,v)
            elif key == 'highlight':
                mudlet.toggleHighlight(self['id'], value)
            else:         
                self.rooms[self['id']]=self
            
        def setContainer(self,c):
            self.rooms = c
                
    class AreaNamesMap(dict):
        def __init__(self):
            super(Mapper.AreaNamesMap,self).__init__()
            for key,value in HOST_AREA_NAMES_MAP.iteritems():
                super(Mapper.AreaNamesMap,self).__setitem__(ord(key),value)
                
        def __setitem__(self,key,value):
            mudlet.setAreaName( key, value )
            super(Mapper.AreaNamesMap,self).__setitem__(key,value)  
            
        def __delitem__(self, key):
            mudlet.deleteArea(key)
            for k,v in Mapper.rooms.items():
                if v['area'] == key:
                    del Mapper.rooms[k]
            super(Mapper.AreaNamesMap,self).__delitem__(key)            
                
    class CustomEnvColors(dict):
        def __init__(self):
            super(Mapper.CustomEnvColors,self).__init__()
            for key,value in HOST_CUSTOM_ENV_COLORS.iteritems():
                super(Mapper.CustomEnvColors,self).__setitem__(ord(key),value)                
            
        def __setitem__(self,key,value):
            mudlet.setCustomEnvColor(key,value)
            super(Mapper.CustomEnvColors,self).__setitem__(key,value)
            
        def __delitem__(self, key):
            mudlet.removeCustomEnvColor(key)
            super(Mapper.CustomEnvColors,self).__delitem__(key)
                
#    class HashTable(dict):
#        def __init__(self):
#            super(Mapper.HashTable,self).__init__()
#            for key,value in HOST_MAP_HASH_TABLE.iteritems():
#                super(Mapper.HashTable,self).__setitem__(str(key),value)                
                
    class MapLabels(dict):
        def __init__(self):
            super(Mapper.MapLabels,self).__init__()
            for key,value in HOST_MAP_LABELS.iteritems():
                labelmap = {}
                for map_key,map_value in value.iteritems():
                    l = {}
                    for label_key,label_value in map_value.iteritems():
                        l[str(label_key)] = label_value
                    labelmap[ord(map_key)]=Mapper.MapLabel(l.get('pos'),l.get('size'),l.get('text'),l.get('fgColor'),l.get('bgColor'),l.get('pix'),l.get('pointer'))
                super(Mapper.MapLabels,self).__setitem__(ord(key),Mapper.Labels(labelmap))
                self[ord(key)].setContainer(self,ord(key))
                
        def __setitem__(self,key,value):
            if type(value) == dict:
                value = Mapper.Labels(value)
            if type(value) == Mapper.Labels:
                value.setContainer(self,key)
                for k,v in value.iteritems():
                     mudlet.updateMapLabel( key, v.text, v.pos.x(), v.pos.y(), v.fgColor, v.bgColor, k )
                super(Mapper.MapLabels,self).__setitem__(key,value)          
            else:
                raise Exception('Value must be a Room or a dict')
                
        def __delitem__(self,key):
            for k,v in self[key].items():
                del self[key][k]
            super(Mapper.MapLabels,self).__delitem__(key)
                
    class Labels(dict):
        def __init__(self,data):
            for k,v in data.iteritems():
                v.setContainer(self,k)
                super(Mapper.Labels,self).__setitem__(k,v)                
                
        def __setitem__(self,key,value):            
            if type(value) == Mapper.MapLabel:
                mudlet.updateMapLabel( self.areaid, value.text, value.pos.x(), value.pos.y(), value.fgColor, value.bgColor, key )            
            else:
                raise Exception('Value must either be a MapLabel or a dict')
            value.setContainer(self,key)
            super(Mapper.Labels,self).__setitem__(key,value)
            
        def __delitem__(self,key):
            mudlet.deleteMapLabel(self.areaid, key)
            super(Mapper.Labels,self).__delitem__(key)
            
        def setContainer(self,c,areaid):
            self.maplabels = c
            self.areaid = areaid
            
    class MapLabel(object):
        def __init__(self,pos,size,text,fgColor,bgColor,pix=None,pointer=None):            
            super(Mapper.MapLabel,self).__setattr__('pos', pos)
            super(Mapper.MapLabel,self).__setattr__('pointer', pointer)
            super(Mapper.MapLabel,self).__setattr__('size', size)
            super(Mapper.MapLabel,self).__setattr__('text', text)
            super(Mapper.MapLabel,self).__setattr__('fgColor', fgColor)
            super(Mapper.MapLabel,self).__setattr__('bgColor', bgColor)
            super(Mapper.MapLabel,self).__setattr__('pix', pix)
            
        def __setattr__(self, name, value):
            super(Mapper.MapLabel,self).__setattr__(name, value)
            self.labels[self.labelid]=self
            
        def setContainer(self,c,labelid):
            super(Mapper.MapLabel,self).__setattr__('labels', c)
            super(Mapper.MapLabel,self).__setattr__('labelid', labelid)
            
    def centerview(self,roomid):
        mudlet.centerview(roomid)
        
    def getPath(self,from_room,to_room):
        return mudlet.getPath(from_room,to_room)
        
    def gotoRoom(self,room):
        return mudlet.gotoRoom(room)
        
    def saveMap(self,location):
        return mudlet.saveMap(location)
        
    def setGridMode(self,area,on):
        return mudlet.setGridMode(area,on)
        
    def refresh(self):
        mudlet.update2DMapperNow()
            

line = ''
command = ''
atcp = {}
channel102 = {}
gmcp=NestedDict()
mapper = Mapper()

def onConnect():
    """Run when connection established
        Placeholder, should reimplement in PythonLocal.py"""
    
def onDisconnect():
    """Run on disconnect
        Placeholder, should reimplement in PythonLocal.py"""
    
def handleWindowResizeEvent():
    """Run when window resizes
        Placeholder, should reimplement in PythonLocal.py"""

def send(text,wantPrint=True):
    mudlet.send(text,wantPrint)
    
def expandAlias(text,wantPrint=True):
    mudlet.expandAlias(text,wantPrint)

def selectString(text,pos,console='main'):
    return mudlet.selectString(text,pos,console)
    
def resetFormat(console='main'):
    mudlet.resetFormat(console)

def setBgColor(r,g,b,console='main'): 
    mudlet.setBgColor(r,g,b,console)
    
def setFgColor(r,g,b,console='main'):
    mudlet.setFgColor(r,g,b,console)
    
def bg(color,console='main'):
    code = color_dict[color.strip()]
    setBgColor(code[0],code[1],code[2],console)
    
def fg(color,console='main'):
    code = color_dict[color.strip()]
    setFgColor(code[0],code[1],code[2],console)
    
def enableTimer(timer):
    return mudlet.enableTimer(timer)
    
def enableKey(key):
    return mudlet.enableKey(key)
    
def enableTrigger(trigger):
    return mudlet.enableTrigger(trigger)
    
def enableAlias(alias):
    return mudlet.enableAlias(alias)
    
def disableTimer(timer):
    return mudlet.disableTimer(timer)
    
def disableKey(key):
    return mudlet.disableKey(key)
    
def disableTrigger(trigger):
    return mudlet.disableTrigger(trigger)
    
def disableAlias(alias):
    return mudlet.disableAlias(alias)
    
def selectCaptureGroup(group):
    return mudlet.selectCaptureGroup(group)
    
def replace(text,console='main'):
    mudlet.replace(text,console)

def replaceAll(what,text,console='main'):
    mudlet.replaceAll(what,text,console)
    
def selectSection(col,length,console='main'):
    return mudlet.selectSection(col,length,console)
    
def deleteLine(console='main'):
    mudlet.deleteLine(console)

def reconnect():
    mudlet.reconnect()

def disconnect():
    mudlet.disconnect()
    
def raiseEvent(*arg):
    mudlet.raiseEvent(arg)
    

#Color Table ported from Lua table by Vadim Peretrokin 2009.
color_dict = {
        'snow'                  : (255, 250, 250),
        'ghost_white'           : (248, 248, 255),
        'GhostWhite'            : (248, 248, 255),
        'white_smoke'           : (245, 245, 245),
        'WhiteSmoke'            : (245, 245, 245),
        'gainsboro'             : (220, 220, 220),
        'floral_white'          : (255, 250, 240),
        'FloralWhite'           : (255, 250, 240),
        'old_lace'              : (253, 245, 230),
        'OldLace'               : (253, 245, 230),
        'linen'                 : (250, 240, 230),
        'antique_white'         : (250, 235, 215),
        'AntiqueWhite'          : (250, 235, 215),
        'papaya_whip'           : (255, 239, 213),
        'PapayaWhip'            : (255, 239, 213),
        'blanched_almond'       : (255, 235, 205),
        'BlanchedAlmond'        : (255, 235, 205),
        'bisque'                : (255, 228, 196),
        'peach_puff'            : (255, 218, 185),
        'PeachPuff'             : (255, 218, 185),
        'navajo_white'          : (255, 222, 173),
        'NavajoWhite'           : (255, 222, 173),
        'moccasin'              : (255, 228, 181),
        'cornsilk'              : (255, 248, 220),
        'ivory'                 : (255, 255, 240),
        'lemon_chiffon'         : (255, 250, 205),
        'LemonChiffon'          : (255, 250, 205),
        'seashell'              : (255, 245, 238),
        'honeydew'              : (240, 255, 240),
        'mint_cream'            : (245, 255, 250),
        'MintCream'             : (245, 255, 250),
        'azure'                 : (240, 255, 255),
        'alice_blue'            : (240, 248, 255),
        'AliceBlue'             : (240, 248, 255),
        'lavender'              : (230, 230, 250),
        'lavender_blush'        : (255, 240, 245),
        'LavenderBlush'         : (255, 240, 245),
        'misty_rose'            : (255, 228, 225),
        'MistyRose'             : (255, 228, 225),
        'white'                 : (255, 255, 255),
        'black'                 : (0, 0, 0),
        'dark_slate_gray'       : (47, 79, 79),
        'DarkSlateGray'         : (47, 79, 79),
        'dark_slate_grey'       : (47, 79, 79),
        'DarkSlateGrey'         : (47, 79, 79),
        'dim_gray'              : (105, 105, 105),
        'DimGray'               : (105, 105, 105),
        'dim_grey'              : (105, 105, 105),
        'DimGrey'               : (105, 105, 105),
        'slate_gray'            : (112, 128, 144),
        'SlateGray'             : (112, 128, 144),
        'slate_grey'            : (112, 128, 144),
        'SlateGrey'             : (112, 128, 144),
        'light_slate_gray'      : (119, 136, 153),
        'LightSlateGray'        : (119, 136, 153),
        'light_slate_grey'      : (119, 136, 153),
        'LightSlateGrey'        : (119, 136, 153),
        'gray'                  : (190, 190, 190),
        'grey'                  : (190, 190, 190),
        'light_grey'            : (211, 211, 211),
        'LightGrey'             : (211, 211, 211),
        'light_gray'            : (211, 211, 211),
        'LightGray'             : (211, 211, 211),
        'midnight_blue'         : (25, 25, 112),
        'MidnightBlue'          : (25, 25, 112),
        'navy'                  : (0, 0, 128),
        'navy_blue'             : (0, 0, 128),
        'NavyBlue'              : (0, 0, 128),
        'cornflower_blue'       : (100, 149, 237),
        'CornflowerBlue'        : (100, 149, 237),
        'dark_slate_blue'       : (72, 61, 139),
        'DarkSlateBlue'         : (72, 61, 139),
        'slate_blue'            : (106, 90, 205),
        'SlateBlue'             : (106, 90, 205),
        'medium_slate_blue'     : (123, 104, 238),
        'MediumSlateBlue'       : (123, 104, 238),
        'light_slate_blue'      : (132, 112, 255),
        'LightSlateBlue'        : (132, 112, 255),
        'medium_blue'           : (0, 0, 205),
        'MediumBlue'            : (0, 0, 205),
        'royal_blue'            : (65, 105, 225),
        'RoyalBlue'             : (65, 105, 225),
        'blue'                  : (0, 0, 255),
        'dodger_blue'           : (30, 144, 255),
        'DodgerBlue'            : (30, 144, 255),
        'deep_sky_blue'         : (0, 191, 255),
        'DeepSkyBlue'           : (0, 191, 255),
        'sky_blue'              : (135, 206, 235),
        'SkyBlue'               : (135, 206, 235),
        'light_sky_blue'        : (135, 206, 250),
        'LightSkyBlue'          : (135, 206, 250),
        'steel_blue'            : (70, 130, 180),
        'SteelBlue'             : (70, 130, 180),
        'light_steel_blue'      : (176, 196, 222),
        'LightSteelBlue'        : (176, 196, 222),
        'light_blue'            : (173, 216, 230),
        'LightBlue'             : (173, 216, 230),
        'powder_blue'           : (176, 224, 230),
        'PowderBlue'            : (176, 224, 230),
        'pale_turquoise'        : (175, 238, 238),
        'PaleTurquoise'         : (175, 238, 238),
        'dark_turquoise'        : (0, 206, 209),
        'DarkTurquoise'         : (0, 206, 209),
        'medium_turquoise'      : (72, 209, 204),
        'MediumTurquoise'       : (72, 209, 204),
        'turquoise'             : (64, 224, 208),
        'cyan'                  : (0, 255, 255),
        'light_cyan'            : (224, 255, 255),
        'LightCyan'             : (224, 255, 255),
        'cadet_blue'            : (95, 158, 160),
        'CadetBlue'             : (95, 158, 160),
        'medium_aquamarine'     : (102, 205, 170),
        'MediumAquamarine'      : (102, 205, 170),
        'aquamarine'            : (127, 255, 212),
        'dark_green'            : (0, 100, 0),
        'DarkGreen'             : (0, 100, 0),
        'dark_olive_green'      : (85, 107, 47),
        'DarkOliveGreen'        : (85, 107, 47),
        'dark_sea_green'        : (143, 188, 143),
        'DarkSeaGreen'          : (143, 188, 143),
        'sea_green'             : (46, 139, 87),
        'SeaGreen'              : (46, 139, 87),
        'medium_sea_green'      : (60, 179, 113),
        'MediumSeaGreen'        : (60, 179, 113),
        'light_sea_green'       : (32, 178, 170),
        'LightSeaGreen'         : (32, 178, 170),
        'pale_green'            : (152, 251, 152),
        'PaleGreen'             : (152, 251, 152),
        'spring_green'          : (0, 255, 127),
        'SpringGreen'           : (0, 255, 127),
        'lawn_green'            : (124, 252, 0),
        'LawnGreen'             : (124, 252, 0),
        'green'                 : (0, 255, 0),
        'chartreuse'            : (127, 255, 0),
        'medium_spring_green'   : (0, 250, 154),
        'MediumSpringGreen'     : (0, 250, 154),
        'green_yellow'          : (173, 255, 47),
        'GreenYellow'           : (173, 255, 47),
        'lime_green'            : (50, 205, 50),
        'LimeGreen'             : (50, 205, 50),
        'yellow_green'          : (154, 205, 50),
        'YellowGreen'           : (154, 205, 50),
        'forest_green'          : (34, 139, 34),
        'ForestGreen'           : (34, 139, 34),
        'olive_drab'            : (107, 142, 35),
        'OliveDrab'             : (107, 142, 35),
        'dark_khaki'            : (189, 183, 107),
        'DarkKhaki'             : (189, 183, 107),
        'khaki'                 : (240, 230, 140),
        'pale_goldenrod'        : (238, 232, 170),
        'PaleGoldenrod'         : (238, 232, 170),
        'light_goldenrod_yellow': (250, 250, 210),
        'LightGoldenrodYellow'  : (250, 250, 210),
        'light_yellow'          : (255, 255, 224),
        'LightYellow'           : (255, 255, 224),
        'yellow'                : (255, 255, 0),
        'gold'                  : (255, 215, 0),
        'light_goldenrod'       : (238, 221, 130),
        'LightGoldenrod'        : (238, 221, 130),
        'goldenrod'             : (218, 165, 32),
        'dark_goldenrod'        : (184, 134, 11),
        'DarkGoldenrod'         : (184, 134, 11),
        'rosy_brown'            : (188, 143, 143),
        'RosyBrown'             : (188, 143, 143),
        'indian_red'            : (205, 92, 92),
        'IndianRed'             : (205, 92, 92),
        'saddle_brown'          : (139, 69, 19),
        'SaddleBrown'           : (139, 69, 19),
        'sienna'                : (160, 82, 45),
        'peru'                  : (205, 133, 63),
        'burlywood'             : (222, 184, 135),
        'beige'                 : (245, 245, 220),
        'wheat'                 : (245, 222, 179),
        'sandy_brown'           : (244, 164, 96),
        'SandyBrown'            : (244, 164, 96),
        'tan'                   : (210, 180, 140),
        'chocolate'             : (210, 105, 30),
        'firebrick'             : (178, 34, 34),
        'brown'                 : (165, 42, 42),
        'dark_salmon'           : (233, 150, 122),
        'DarkSalmon'            : (233, 150, 122),
        'salmon'                : (250, 128, 114),
        'light_salmon'          : (255, 160, 122),
        'LightSalmon'           : (255, 160, 122),
        'orange'                : (255, 165, 0),
        'dark_orange'           : (255, 140, 0),
        'DarkOrange'            : (255, 140, 0),
        'coral'                 : (255, 127, 80),
        'light_coral'           : (240, 128, 128),
        'LightCoral'            : (240, 128, 128),
        'tomato'                : (255, 99, 71),
        'orange_red'            : (255, 69, 0),
        'OrangeRed'             : (255, 69, 0),
        'red'                   : (255, 0, 0),
        'hot_pink'              : (255, 105, 180),
        'HotPink'               : (255, 105, 180),
        'deep_pink'             : (255, 20, 147),
        'DeepPink'              : (255, 20, 147),
        'pink'                  : (255, 192, 203),
        'light_pink'            : (255, 182, 193),
        'LightPink'             : (255, 182, 193),
        'pale_violet_red'       : (219, 112, 147),
        'PaleVioletRed'         : (219, 112, 147),
        'maroon'                : (176, 48, 96),
        'medium_violet_red'     : (199, 21, 133),
        'MediumVioletRed'       : (199, 21, 133),
        'violet_red'            : (208, 32, 144),
        'VioletRed'             : (208, 32, 144),
        'magenta'               : (255, 0, 255),
        'violet'                : (238, 130, 238),
        'plum'                  : (221, 160, 221),
        'orchid'                : (218, 112, 214),
        'medium_orchid'         : (186, 85, 211),
        'MediumOrchid'          : (186, 85, 211),
        'dark_orchid'           : (153, 50, 204),
        'DarkOrchid'            : (153, 50, 204),
        'dark_violet'           : (148, 0, 211),
        'DarkViolet'            : (148, 0, 211),
        'blue_violet'           : (138, 43, 226),
        'BlueViolet'            : (138, 43, 226),
        'purple'                : (160, 32, 240),
        'medium_purple'         : (147, 112, 219),
        'MediumPurple'          : (147, 112, 219),
        'thistle'               : (216, 191, 216)
}

# Functions added by KrimMalak.

defaultbrowser="Unselected" #Placing here till I figure out how to make it a Mudlet variable a user can set.

def deselect():
    selectString("",1)
    
def replaceLine(what):
    '''Does not use insertText like Lua version, but seems to work fine.'''
    selectString(line, 1)
    replace(what)

def openUrl(url):
    """This can use some further expanding, but not up to adding stuff to the Settings menu yet.
       Basically, on my version of Ubuntu I had to explicitly state the browser I wanted.  Would be nice to
       be able what brower the user wanted to use.  This link lists browsers Python understands to look for: 
       http://www.python.org/doc//current/library/webbrowser.html#module-webbrowser
       Status: Functioning"""

    global defaultbrowser
    if defaultbrowser=="Unselected":
        defaultbrowser='firefox'
    browser=webbrowser.get(defaultbrowser)
    browser.open_new_tab(url)

def sendAll(commands):
    """Commands need to be passed as a list or tuple.  Status: Complete"""
    for x in commands:
        send(x)


def display(obj):
    pprint.pprint(obj,width=60)


def cecho(text,console='main',insert=False):
    text=re.split("(<.*?>)",text)
    for line in text:
        if re.match("<(.*?)>",line):
            match=re.match("<(.*?)>",line)
            if ',' in match.group(1):
                split_match=match.group(1).split(',')
                if split_match[0] != '' and split_match[0] in color_dict:
                    fg(split_match[0],console)
                if split_match[1] != '' and split_match[1] in color_dict:
                    bg(split_match[1],console)
            elif match.group(1) in color_dict:
                fg(match.group(1),console)
        else:
            if insert==False: 
		echo(line,console) 
	    else: 
		insertText(line,console)
    resetFormat()

def decho(text,console='main',insert=False):
    text=re.split("(<.*?>)",text)
    for line in text:
        if re.match("<(.*?)>",line):
            match=re.match("<(.*?)>",line)
            if ':' in match.group(1):
                split_match=match.group(1).split(':')
                if split_match[0] != '':
                    codes=split_match[0].split(',')
                    setFgColor(int(codes[0]), int(codes[1]),int(codes[2]),console)
                if split_match[1] != '':
                    codes=split_match[1].split(',')
                    setBgColor(int(codes[0]), int(codes[1]),int(codes[2]),console)
            else:
                codes=match.group(1).split(',')
                setFgColor(int(codes[0]), int(codes[1]),int(codes[2]),console)
        else:
            if insert==False: 
                echo(line,console) 
            else: 
                insertText(line,console)
    resetFormat()

def hecho(text,console='main',insert=False):
    text=re.split("(\|c\w{6},\w{6}|\|c\w{6})",text)
    for line in text:
        if re.match("\|c\w{6},\w{6}|\|c\w{6}",line):
            m=re.match("(\|c\w{6},\w{6}|\|c\w{6})",line)
            if ',' in m.group(1):
                split_match=m.group(1).split(',')
                if split_match[0] != '':
                    codes=split_match[0]
                    codes=codes[2:4],codes[4:6],codes[6:8]
                    setFgColor(int(codes[0],16), int(codes[1],16),int(codes[2],16),console)
                if split_match[1] != '':
                    codes=split_match[1]
                    codes=codes[0:2],codes[2:4],codes[4:6]
                    setBgColor(int(codes[0],16), int(codes[1],16),int(codes[2],16),console)
            else:
                codes=m.group(1)
                codes=codes[2:4],codes[4:6],codes[6:8]
                setFgColor(int(codes[0],16), int(codes[1],16),int(codes[2],16),console)
        else:
            if insert==False: 
                echo(line,console) 
            else: 
                insertText(line,console)
    resetFormat()


def replaceWildcard(what, replacement):
    selectCaptureGroup(what)
    replace(replacement)
    

def showColors(wide=3):
    pos=1
    for k in color_dict.keys():
        v=color_dict[k]
        lum = (0.2126 * ((float(v[0])/255)**2.2)) + (0.7152 * ((float(v[1])/255)**2.2)) + (0.0722 * ((float(v[2])/255)**2.2))
        if lum > 0.5:
            fg="black"
        else:
            fg="white"
        if pos==wide:
            cecho("<"+fg+","+k+">"+k+" "*(23-len(k))+"<,black>  \n")
            pos=1
        else:
            cecho("<"+fg+","+k+">"+k+" "*(23-len(k))+"<,black>  ")
            pos=pos+1

def sendGMCP(msg):
    mudlet.sendGMCP(msg)

def sendATCP(msg):
    mudlet.sendATCP(msg)

def sendTelnetChannel102(msg):
    mudlet.sendTelnetChannel102(msg)

def sendIrc(channel, msg):
    mudlet.sendIrc(channel, msg)

def echo(txt, console='main'):
    mudlet.echo(txt, console)

def echoLink(txt, func, hint, console='main', customFormat=False):
    mudlet.echoLink(txt, func, hint, console, customFormat)

def insertLink(txt, func, hint, console='main', customFormat=False):
    mudlet.insertLink(txt, func, hint, console, customFormat)

def setLink(func, hint, console='main'):
    mudlet.setLink(func, hint, console)

def echoPopup(txt, func, hint, console='main', customFormat=False):
    mudlet.echoPopup(txt, func, hint, console, customFormat)

def setPopup(func,hint,console='main'):
    mudlet.setPopup(func,hint,console)

def insertPopup(txt, func, hint, console='main'):
    mudlet.insertPopup(txt, func, hint, console)

def createBuffer(name):
    mudlet.createBuffer(name)

def appendBuffer(console='main'):
    mudlet.appendBuffer(console)

def getLineNumber():
    return mudlet.getLineNumber()

def copy(console='main'):
    mudlet.copy(console)

def paste(console='main'):
    mudlet.paste(console)

def cut():
    mudlet.cut()

def feedTriggers(txt):
    mudlet.feedTriggers(txt)

def setBold(doBold,console='main'):
    """console=The console you want to set, doBold=True/False for setting
       bold on or off."""
    mudlet.setBold(console,active)

def setUnderline(doUnderline,console='main'):
    """console=The console you want to set, doUnderline=True/False for setting
       Underline on or off."""
    mudlet.setUnderline(console,active)

def setItalics(doItalics,console='main'):
    """console=The console you want to set, doItalics=True/False for setting
       Italics on or off."""
    mudlet.setItalics(console,active)

def moveCursor(xpos, ypos, console='main'):
    mudlet.moveCursor(xpos,ypos,console)

def moveCursorEnd(console='main'):
    mudlet.moveCursorEnd(console)

def pasteWindow(console='main'):
    mudlet.pasteWindow(console)

def selectCurrentLine(console='main'):
    mudlet.selectCurrentLine(console)

def wrapLine(linenum, console='main'):
    mudlet.wrapLine(linenum, console)

def getFgColor(console='main'):
    return mudlet.getFgColor(console)

def getBgColor(console='main'):
    return mudlet.getBgColor(console)

def insertHTML(txt):
    mudlet.insertHTML(txt)

def insertText(txt,console='main'):
    mudlet.insertText(txt, console)

def isAnsiFgColor(color, console='main'):
    return mudlet.isAnsiFgColor(color, console)

def isAnsiBgColor(color, console='main'):
    return mudlet.isAnsiBgColor(color, console)
    
def getRGB(color):
    r,g,b = color_dict[color.strip()]
    return r,g,b

def getCurrentLine(console='main'):
    return mudlet.getCurrentLine(console)
    
def appendCmdLine(txt):
    mudlet.appendCmdLine(txt)
    
def denyCurrentSend():
    mudlet.denyCurrentSend()
    
def getLastLineNumber(console='main'):
    return mudlet.getLastLineNumber(console)
    
def getLineCount(console='main'):
    return mudlet.getLineCount(console)

def prefix(what, func="None", fg="", bg="", console='main'):
    moveCursor(0,getLineNumber(), console)
    if func=="None":
        insertText(what,console)
    elif func=='cecho':
        cecho("<"+fg+","+bg+">"+what,console,insert=True)
    elif func=='decho':
        decho("<"+str(fg[0])+","+str(fg[1])+","+str(fg[2])+":"+str(bg[0])+","+str(bg[1])+","+str(bg[2])+">"+what,console,insert=True)
    elif func=='hecho':
        hecho("|c"+fg+","+bg+what,console,insert=True)

def suffix(what, func="None", fg="", bg="", console='main'):
    length = len(line)
    moveCursor(length, getLineNumber(), console)
    if func=="None":
        insertText(what,console)
    elif func=='cecho':
        cecho("<"+fg+","+bg+">"+what,console,insert=True)
    elif func=='decho':
        decho("<"+str(fg[0])+","+str(fg[1])+","+str(fg[2])+":"+str(bg[0])+","+str(bg[1])+","+str(bg[2])+">"+what,console,insert=True)
    elif func=='hecho':
        hecho("|c"+fg+","+bg+what,console,insert=True)

def getLines(From,To):
    return mudlet.getLines(From,To)

def getTime(return_string,fmt="yyyy.MM.dd hh:mm:ss.zzz"):
    if return_string==True:
	time=mudlet.getTime(return_string,fmt)[0]
	time=str(time)
        return time
    else:
	time=dict(zip(("hour","min",'sec','msec','year','month','day'),mudlet.getTime(return_string,fmt)))
        return time

def getTimeStamp(line,console='main'):
    return mudlet.getTimeStamp(line,console)

def isPrompt():
    return mudlet.isPrompt()

def startLogging(logOn):
    mudlet.startLogging(logOn)

def isActive(obj, objType):
    return mudlet.isActive(obj,objType)

def killAlias(obj):
    return mudlet.killAlias(obj)

def killTrigger(obj):
    return mudlet.killTrigger(obj)

def killTimer(obj):
    return mudlet.killTimer(obj)

def exists(obj, objType):
    return mudlet.exists(obj, objType)

def setTriggerStayOpen(name,numOfLines):
    mudlet.setTriggerStayOpen(name,numOfLines)

def showMultimatches():
    echo("\n-------------------------------------------------------");
    echo("\nThe table multimatches[n][m] contains:");
    echo("\n-------------------------------------------------------");
    for i in range(0,len(multimatches)):
        echo("\nregex " + str(i) + " captured: (multimatches["+ str(i) +"][1-n])");
        for i2 in range(0,len(multimatches[i])):
                echo("\n          key="+str(i2)+" value="+multimatches[i][i2]);
    echo("\n-------------------------------------------------------\n");

def createStopWatch():
    return mudlet.createStopWatch()

def stopStopWatch( ID ):
    return mudlet.stopStopWatch( ID )

def startStopWatch( ID ):
    return mudlet.startStopWatch( ID )

def resetStopWatch( ID ):
    return mudlet.startStopWatch( ID )

def getStopWatchTime( ID ):
    return mudlet.getStopWatchTime( ID )

def getMudletHomeDir():
    return mudlet.getMudletHomeDir()

def getNetworkLatency():
    return mudlet.getNetworkLatency()

def resetProfile():
    mudlet.resetProfile()

def connectToServer(port, url):
    mudlet.connectToServer(port, url)

def downloadFile(path, url):
    mudlet.downloadFile(path, url)

def invokeFileDialog(directory, title):
    return mudlet.invokeFileDialog(directory, title)

def loadRawFile(the_file):
    mudlet.loadRawFile(the_file)

def playSoundFile(sound):
    mudlet.playSoundFile(sound)

def sendSocket(txt):
    mudlet.sendSocket(txt)

def permTimer(name,folder,time,func):
    return mudlet.startPermTimer(name,folder,time,func)

def tempBeginOfLineTrigger(regex,func):
    return mudlet.startTempBeginOfLineTrigger(regex,func)

def tempTimer(time, func):
    return mudlet.startTempTimer(time, func)

def permAlias(name,folder,regex,func):
    return mudlet.startPermAlias(name,folder,regex,func)

def tempAlias(regex,func):
    return mudlet.startTempAlias(regex,func)

def tempExactMatchTrigger(regex,func):
    return mudlet.startTempExactMatchTrigger(regex,func)

def tempTrigger(regex,func):
    return mudlet.startTempTrigger(regex,func)

def tempLineTrigger(_from, howmany, func):
    return mudlet.startTempLineTrigger(_from, howmany, func)

def tempColorTrigger(fg, bg, func):
    return mudlet.startTempColorTrigger(fg, bg, func)

def permRegexTrigger(name,folder,regexlist,func):
    return mudlet.startPermRegexTrigger(name,folder,regexlist,func)

def permSubstringTrigger(name,folder,regexlist,func):
    return mudlet.startPermSubstringTrigger(name,folder,regexlist,func)

def permBeginOfLineStringTrigger(name,folder,regexlist,func):
    return mudlet.startPermBeginOfLineStringTrigger(name,folder,regexlist,func)

def clearUserWindow(console):
    mudlet.clearUserWindow(console)

clearWindow=clearUserWindow
clearConsole=clearUserWindow

def createLabel(name, xpos, ypos, width, height, fillBackground):
    return mudlet.createLabel(name, xpos, ypos, width, height, fillBackground)

def createMiniConsole(name, xpos, ypos, width, height):
    return mudlet.createMiniConsole(name, xpos, ypos, width, height)

def echoUserWindow(console, txt):
    mudlet.echoUserWindow(console, txt)

def getButtonState():
    return mudlet.getButtonState()

def getMainConsoleWidth():
    return mudlet.getMainConsoleWidth()

def getMainWindowSize():
    return mudlet.getMainWindowSize()

def calcFontSize(size):
    return mudlet.calcFontSize(size)

def hasFocus():
    return mudlet.hasFocus()

def hideToolBar(name):
    mudlet.hideToolBar(name)

def showToolBar(name):
    mudlet.showToolBar(name)

def moveWindow(console, xpos, ypos):
    mudlet.moveWindow(console, xpos, ypos)

moveUserWindow=moveWindow

def hideWindow(console):
    return mudlet.hideUserWindow(console)

hideUserWindow=hideWindow

def showWindow(console):
    return mudlet.showUserWindow(console)

showUserWindow=showWindow

def openUserWindow(name):
    mudlet.openUserWindow(name)

openWindow=openUserWindow

def resizeUserWindow(name, x, y):
    mudlet.resizeUserWindow(name, x, y)

resizeWindow=resizeUserWindow

def setBackgroundColor(console, r, g, b, alpha):
    mudlet.setBackgroundColor(console, r, g, b, alpha)

def setBackgroundImage(console, path):
    mudlet.setBackgroundImage(console, path)

def setBorderTop(size):
    mudlet.setBorderTop(size)

def setBorderBottom(size):
    mudlet.setBorderBottom(size)

def setBorderLeft(size):
    mudlet.setBorderLeft(size)

def setBorderRight(size):
    mudlet.setBorderRight(size)

def setConsoleBufferSize(console, limit, delete_batch):
    mudlet.setConsoleBufferSize(console, limit, delete_batch)

def setMainWindowSize(wide, height):
    mudlet.setMainWindowSize(wide, height)

def setMiniConsoleFontSize(console, size):
    mudlet.setMiniConsoleFontSize(console, size)

def setTextFormat(console, fR, fG, fB, bR, bG ,bB, bold, underline, italics):
    return mudlet.setTextFormat(console, fR, fG, fB, bR, bG ,bB, bold, underline, italics)

def setWindowWrap(console, wrap):
    return mudlet.setWindowWrap(console, wrap)

def setWindowWrapIndent(console, indent):
    mudlet.setWindowWrapIndent(console, indent)

def createConsole(consoleName, fontSize, charsPerLine, numberOfLines, Xpos, Ypos):
   createMiniConsole(consoleName,0,0,1,1)
   setMiniConsoleFontSize(consoleName, fontSize)
   x,y = calcFontSize( fontSize )
   resizeWindow(consoleName, x*charsPerLine, y*numberOfLines)
   setWindowWrap(consoleName, Xpos)
   moveWindow(consoleName, Xpos, Ypos)
   setBackgroundColor(consoleName,0,0,0,0)
   setFgColor(255,255,255,consoleName)

#Dict holding information about Gauges.
gaugesTable={}

def createGauge(gaugeName, width, height, Xpos, Ypos, gaugeText, color1, color2="", color3=""):
        createLabel(gaugeName+"_back",0,0,0,0,1)
        if color2=="":
                red, green, blue = getRGB(color1)
                setBackgroundColor(gaugeName+"_back", red , green, blue, 100)
        else:
                setBackgroundColor(gaugeName+"_back", color1 ,color2, color3, 100)
        moveWindow(gaugeName+"_back", Xpos, Ypos)
        resizeWindow(gaugeName+"_back", width, height)
        showWindow(gaugeName+"_back")
        createLabel(gaugeName,0,0,0,0,1)
        if color2 == "":
                red, green, blue = getRGB(color1)
                setBackgroundColor(gaugeName, red , green, blue, 255)
        else:
                setBackgroundColor(gaugeName, color1 ,color2, color3, 255)
        moveWindow(gaugeName, Xpos, Ypos)
        resizeWindow(gaugeName, width, height)
        showWindow(gaugeName)

        gaugesTable[gaugeName] = {"width" : width, "height" : height, "xpos" : Xpos, "ypos" : Ypos,"text" : gaugeText, "color1" : color1, "color2" : color2, "color3" : color3}

        if gaugeText != "":
                setGaugeText(gaugeName,gaugeText, "black")
        else:
                setGaugeText(label=gaugeName)

def moveGauge(gaugeName, newX, newY):
        #assert(gaugesTable[gaugeName], "moveGauge: no such gauge exists.")
        #assert(newX and newY, "moveGauge: need to have both X and Y dimensions.")
        moveWindow(gaugeName, newX, newY)
        moveWindow(gaugeName+"_back", newX, newY)
        gaugesTable[gaugeName]["xpos"], gaugesTable[gaugeName]["ypos"] = newX, newY

def resizeGauge(gaugeName, width, height):
    #assert(gaugesTable[gaugeName], "resizeGauge: no such gauge exists.")
    #assert(width and height, "resizeGauge: need to have both width and height.")
    resizeWindow(gaugeName, width, height)
    resizeWindow(gaugeName+"_back", width, height)
    gaugesTable[gaugeName]["width"], gaugesTable[gaugeName]["height"] = width, height

def setGaugeStyleSheet(gaugeName, css):
    #assert(gaugesTable[gaugeName], "setGaugeStyleSheet: no such gauge exists.")
    setLabelStyleSheet(gaugeName, css)
    setLabelStyleSheet(gaugeName+"_back", css)

def RGB2Hex(red, green, blue):
    _hex="%0.2X" % red+"%0.2X" % green+"%0.2X" % blue
    return _hex

def setGaugeText(gaugeName, gaugeText="", color1="", color2="", color3=""):
        #assert(gaugesTable[gaugeName], "setGauge: no such gauge exists.")
        red,green,blue = 0,0,0
        l_labelText = gaugeText
        if color1 != "":
                if color2 == "":
                        red, green, blue = getRGB(color1)
                else:
                        red, green, blue = color1, color2, color3
        l_EchoString = "<font color=#"+str(RGB2Hex(red,green,blue))+">"+l_labelText+"</font>"
        echo(l_EchoString,gaugeName)
        echo(l_EchoString,gaugeName+"_back")
        gaugesTable[gaugeName]["text"] = l_EchoString
        gaugesTable[gaugeName]["color1"], gaugesTable[gaugeName]["color2"], gaugesTable[gaugeName]["color3"] = color1, color2, color3

def setLabelClickCallback(label,func,args=""):
    mudlet.setLabelClickCallback(label,func,args)

def setLabelStyleSheet(label,sheet):
    mudlet.setLabelStyleSheet(label,sheet)

def setGauge(name,value,maxValue,txt=""):
        #assert(gaugesTable[gaugeName], "setGauge: no such gauge exists.")
        #assert(currentValue and maxValue, "setGauge: need to have both current and max values.")

        resizeWindow(name, gaugesTable[name].width/100*((100/maxValue)*value), gaugesTable[name].height)

        # if we wanted to change the text, we do it
        if gaugeText != "" :
                echo(name+"_back", txt)
                echo(name, txt)
                gaugesTable[name].text = txt
    
try:
    #Put code in PythonLocal.py file for code you would like to run on startup of mudlet.
    execfile('PythonLocal.py')
catch IOError:
    pass
