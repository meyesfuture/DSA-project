#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "opcode_stats.h"

static size_t opcode_hash(uint16_t opcode) {
    unsigned int high = opcode >> 8;
    unsigned int low = opcode & 0xFF;

    return (high * 31u + low) % OPCODE_STATS_TABLE_SIZE;
}

void opcode_stats_init(struct opcode_stats *stats) {
    if (stats == NULL) {
        return;
    }

    for (size_t i = 0; i < OPCODE_STATS_TABLE_SIZE; i++) {
        stats->table[i] = NULL;
    }

    stats->total = 0;
    stats->unique = 0;
}

int opcode_stats_record(struct opcode_stats *stats, uint16_t opcode) {
    if (stats == NULL) {
        return 0;
    }

    size_t index = opcode_hash(opcode);
    struct opcode_node *node = stats->table[index];

    while (node != NULL) {
        if (node->opcode == opcode) {
            node->count++;
            stats->total++;
            return 1;
        }

        node = node->next;
    }

    node = malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }

    node->opcode = opcode;
    node->count = 1;
    node->next = stats->table[index];
    stats->table[index] = node;

    stats->total++;
    stats->unique++;
    return 1;
}

void opcode_stats_print(const struct opcode_stats *stats) {
    if (stats == NULL) {
        return;
    }

    printf("\nCHIP-8 OPCODE EXECUTION STATISTICS\n\n");
    printf("%-12s %s\n", "Opcode", "Executions");

    for (size_t i = 0; i < OPCODE_STATS_TABLE_SIZE; i++) {
        const struct opcode_node *node = stats->table[i];

        while (node != NULL) {
            printf("0x%04X       %" PRIu64 "\n", (unsigned int)node->opcode, node->count);
            node = node->next;
        }
    }

    printf("\nTotal instructions executed: %" PRIu64 "\n", stats->total);
    printf("Unique opcodes encountered: %lu\n\n", (unsigned long)stats->unique);
}

void opcode_stats_reset(struct opcode_stats *stats) {
    if (stats == NULL) {
        return;
    }

    for (size_t i = 0; i < OPCODE_STATS_TABLE_SIZE; i++) {
        struct opcode_node *node = stats->table[i];

        while (node != NULL) {
            struct opcode_node *next = node->next;
            free(node);
            node = next;
        }

        stats->table[i] = NULL;
    }

    stats->total = 0;
    stats->unique = 0;
}
