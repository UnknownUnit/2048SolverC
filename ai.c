  /*
   ============================================================================
   Name        : 2048.c
   Author      : Jia Shun Low (jlow3@student.unimelb.edu.au). 
   Description : Console version of the game "2048" for GNU/Linux
   ============================================================================
   */
  //ALGORITHMS AND DATA STRUCTURES ARE FUN
  #include <time.h>
  #include <stdlib.h>
  #include "ai.h"
  #include "utils.h"
  #include "priority_queue.h"

  //Some predefined constants.
  #define INITIAL_EXPLORE_SIZE 300
  #define LEFT 0
  #define RIGHT 1
  #define UP 2
  #define DOWN 3
  #define RESIZE 2
  
  struct heap h;
  
  void initialize_ai(){
    heap_init(&h);
  }

  /**
  * Find best action by building all possible paths up to depth max_depth
  * and back propagate using either max or avg
  */
  move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, 
  int* nodes_generated, int* nodes_expanded, uint32_t score){
    /*Default starting algorithm, randomly chooses between 4 movements*/
    move_t best_action = rand() % 4; 
    /*If max_depth is 0, we cannot explore further. We just return 
    a random direction*/
    if(max_depth == 0){
      return best_action; 
    }
    //Decision score of each movement from current state of the board. 
    double leftScore = 0, rightScore = 0, upScore = 0, downScore = 0;
    int i = 0, j= 0; //Use these i and j for any loops. 
    node_t* start = create_node(); //Our starting node containing the first board. 
    //Set board in this node to be current board.
    for(i = 0; i<SIZE; i++){
      for( j =0 ; j<SIZE; j++){
        start->board[i][j] = board[i][j];
      }
    }
    node_t* node = start; 
    int explore_size = INITIAL_EXPLORE_SIZE; 
    node_t** explored; 
    explored = malloc(explore_size*sizeof(node_t*));
    //checks if malloc was successful, to prevent memory disasters. 
    if(!explored) {
      printf("Error allocating memory...\n");
      exit(-1);
    }
    int explored_index = 0; //Use this to access explored array. 
    
    /*assign PQ containing node only to frontier.
    Our PQ is being implemented using heap*/
    heap_push(&h, node); //PQ contains starting node only, for now. 
   
    while(h.count >0){
      node = heap_delete(&h); //This is the popped node 
      *nodes_expanded+=1; 
      explored[explored_index] = node; 
      explored_index++; //node will be added to next index during next iteration.
      //if too big, realloc then check if allocation was successful.
      if(explored_index==explore_size){
        explore_size *= RESIZE; 
        explored = realloc(explored,  explore_size*sizeof(node_t*) );
        //checks if malloc was successful, to prevent memory disasters. 
        if(!explored) {
          printf("Error allocating memory...\n");
          exit(-1);
        }
      }
      if(node->depth < max_depth){
        //For each direction
        for(i = 0; i<4; i++){
          
          /*Following chunk of code is applyAction(node) in assignment
          specification.*/ 
          node_t* newNode = malloc(sizeof(node_t));
          
          //asserting that malloc went well!
          if(!newNode) {
            printf("Error allocating memory...\n");
            exit(-1);
          }          
          
          //Some default values for newNode. 
          *newNode = *node;
          *nodes_generated += 1; 
          node->num_childs += 1;
          newNode->num_childs = 0; 
          int oldDepth = node->depth; 
          newNode->depth = oldDepth+1; 
          newNode -> parent = node; 
          newNode->highestFromChild = 0; 
          newNode->move = i;
          newNode->averageFromChild = 0; 
          bool success = execute_move_t(newNode->board, &newNode->priority, i);
          newNode->propScore = newNode->priority;        
          
          if(success){//if change of board occured...
            heap_push(&h, newNode); //...push node into our PQ...
            //..then propagate our score back ...
            propagateBackScoreToFirstAction(newNode, &upScore, &downScore, 
              &leftScore, &rightScore, propagation); 
          }else{
            //If move not possible, delete the node. 
            newNode->parent->num_childs -= 1; 
            free(newNode); 
          }
        }
      }
    }
    
    /*Frees all memory of explored array, if not we quickly run out of space. 
    To anyone reading this, remember to take time off and free your mind as well :D*/
    free(explored);  
    
    //Following code decides which is the best action to take. 
    int scoreChosen = chooseMaxPriority(upScore, downScore, leftScore, rightScore);
    if(scoreChosen == upScore){
      best_action = UP;
    }else if (scoreChosen == downScore){
      best_action = DOWN; 
    }else if (scoreChosen == leftScore){
      best_action = LEFT; 
    }else{
      best_action = RIGHT; 
    }
   
    //best action is then returned to 2048.c
    return best_action; 
  }

  node_t *create_node(){
    /*This line creates a pointer to that node*/
    node_t* node = malloc(sizeof(node_t));
    /*assert had issues working on dimefox, while I was coding
    so I decided to check for malloc failure in this way. Adapted
    from code provided in priority_queue.c*/
    if(!node) {
      printf("Error allocating memory...\n");
      exit(-1);
    }
    //Some default values of auxilary information in node
    node->priority = 1; 
    node->depth = 0; 
    node->num_childs = 0; 
    node->move = none; 
    node->propScore = 0; 
    node->highestFromChild = 0; 
    node->averageFromChild = 0; 
    /*Good practice to set all pointers in node to null*/
    node->parent = NULL; 
   
    return node; 
  }
  void propagateBackScoreToFirstAction(node_t *newNode, double* upScore,
   double* downScore, double* leftScore, double* rightScore, propagation_t propagation){
    node_t* propFrom = newNode; 
    node_t* propTo = newNode->parent; 
    int current_depth = propFrom->depth; 
    
    while(current_depth>1){
      if(propagation == max){//if we are propagating using maximize way. 
        if(propTo->highestFromChild < propFrom->propScore){
          //Subtract the old highest child value that was being used to get the propScore.
          propTo->propScore -= propTo->highestFromChild; 
          propTo->highestFromChild = propFrom->propScore;
          propTo->propScore += propTo->highestFromChild; 
        }
      if(propagation == avg){//If we are propagating using the average way. 
         propTo->propScore -= propTo->averageFromChild; //Subtract the old average score from children. 
         //(old average*(children - 1) + propScore from this child) / children IS new average. 
         propTo->averageFromChild = ((propTo->averageFromChild)*(propTo->num_childs +1)
          + propFrom->propScore)/propTo->num_childs; 
         propTo->propScore += propTo->averageFromChild; 
        }
      }
      propFrom = propTo; 
      propTo = propTo->parent; 
      current_depth--;
    }
    
    /*Following if statements decide which action is being updated
    by all the scores we just propagated, by tracing each node 
    to it's parent until we find the node point to the root. 
    We then use it's direction as the action we are adding our 
    propagated scores to*/
    if(propFrom->move == LEFT){
      *leftScore = propFrom->propScore; 
    }else if(propFrom->move == RIGHT){
      *rightScore = propFrom->propScore; 
    }else if(propFrom->move == UP){
      *upScore = propFrom->propScore; 
    }else if(propFrom->move == DOWN){
      *downScore = propFrom->propScore; 
    }
  }

  /**chooses between max of 4 numbers, ties are broken up 
  by the int roller which has a 1/2 chance of being either 
  1 or -1 for the maximum function and is used as a coin flip
  to decide between ties
  */
  double chooseMaxPriority(double upScore,double downScore,
    double leftScore,double rightScore){
    int roller = -1;
    double firstMax = maximum(upScore, downScore, &roller);
    double secondMax = maximum(leftScore, rightScore, &roller);
    double finalMax = maximum(firstMax, secondMax, &roller);  
    return finalMax; 
  }

  double maximum(double x1, double x2,int *roller){
    if(x2 == x1){
      if (*roller == 1){
        *roller*=-1; 
        return x1; 
      }else{
        *roller*=-1; 
        return x2; 
      }
    }else if (x2>x1){
      *roller*=-1; 
      return x2;
    }else {
      *roller*=-1; 
      return x1;
    }
  }


