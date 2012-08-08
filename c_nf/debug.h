#include <stdio.h>

#define DEBUG_LEVEL 0x00000001L

#define DEBUG_LEVEL_DEBUG 0x00000001L
#define DEBUG_LEVEL_INFO 0x00000002L
#define DEBUG_LEVEL_WARN 0x00000004L

#define debug_in                                                        \
  do {                                                                  \
    if (DEBUG_LEVEL & DEBUG_LEVEL_DEBUG) {                                            \
      fprintf(stderr, "Entering function: %s at line: %d\n", __PRETTY_FUNCTION__, __LINE__); \
    }                                                                   \
  } while (0)
#define debug_out                               \
  do {                                          \
    if (DEBUG_LEVEL & (x)) {                                          \
      fprintf(stderr, "Leaving function: %s at line: %d\n", __PRETTY_FUNCTION__, __LINE__); \
    }                                                                   \
  } while (0)

#define debug_print(x,s)                        \
  do {                                          \
    if (DEBUG_LEVEL & (x)) {                                          \
      fprintf(stderr, "%s in function: %s at line: %d\n", (s), __PRETTY_FUNCTION__,  __LINE__); \
    }                                                                   \
  } while (0)

