#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slist.h"
#include "pattern-matching.h"

//private methods 
void destroy_state(pm_state_t* state);
pm_state_t*  create_new_state();

/* Initializes the fsm parameters (sthe fsm itself sould be allocated).  Returns 0 on success, -1 on failure. 
*  this function should init zero state
*/
int pm_init(pm_t* fsm)
{
	if(fsm == NULL)
		return -1;
	fsm->zerostate = (pm_state_t*)malloc(sizeof(pm_state_t)+1);//init the zero state
	if(fsm->zerostate == NULL)
		return -1;
	fsm->zerostate->id = 0;
	fsm->zerostate->depth = 0;
	fsm->newstate = 1;
	
	fsm->zerostate->output = (slist_t*)malloc(sizeof(slist_t)+1);//init the output list of zero state
	if(fsm->zerostate->output == NULL)
		return -1;
	slist_init(fsm->zerostate->output);
	
	fsm->zerostate->fail = NULL;
	
	fsm->zerostate->_transitions = (slist_t*)malloc(sizeof(slist_t)+1);//init the transitions list of zero state
	if(fsm->zerostate->_transitions == NULL)
		return -1;
	slist_init(fsm->zerostate->_transitions);

	return 0;//Succeeded to init
}

/* Adds a new string to the fsm, given that the string is of length n. 
   Returns 0 on success, -1 on failure.*/
int pm_addstring(pm_t* fsm, unsigned char* pattern, size_t n)
{
	//check extremes cases
	if(fsm == NULL || fsm->zerostate == NULL || fsm->zerostate->output == NULL || fsm->zerostate->_transitions == NULL || pattern == NULL || strcmp((char*)pattern,"")==0)
		return -1;
	int i, j = 0, current_depth = 1;
	pm_state_t* cuurent_state = fsm->zerostate;
	while(pm_goto_get(cuurent_state, pattern[j]) != NULL)//there is an edge for that symbol in the FSM   
	{
		cuurent_state = pm_goto_get(cuurent_state, pattern[j]);
		current_depth++;
		j++;
	}
	// no state that fitting to this pattern, therfore need to create a new state
	for(i = j; i < n; i++)
	{
		pm_state_t* new_state = create_new_state();
		
		new_state->id = fsm->newstate;
		new_state->depth = current_depth;
		printf("Allocating state %d\n", new_state->id); 
		
		if( pm_goto_set(cuurent_state, pattern[i], new_state)==-1 )//create the edge
			return -1;
		printf("%d -> %c -> %d\n",cuurent_state->id, pattern[i], new_state->id);
		cuurent_state = new_state;//advance the state
		current_depth++;
		fsm->newstate++;		
	}

	if( slist_append(cuurent_state->output, pattern)==-1 )//try_to_append 
		return -1;
	return 0;	
}

/* Finalizes construction by setting up the failrue transitions, as
   well as the goto transitions of the zerostate. 
   Returns 0 on success, -1 on failure.*/
int pm_makeFSM(pm_t* fsm)
{
	//check extremes cases
	if(fsm == NULL || fsm->zerostate == NULL || fsm->zerostate->output == NULL || fsm->zerostate->_transitions == NULL)
		return -1;
	if(fsm->zerostate->_transitions->size == 0)//Making sure the main call to addstring
		return -1;
	
	slist_t* queue = (slist_t* )malloc(sizeof(slist_t)+1);
	if(queue == NULL)
		return -1;
	slist_init(queue);	   
	
	slist_node_t* EdgeNodeTemp = slist_head(fsm->zerostate->_transitions);//move on list of edge in the tree;//save the head's transitions list of root, in  order to run on list
	pm_labeled_edge_t* theEdge;
	while(EdgeNodeTemp != NULL) 
	{	
		theEdge = slist_data(EdgeNodeTemp);
		if( slist_append(queue, theEdge->state)==-1 )
			return -1;
		theEdge->state->fail= fsm->zerostate;// the failure function of all the states in depth of 1, automatically goto root			
		EdgeNodeTemp = EdgeNodeTemp->next;
	}
	char current_label;
	pm_state_t* next_state;
	pm_state_t* current_destination_state;
	pm_state_t* failure_parent;
	pm_state_t* sNext;//move to the 'son' state
	slist_node_t* temp;
	while(slist_size(queue) > 0)
	{
		next_state = slist_pop_first(queue);//dequeue next state in queue
		temp = slist_head(next_state->_transitions);//save the head's transitions list of the next state, in  order to run on list	
		while(temp != NULL) 
		{
			current_destination_state = ((pm_labeled_edge_t*)temp->data)->state;
			current_label = ((pm_labeled_edge_t*)temp->data)->label;	
			
			slist_append(queue, current_destination_state); // enqueue(son)
			
			failure_parent = next_state->fail;
			
			sNext = pm_goto_get(failure_parent, current_label);
			while(sNext == NULL)//there is no edge for that symbol in the FSM   
			{
				if(failure_parent != fsm->zerostate)
				{
					failure_parent = failure_parent->fail;
				}		
				sNext = pm_goto_get(failure_parent, current_label);
				if(failure_parent == fsm->zerostate && sNext == NULL )//validate when state is a zerostate
				{
					sNext = fsm->zerostate;
				}
			}
			current_destination_state->fail = sNext;	
			printf("Setting f(%d) = %d\n", current_destination_state->id, current_destination_state->fail->id); 

			if( slist_append_list(current_destination_state->output, current_destination_state->fail->output)==-1 )	//unoin the output lists
				return -1;
			temp = temp->next;// advance to next edge of next state in queue
		}
		
	}
	//now queue is empty
	slist_destroy(queue, SLIST_FREE_DATA);
	free(queue);
	queue = NULL;
	return 0;
}

/* Set a transition arrow from this from_state, via a symbol, to a
   to_state. will be used in the pm_addstring and pm_makeFSM functions.
   Returns 0 on success, -1 on failure.*/   
int pm_goto_set(pm_state_t* from_state, unsigned char symbol, pm_state_t* to_state)
{
	pm_labeled_edge_t* new_edge = (pm_labeled_edge_t*)malloc(sizeof(pm_labeled_edge_t)+1);
	if(new_edge == NULL)
		return -1;
	new_edge->label = symbol;
	new_edge->state = to_state; 

	if( slist_append(from_state->_transitions, new_edge)==-1 )//try_to_append
		return -1;
	return 0;//Succeeded to set
}

/* Returns the transition state.  If no such state exists, returns NULL. 
   will be used in pm_addstring, pm_makeFSM, pm_fsm_search, pm_destroy functions. */
pm_state_t* pm_goto_get(pm_state_t* state, unsigned char symbol)
{
	slist_node_t* temp = state->_transitions->head;//save the head of _transitions list in temp order to run on list
	while(temp != NULL) 
	{
		if( ((pm_labeled_edge_t*)temp->data)->label == symbol )
		{
			return ( ((pm_labeled_edge_t*)temp->data)->state );
		}
		temp = temp->next;
	}	
	//if does not exist edge with that symbol that go from this state.
	return NULL;
}

/* Search for matches in a string of size n in the FSM. 
   if there are no matches return empty list */
slist_t* pm_fsm_search(pm_state_t* root, unsigned char* text, size_t n)
{			
	if(root == NULL || root->_transitions->size == 0 || text == NULL)//check extremes cases
		return NULL;

	// after every call to addstring must call to makeFSM, so must check if the defined the fail of all sons of zerostate
	slist_node_t* temp = root->_transitions->head;
	while(temp != NULL) 
	{
		if( ((pm_labeled_edge_t*)temp->data)->state->fail == NULL )
			return NULL;						
		temp = temp->next;
	}
	slist_t* matches_list = (slist_t*)malloc(sizeof(slist_t)+1);
	if(matches_list == NULL)
		return NULL;
	slist_init(matches_list);
	
	pm_state_t* state = root;//init state to root
	pm_state_t* sNext;//son state
	int i;
	for(i = 0; i < n; i++)// run on text
	{
		sNext = pm_goto_get(state, text[i]);	
		while(sNext == NULL)//there is no edge for that symbol in the FSM   
		{
			if(state != root)
			{
				state = state->fail;
			}		
			sNext = pm_goto_get(state, text[i]);
			if(state == root && sNext == NULL )//validate when state is a zerostate
			{
				sNext = root;
			}	
		}
		state = sNext;
		if(slist_size(state->output) > 0)
		{						
			slist_node_t* temp = state->output->head;//save the head of output list in temp in order to run on list
			while(temp != NULL) 
			{					
				pm_match_t* match = (pm_match_t*)malloc(sizeof(pm_match_t)+1);
				if(match == NULL)
					return NULL;
				match->pattern = temp->data;				
				match->start_pos = i- strlen(match->pattern)+1;
				match->end_pos = i;
				match->fstate = state; 							
				printf("Pattern: %s, start at: %d, ends at: %d, last state = %d\n", match->pattern, match->start_pos, match->end_pos, match->fstate->id);				
				if( slist_append(matches_list, match)==-1 )
					return NULL;				
				temp = temp->next;				
			}								
		}
	}
	return matches_list;	
}

/* Destroys the fsm, deallocating memory. */
void pm_destroy(pm_t* fsm)
{	
	if(fsm == NULL || fsm->zerostate == NULL || fsm->zerostate->output == NULL || fsm->zerostate->_transitions == NULL)
		return;
	slist_t* queue = (slist_t*)malloc(sizeof(slist_t)+1);//this queue will hold all the states 
	if(queue == NULL)
		return;
	slist_init(queue);	   

	pm_state_t* current_destination_state = fsm->zerostate;
	slist_append(queue, current_destination_state); // enqueue(root)
	slist_node_t* temp;// hold the head of transitions list of next state from the Queue
	while(slist_size(queue) > 0)
	{
		pm_state_t* next_state = slist_pop_first(queue);//dequeue next state in queue
		temp = next_state->_transitions->head;//save the head's transitions list of the next state, in  order to run on list	
		while(temp != NULL) 
		{
			current_destination_state = ((pm_labeled_edge_t*)temp->data)->state;
			slist_append(queue, current_destination_state); // enqueue(son)
			temp = temp->next;// advance to next edge of next state in queue 			
		}		
		destroy_state(next_state);
	}
	//now queue is empty
	slist_destroy(queue, SLIST_FREE_DATA);
	free(queue);
	queue = NULL; 	 
}
void destroy_state(pm_state_t* state)
{
	slist_destroy(state->output, SLIST_LEAVE_DATA);//free the output list of current state
	free(state->output);
	state->output = NULL;
	
	slist_destroy(state->_transitions, SLIST_FREE_DATA);//free the transitions list of current state
	free(state->_transitions);
	state->_transitions = NULL;
	
	free(state);
	state = NULL;
}

/* Create new state, allocate lists. */
pm_state_t*  create_new_state()
{
	pm_state_t* new_state;
	new_state = (pm_state_t*)malloc(sizeof(pm_state_t)+1);// init the state
	if(new_state == NULL)
		return NULL;
 
	new_state->output = (slist_t*)malloc(sizeof(slist_t)+1);//init the output list for each new state
	if(new_state->output == NULL)
		return NULL;
	slist_init(new_state->output);

	new_state->_transitions = (slist_t*)malloc(sizeof(slist_t)+1);//init the transitions list for each new state
	if(new_state->_transitions == NULL)
		return NULL;
	slist_init(new_state->_transitions);

	return 	new_state;
}
