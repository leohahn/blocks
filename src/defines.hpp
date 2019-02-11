#ifndef BLOCKS_DEFINES_H
#define BLOCKS_DEFINES_H

#define KILOBYTES(x) (x)*1024
#define MEGABYTES(x) KILOBYTES(x)*1024
#define GIGABYTES(x) MEGABYTES(x)*1024

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

#ifdef NDEBUG
#define BLOCKS_PRODUCTION 1
#else
#define BLOCKS_DEBUG 1
#endif

#endif // BLOCKS_DEFINES_H
