from subprocess import check_output, CalledProcessError

import SCons.Scanner.C
import SCons.Tool

# SConstruct file
env=Environment(CPPPATH=['../'])
SCons.Tool.SourceFileScanner.add_scanner('.rl', SCons.Scanner.C.CScanner())

ragel_path = check_output('which ragel', shell=True).strip()
ragel_bld = Builder(action = '%s -G2 -o $TARGET $SOURCE' % ragel_path,
              suffix = '.cpp', src_suffix = '.rl',
              source_scanner = SCons.Scanner.C.CScanner())
ragel_dot_bld = Builder(action = '%s -o $TARGET -V -p $SOURCE' % ragel_path,
              suffix = '.dot', src_suffix = '.rl')



# Add the new Builder to the list of builders
env['BUILDERS']['Ragel'] = ragel_bld
env['BUILDERS']['RagelDot'] = ragel_dot_bld

DEBUG = ARGUMENTS.get('DEBUG', 0)
WITH_MAIN = ARGUMENTS.get('WITH_MAIN', 0)

if DEBUG:
    env.Append(CPPFLAGS=['-g'])
    env.Append(LINKFLAGS=['-g'])
else:
    env.Append(CPPFLAGS=['-O3'])

# Generate foo.vds from foo.txt using mk_vdu
hmetis_source = env.Ragel('HMetisResultParser.rl')
#env.Program('VPRNetParser', ['main.cpp', vpr_source])

try:
    dot_path = check_output('which dot', shell=True).strip()
    dot_bld = Builder(action = '%s -o $TARGET -Tpdf $SOURCE' % dot_path,
                suffix = '.pdf', src_suffix = '.dot')
    env['BUILDERS']['Dot'] = dot_bld
    dot = env.RagelDot('HMetisResultParser.rl')
    env.Dot(dot)
except CalledProcessError:
    pass

if WITH_MAIN:
    env.Program(['PartitionReader.cpp', hmetis_source])
env.Export('hmetis_source')
