# SConstruct file
env=Environment()

ragel_bld = Builder(action = '/usr/bin/ragel -G2 -o $TARGET $SOURCE',
              suffix = '.cpp', src_suffix = '.rl')
ragel_dot_bld = Builder(action = '/usr/bin/ragel -o $TARGET -V -p $SOURCE',
              suffix = '.dot', src_suffix = '.rl')
dot_bld = Builder(action = '/usr/bin/dot -o $TARGET -Tpdf $SOURCE',
              suffix = '.pdf', src_suffix = '.dot')


# Add the new Builder to the list of builders
env['BUILDERS']['Ragel'] = ragel_bld
env['BUILDERS']['RagelDot'] = ragel_dot_bld
env['BUILDERS']['Dot'] = dot_bld

# Generate foo.vds from foo.txt using mk_vds
vpr_source = env.Ragel('VPRNetParser.rl')
Program('VPRNetParser', ['main.cpp', vpr_source])

dot = env.RagelDot('VPRNetParser.rl')
env.Dot(dot)
