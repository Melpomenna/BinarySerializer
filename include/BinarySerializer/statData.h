/**
 * @file StatData.h
 * @brief Определение структуры данных
 *
 * Этот заголовочный файл содержит определение структуры StatData,
 * которая используется для хранения статистической информации
 *
 * @author Melpomenna
 * @version 1.0
 * @date 03.11.2025
 */

#ifndef BINARYSERIALIZER_STATDATA_H
#define BINARYSERIALIZER_STATDATA_H

typedef struct StatData {
  long id;
  int count;
  float cost;
  unsigned int primary : 1;
  unsigned int mode : 3;
} StatData;

#endif // BINARYSERIALIZER_