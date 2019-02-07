#include "slist.h"
#include <stdlib.h>

/** Initialize a single linked list
	\param list - the list to initialize */
void slist_init(slist_t* list)
{
	if(list == NULL)
		return;
	slist_size(list) = 0;
	slist_head(list) = NULL;	
	slist_tail(list) = NULL;
}

/** Destroy and de-allocate the memory hold by a list
	\param list - a pointer to an existing list
	\param dealloc flag that indicates whether stored data should also be de-allocated */
void slist_destroy(slist_t* list, slist_destroy_t flag)
{
	if(list == NULL)
		return;
	slist_size(list) = 0;
	slist_node_t *current;
	while(slist_head(list) != NULL) 
	{
		current = slist_head(list);
		slist_head(list) = current->next;
		if(flag == SLIST_FREE_DATA)
		{
			free(current->data);
		}
		free(current);
		current = NULL;
  	}
}

/** Pop the first element in the list
	\param list - a pointer to a list
	\return a pointer to the data of the element, or NULL if the list is empty */
void* slist_pop_first(slist_t* list)
{
	if(list == NULL)
		return NULL;
	if(slist_size(list) == 0)// the list is empty 
	{	
		return NULL;
	}
	else                     // the list is not empty           
	{
		slist_node_t* temp_head; 				
		temp_head = slist_head(list);//save the head
		slist_head(list) = slist_next(slist_head(list));// pop the first element
		slist_size(list)--;
		void* data_of_first_element = temp_head->data;//save the data
		free(temp_head);
		return (data_of_first_element); 		
	}
}

/* Append data to list (add as last node of the list)
	\param list - a pointer to a list
	\param data - the data to place in the list
	\return 0 on success, or -1 on failure */
int slist_append(slist_t* list, void* new_data)
{
	if(list == NULL)
		return -1;
	slist_node_t* new_node;
    	new_node = (slist_node_t*) malloc(sizeof (slist_node_t));
	if(new_node == NULL)
		return -1;
	else
	{
		new_node->data = new_data;    
		new_node->next = NULL;
		if(list->size == 0)// the list is empty 
		{
			slist_head(list) = slist_tail(list) = new_node;	
			slist_size(list)++;
			return 0;
		}                                                          
		slist_next(slist_tail(list)) = new_node;
		slist_tail(list) = new_node;
		slist_size(list)++;
		return 0;	
	}
}

/** Prepend data to list (add as first node of the list)
	\param list - a pointer to list
	\param data - the data to place in the list
	\return 0 on success, or -1 on failure
*/
int slist_prepend(slist_t* list, void* new_data)
{
	if(list == NULL)
		return -1;
	slist_node_t* new_node;
    	new_node = (slist_node_t*) malloc(sizeof (slist_node_t));
	if(new_node == NULL)
		return -1;
	else
	{
		new_node->data = new_data;
		if(slist_size(list) == 0)// the list is empty 
		{
			slist_head(list) = slist_tail(list) = new_node; 
                        slist_next(slist_tail(list)) = NULL;    
			slist_size(list)++;
			return 0;
		}
		new_node->next = list->head;
		slist_head(list) = new_node;
		slist_size(list)++;
		return 0;	
	}
}

/** \brief Append elements from the second list to the first list, use the slist_append function.
	you can assume that the data of the lists were not allocated and thus should not be deallocated in destroy 
	(the destroy for these lists will use the SLIST_LEAVE_DATA flag)
	\param to a pointer to the destination list
	\param from a pointer to the source list
	\return 0 on success, or -1 on failure
*/
int slist_append_list(slist_t* firstList, slist_t* secoundList)
{
	if(firstList == NULL || secoundList == NULL)
	{
		return -1;
	}
	if( (slist_size(firstList) != 0 || slist_size(secoundList) != 0) )// just have 2 cases that need append element from secound to first
	{
		slist_node_t* temp = slist_head(secoundList);  //save the head of secound list in temp order to run on list	
		while(temp != NULL) 
		{
			if( slist_append(firstList, temp->data) == -1 )
			{
				return -1;
			}
			temp = temp->next;
	  	}	
		return 0;
	}
	return 0;		
}

