#!/usr/bin/env python3

import os
import sys
import tools.resource_compiler as rc

INCLUDES  = ['#include <cinttypes>']
TARGET    = 'pfm2sid'
NAMESPACE = 'pfm2sid::resources'

HEADER=""""""

import lcd_chars

resource_tables = [ lcd_chars.BuildResourceTable() ]
resource_library = rc.ResourceLibrary(resource_tables, target=TARGET, namespace=NAMESPACE, includes=INCLUDES, header=HEADER)

if __name__ == '__main__':
    basename = os.path.splitext(sys.argv[1])[0]
    rc.Process(resource_library, basename)
