import tools.resource_compiler as rc

# Build HD44780 style character bitmaps from 5x8 string definition
def BuildCharacter(char):
    bytes = []
    rows = char.strip().split('\n')
    assert len(rows) == 8
    for row in rows:
        assert len(row) == 5
        byte = 0
        for c in row.strip():
            byte *= 2
            if c == 'X': byte += 1
        bytes.append(byte)
    return bytes

custom_characters = {
'note' :
"""
...X.
...XX
...XX
...X.
...X.
.XXX.
XXXX.
.XX..
""",
'bend' :
"""
.....
.....
.....
..X..
..X..
.XXX.
X...X
XXXXX
""",
'tri' :
"""
.....
.....
.X...
X.X.X
...X.
.....
.....
.....
""",
'saw' :
"""
.....
...XX
..X.X
.X..X
X...X
.....
.....
.....
""",
'square' :
"""
.....
XXX..
X.X..
..X.X
..XXX
.....
.....
.....
""",
}

def BuildResourceTable():
    resource_table = {
            'name' : 'lcd_char',
            'prefix' : 'LCD_CHAR',
            'type' : rc.ResourceTableEnumerated,
            'c_type' : 'uint8_t * const',
            'use_aliases' : False,
            'entries' : [ (k, rc.ArrayResource('data', 'uint8_t', BuildCharacter(v), formatter=lambda x: "0x%02x" % x)) for k, v in custom_characters.items() ],
            'includes' : [],
            }
    return resource_table
