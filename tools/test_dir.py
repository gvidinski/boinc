#!/usr/bin/env python

import boinc_path_config
from Boinc import boinc_project_path, tools
from Boinc.setup_project import *
import os, getopt

home = os.path.expanduser('~')
dest_dir = ('~/projects/periodsearch/')

def mkdir2(d):
    try:
        os.makedirs(d)
    except OSError as e:
        if not os.path.isdir(d):
            raise SystemExit(e)
            
    directories = ('html/user/img',
                   'html/user/css',
                   'html/user/fonts',
    )
    [ mkdir2(os.path.join(dest_dir, x)) for x in directories ]