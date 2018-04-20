#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"


void initialize_ai();
void close_ai();
move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, int* nodes_generated, int* nodes_expanded);
node_t *create_node(); 
#endif
