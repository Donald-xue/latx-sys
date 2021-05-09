incapstone_data = configuration_data()
incapstone_data.set('CAPSTONE_USE_SYS_DYN_MEM', '1')

incapstone_files = files(
  'capstone/cs.c',
  'capstone/MCInst.c',
  'capstone/MCInstrDesc.c',
  'capstone/MCRegisterInfo.c',
  'capstone/SStream.c',
  'capstone/utils.c'
)

incapstone_data.set('CAPSTONE_HAS_X86', 1)
incapstone_files += files(
  'capstone/arch/X86/X86Disassembler.c',
  'capstone/arch/X86/X86DisassemblerDecoder.c',
  'capstone/arch/X86/X86ATTInstPrinter.c',
  'capstone/arch/X86/X86IntelInstPrinter.c',
  'capstone/arch/X86/X86Mapping.c',
  'capstone/arch/X86/X86Module.c'
)

configure_file(output: 'capstone-defs.h', configuration: incapstone_data)

incapstone_cargs = [
  # FIXME: There does not seem to be a way to completely replace the c_args
  # that come from add_project_arguments() -- we can only add to them.
  # So: disable all warnings with a big hammer.
  '-Wno-error', '-w',

  # Include all configuration defines via a header file, which will wind up
  # as a dependency on the object file, and thus changes here will result
  # in a rebuild.
  '-include', 'capstone-defs.h'
]

libincapstone = static_library('capstone',
                             build_by_default: false,
                             sources: incapstone_files,
                             c_args: incapstone_cargs,
                             include_directories: 'capstone/include')
incapstone = declare_dependency(link_with: libincapstone,
                              include_directories: 'capstone/include/capstone')

i386_ss.add(when: 'CONFIG_LATX', if_true: files(
  'env.c',
  'error.c',
  'mem.c',
  'latx-config.c',
  'latx-options.c',
  'latx-profile.c'
))

i386_ss.add(when: 'CONFIG_LATX', if_true: incapstone)

subdir('diStorm')
subdir('ir1')
subdir('ir2')
subdir('optimization')
subdir('translator')
subdir('wrapper')
