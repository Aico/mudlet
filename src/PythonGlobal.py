import io
import site
import traceback
from PythonQt.mudlet import *

mudlet = MudletObject(HOST_HASH)

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
    """Run when connection established
        Placeholder, should reimplement in PythonLocal.py"""
    
def onDisconnect():
    """Run on disconnect
        Placeholder, should reimplement in PythonLocal.py"""
    
def handleWindowResizeEvent():
    """Run when window resizes
        Placeholder, should reimplement in PythonLocal.py"""

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

execfile('PythonLocal.py')
