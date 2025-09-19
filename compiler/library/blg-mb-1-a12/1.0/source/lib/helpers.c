#ifdef _LIB_HELPERS
#else
#define _LIB_HELPERS

__attribute__((always_inline))
static inline
uint16_t abs16 ( int16_t input ) {
  if ( input < 0 )
    return (uint16_t)(-input);
  else
    return (uint16_t)(input);
}

__attribute__((always_inline))
static inline
uint16_t min ( uint16_t one, uint16_t two ) {
  if ( one < two )
    return one;
  else
    return two;
}

__attribute__((always_inline))
static inline
float minf ( float one, float two ) {
  if ( one < two )
    return one;
  else
    return two;
}

__attribute__((always_inline))
static inline
uint16_t max ( uint16_t one, uint16_t two ) {
  if ( one > two )
    return one;
  else
    return two;
}

__attribute__((always_inline))
static inline
float maxf ( float one, float two ) {
  if ( one > two )
    return one;
  else
    return two;
}

uint8_t
isAlphaNum
( uint8_t byte ) {
  if ( byte >= '0' && byte <= '9' ) {
    return 1;
  }
  if ( byte >= 'a' && byte <= 'z' ) {
    return 1;
  }
  if ( byte >= 'A' && byte <= 'Z' ) {
    return 1;
  }
  return 0;
}

#endif
