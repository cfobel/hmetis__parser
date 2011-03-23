# SConstruct file
env=Environment()

ragel_bld = Builder(action = '/usr/bin/ragel -o $TARGET $SOURCE',
              suffix = '.c', src_suffix = '.rl')
ragel_dot_bld = Builder(action = '/usr/bin/ragel -o $TARGET -V -p $SOURCE',
              suffix = '.dot', src_suffix = '.rl')
dot_bld = Builder(action = '/usr/bin/dot -o $TARGET -O -Tpdf $SOURCE',
              suffix = '.pdf', src_suffix = '.dot')


# Add the new Builder to the list of builders
env['BUILDERS']['Ragel'] = ragel_bld
env['BUILDERS']['RagelDot'] = ragel_dot_bld
env['BUILDERS']['Dot'] = dot_bld

# Generate foo.vds from foo.txt using mk_vds
vpr_source = env.Ragel('vpr_netlist_parser.rl')
Program(vpr_source)

dot = env.RagelDot('vpr_netlist_parser.rl')
env.Dot('vpr_netlist_parser.dot')
