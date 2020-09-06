/*
 * parse.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Packet starts at offset+1 */
void parse_packet(uint16_t len, uint16_t offset, uint8_t *packet) {
  for (uint16_t i = offset+1; i < len + offset; i++) {
    printf("%d ", packet[i]);
  }
  printf("\n");
}
