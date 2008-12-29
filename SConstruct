
env = Environment()

# OPTIONS #########################################################################

opts = Options('options.conf')
opts.AddOptions(
	PathOption('DESTDIR','Set the intermediate install "prefix"', '/',
		PathOption.PathAccept),
	PathOption('PREFIX', 'Set the install "prefix"', '/usr',
		PathOption.PathAccept)
)
opts.Update(env)
opts.Save('options.conf',env)

###################################################################################





# DEPENDENCIES ####################################################################
deps = (
	'cairomm-1.0',
	'sigc-2.0',
	'gtkmm-2.4',
	'libglademm-2.4',
	'Xtst',
	'gthread-2.0'
)

conf = Configure(env)
for l in deps:
	#if not conf.CheckLib(l): exit(1)
	conf.CheckLib(l)
env = conf.Finish()

###################################################################################




# COMPILER OPTIONS FOR LIBS #######################################################

libs = (
	'cairomm-1.0',
	'sigc++-2.0',
	'gtkmm-2.4',
	'libglademm-2.4',
	'xtst',
	'gthread-2.0'
)

for l in libs:
	env.ParseConfig('pkg-config ' + l + ' --cflags --libs')

env.Append(LIBS = ['cwiid'])


###################################################################################

prefix_pixmapsdir = env['PREFIX'] + '/share/pixmaps' 
prefix_windowsdir = env['PREFIX'] + '/share/whiteboard/gtk'
prefix_bindir = env['PREFIX'] + '/bin'

dst_pixmapsdir = env['DESTDIR'] + prefix_pixmapsdir
dst_windowsdir = env['DESTDIR'] + prefix_windowsdir
dst_bindir = env['DESTDIR'] + prefix_bindir

def quote(t):
	return '\\"' + t + '\\"'


env.Append(CPPDEFINES=[
	('WINDOWSDIR',quote(prefix_windowsdir)),
	('PIXMAPSDIR',quote(prefix_pixmapsdir)),
	('GETTEXT_PACKAGE', quote('whiteboard')),
	('WHITEBOARDLOCALEDIR', quote('.'))
])

whiteboard = env.Program('whiteboard', Glob('src/*.cpp'))




# INSTALL ##########################################################################

if 'install' in COMMAND_LINE_TARGETS:
	env.Install(dst_pixmapsdir, Glob('src/pixmaps/*'))
	env.Install(dst_windowsdir, Glob('src/pixmaps/*'))
	env.Install(dst_windowsdir, Glob('src/windows/gtk/*'))
	env.Alias('install',dst_pixmapsdir)
	env.Alias('install',dst_windowsdir)

	env.Install(dst_bindir,whiteboard)
	env.Alias('install',dst_bindir)

#####################################################################################
