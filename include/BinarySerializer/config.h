#ifndef BINARYSERIALIZER_CONFIG_H
#define BINARYSERIALIZER_CONFIG_H

#define BINARYSERIALIZER_API __attribute__((visibility("default")))
#define BINARYSERIALIZER_NODISCARD __attribute__((warn_unused_result))

#define BINARYSERIALIZER_LIKELY(__condition)                                   \
  __builtin_expect(!!(__condition), 1)
#define BINARYSERIALIZER_UNLIKELY(__condition)                                 \
  __builtin_expect(!!(__condition), 0)

#endif // BINARYSERIALIZER_CONFIG_H