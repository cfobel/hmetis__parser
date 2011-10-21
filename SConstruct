import SCons.Scanner.C
import SCons.Tool

# SConstruct file
env=Environment(CPPPATH=['../'])
SCons.Tool.SourceFileScanner.add_scanner('.rl', SCons.Scanner.C.CScanner())

ragel_bld = Builder(action = '/usr/bin/ragel -G2 -o $TARGET $SOURCE',
              suffix = '.cpp', src_suffix = '.rl',
              source_scanner = SCons.Scanner.C.CScanner())
ragel_dot_bld = Builder(action = '/usr/bin/ragel -o $TARGET -V -p $SOURCE',
              suffix = '.dot', src_suffix = '.rl')
dot_bld = Builder(action = '/usr/bin/dot -o $TARGET -Tpdf $SOURCE',
              suffix = '.pdf', src_suffix = '.dot')


# Add the new Builder to the list of builders
env['BUILDERS']['Ragel'] = ragel_bld
env['BUILDERS']['RagelDot'] = ragel_dot_bld
env['BUILDERS']['Dot'] = dot_bld

DEBUG = ARGUMENTS.get('DEBUG', 0)

if DEBUG:
    env.Append(CPPFLAGS=['-g'])
    env.Append(LINKFLAGS=['-g'])
else:
    env.Append(CPPFLAGS=['-O3'])

# Generate foo.vds from foo.txt using mk_vdu
vpr_source = env.Ragel('VPRNetParser.rl')
#env.Program('VPRNetParser', ['main.cpp', vpr_source])

dot = env.RagelDot('VPRNetParser.rl')
env.Dot(dot)

env.Export('vpr_source')
