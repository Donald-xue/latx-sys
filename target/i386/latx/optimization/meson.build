i386_ss.add(when: 'CONFIG_LATX', if_true: files(
  'flag-lbt.c',
  'flag-pattern.c',
  'flag-reduction.c',
  'ibtc.c',
  'ir2-optimization.c',
  'shadow-stack.c'
))

i386_softmmu_ss.add(when: 'CONFIG_LATX', if_true: files(
  'sys-lsfpu.c',
  'sys-staticcs.c',
  'sys-intb.c',
  'sys-intb-njc.c',
  'sys-sigint.c',
  'sys-fastcs.c',
  'sys-cc-pro.c',
  'sys-flag-rdtn.c',
  'sys-inst-ptn.c',
  'sys-em.c'
))
