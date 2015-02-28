#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "crc.h"

int main(int argc, char *argv[]) {
  printf("CRC examples\n");
  unsigned char* buffer = (unsigned char*)malloc(512);
  memset(buffer, 0, 512);
  int crc = crc16_t10dif_by4(0x1234, buffer, 512);
  int crc_base = crc16_t10dif_base(0x1234, buffer, 512);
  printf("CRC: %d, %d\n", crc, crc_base);
  free(buffer);
  return 0;
}


