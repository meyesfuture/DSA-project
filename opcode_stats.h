#ifndef OPCODE_STATS_H
#define OPCODE_STATS_H

#include <stddef.h>
#include <stdint.h>

#define OPCODE_STATS_TABLE_SIZE 256

struct opcode_node {
    uint16_t opcode;
    uint64_t count;
    struct opcode_node *next;
};

struct opcode_stats {
    struct opcode_node *table[OPCODE_STATS_TABLE_SIZE];
    uint64_t total;
    size_t unique;
};

void opcode_stats_init(struct opcode_stats *stats);
int opcode_stats_record(struct opcode_stats *stats, uint16_t opcode);
void opcode_stats_print(const struct opcode_stats *stats);
void opcode_stats_reset(struct opcode_stats *stats);

#endif
