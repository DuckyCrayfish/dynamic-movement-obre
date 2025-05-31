#ifndef MOD_NAME
#error "MOD_NAME must be defined."
#endif

#ifndef MOD_VERSION
#error "MOD_VERSION must be defined."
#endif

#define STRINGIFY(x) STR(x)

#define MOD_NAME_STR STRINGIFY(MOD_NAME)
#define MOD_VERSION_STR STRINGIFY(MOD_VERSION)
