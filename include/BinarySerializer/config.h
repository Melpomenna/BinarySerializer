#ifndef BINARYSERIALIZER_CONFIG_H
#define BINARYSERIALIZER_CONFIG_H

#define BINARYSERIALIZER_API __attribute__((visibility("default")))
#define BINARYSERIALIZER_NODISCARD __attribute__((warn_unused_result))

#define BINARYSERIALIZER_LIKELY(__condition)                                   \
  __builtin_expect(!!(__condition), 1)
#define BINARYSERIALIZER_UNLIKELY(__condition)                                 \
  __builtin_expect(!!(__condition), 0)

#if defined(NDEBUG)
#define LOG_ERR(...)
#define LOG(...)
#else
#define LOG_ERR(__format, ...) fprintf(stderr, __format, __VA_ARGS__)
#define LOG(__format, ...) fprintf(stdout, __format, __VA_ARGS__)
#endif

#endif // BINARYSERIALIZER_CONFIG_H