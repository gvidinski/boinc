#!/usr/bin/env python


'''
Usage: upgrade [options] project_root
Options:
  --srcdir          default: current directory
  --web_only        copy only web (PHP) files
  --server_only     copy everything except web files

Copy source/build files to a project tree,
overwriting what's already there.
Run it in the source directory (tools/upgrade).

If project_root is an absolute path, it's the project root.
Otherwise the project root is HOME/projects/project_root.

'''

import boinc_path_config
from Boinc import boinc_project_path, tools
from Boinc.setup_project import *
import os, getopt

def usage():
    print ("Usage: upgrade [--web_only | --server_only] [--srcdir DIR] project_root")
    raise SystemExit

def syntax_error(str):
    raise SystemExit('%s; See "%s --help" for help\n' % (str, sys.argv[0]))

try:
    opts, args = getopt.getopt(sys.argv[1:], '', ['help', 'web_only', 'server_only', 'srcdir='])
except getopt.GetoptError as e:
    syntax_error(e)

home = os.path.expanduser('~')

options.web_only = False
options.server_only = False
options.srcdir = None

for o,a in opts:
    if o == '--help':   usage()
    elif o == '--web_only': options.web_only = True
    elif o == '--server_only': options.server_only = True
    elif o == '--srcdir': options.srcdir = a
    else:
        raise SystemExit('internal error o=%s'%o)

if len(args) == 1:
    if os.path.isabs(args[0]):
        INSTALL_DIR = args[0]
    else:
        INSTALL_DIR = os.path.join(home, 'projects', args[0])
else:
    syntax_error('No project name')

if not options.srcdir:
    for dir in ('.', '..'):
        if os.path.exists(os.path.join(dir, 'html', 'project.sample', 'project.inc')):
            options.srcdir = dir
    if not options.srcdir:
        syntax_error('Not running in the source directory and --srcdir was not specified')

if not options.web_only:
    print ("Don't forget to do 'make' first!")
    
if not tools.query_noyes("Overwrite files in "+INSTALL_DIR):
    raise SystemExit

if not options.web_only:
    if os.system('cd '+INSTALL_DIR+'; ./bin/stop'):
        raise SystemExit("Couldn't stop project!")

print ("Upgrading files... ")

options.install_method = 'copy'

init()
    
if options.install_method == 'copy':
    options.install_function = shutil.copy

# install = options.install_function
def install(src, dest, unless_exists=False):
    if unless_exists and os.path.exists(dest):
        return
    try:
        options.install_function(src, dest)
    except:
        print('failed to copy ' + src + ' to ' + dest)
        return

def dest(self, *dirs):
        location = self.project_dir
        for x in dirs:
            location = os.path.join(location, x)
        return location

def srcdir(location):
    print (location)
    return os.path.join(options.srcdir, location)

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

install_boinc_files(INSTALL_DIR, not options.server_only, not options.web_only)

def upgrade_files(self):
    install(srcdir('html/inv/bootstrap.inc'),
        self.dest('html/inc/bootstrap.inc'))
    install(srcdir('html/inv/news.inc'),
        self.dest('html/inc/news.inc'))
    install(srcdir('html/inv/uotd.inc'),
        self.dest('html/inc/uotd.inc'))
    install(srcdir('html/inv/user.inc'),
        self.dest('html/inc/user.inc'))
    install(srcdir('html/inv/util.inc'),
        self.dest('html/inc/util.inc'))    
    
    install(srcdir('html/user/favicon.ico'),
        self.dest('html/user/favicon.ico'))
    install(srcdir('html/user/login_form.php'),
        self.dest('html/user/login_form.php'))
    install(srcdir('html/user/server_status.php'),
        self.dest('html/user/server_status.php'))
    install(srcdir('html/user/server_status.php'),
        self.dest('html/user/index.php'))
    
    install(srcdir('html/project.sample/project.inc'),
        self.dest('html/project/project.inc'))
    install(srcdir('html/project.sample/project_specific_prefs.inc'),
        self.dest('html/project/project_specific_prefs.inc'))
    install(srcdir('html/project.sample/cache_parameters.inc'),
        self.dest('html/project/cache_parameters.inc'))
    install(srcdir('html/project.sample/project_description.php'),
        self.dest('html/project/project_description.php'))
    install(srcdir('html/project.sample/daemons.inc'),
        self.dest('html/project/daemons.inc'))
    install(srcdir('html/project.sample/hosts.html'),
        self.dest('html/project/hosts.html'))
    install(srcdir('html/project.sample/google_tag.html'),
        self.dest('html/project/google_tag.html'))
    install(srcdir('html/project.sample/server_summary.inc'),
        self.dest('html/project/server_summary.inc'))
    
    install(srcdir('html/ops/ops_server_status.php'),
    self.dest('html/ops/ops_server_status.php'))

print ("Upgrading files... done")

print ("Updating translations")
try:
    os.system('cd '+INSTALL_DIR+'/html/ops; ./update_translations.php -d 1')
except:
    print ('''Couldn't install translation files''')

opt = '';
if options.server_only:
    opt = ' --server'
if options.web_only:
    opt = ' --web'

print ("Checking for DB updates")
os.system('cd '+INSTALL_DIR+'/html/ops; ./upgrade_db.php'+opt)

if os.path.exists(INSTALL_DIR+'/html/project/project_news.inc'):
    print ('''\

html/project/project_news.inc is deprecated.
Run html/ops/news_convert.php to convert project news to forum format.
''')

if not options.web_only:
    print ("Run `bin/start' to restart the project.")
