#ifndef __CONFIG_H__
#define __CONFIG_H__

#define HAVE_INTERP_H

#define HAVE_ALLOCA 1
#define HAVE_ALLOCA_H 1

/* widths of primitive types */

#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_VOID_P 4

#define squeakInt64 long long

#define DOUBLE_WORD_ORDER 1

/* other configured variables */

#define SQ_VERSION "3.9a-7021"
#define VM_VERSION "3.9-4"
#define VM_LIBDIR "/usr/local/lib/squeak/3.9-4"
#define VM_MODULE_PREFIX ""

#endif /* __CONFIG_H__ */
