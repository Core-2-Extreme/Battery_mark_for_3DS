#include "system/headers.hpp"

LightLock util_queue_mutex = 1;//Initially unlocked state.

Result_with_string Util_queue_create(Queue* queue, int max_items)
{
	Result_with_string result;

	if(!queue || max_items == 0)
		goto invalid_arg;

	LightLock_Lock(&util_queue_mutex);

	if(queue->data || queue->event_id)
		goto already_inited;

	memset(queue, 0x0, sizeof(Queue));

	queue->data = (u32*)malloc(max_items * sizeof(u32));
	queue->event_id = (u32*)malloc(max_items * sizeof(u32));
	if(!queue->data || !queue->event_id)
		goto out_of_memory;

	memset(queue->event_id, 0x0, (max_items * sizeof(u32)));
	memset(queue->data, 0x0, (max_items * sizeof(u32)));
	queue->max_items = max_items;
	queue->next_index = 0;

	LightEvent_Init(&queue->receive_wait_event, RESET_ONESHOT);
	LightEvent_Init(&queue->send_wait_event, RESET_ONESHOT);

	LightLock_Unlock(&util_queue_mutex);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	already_inited:
	LightLock_Unlock(&util_queue_mutex);
	result.code = DEF_ERR_ALREADY_INITIALIZED;
	result.string = DEF_ERR_ALREADY_INITIALIZED_STR;
	return result;

	out_of_memory:
	free(queue->data);
	free(queue->event_id);
	queue->data = NULL;
	queue->event_id = NULL;
	LightLock_Unlock(&util_queue_mutex);
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;
}

Result_with_string Util_queue_add(Queue* queue, u32 event_id, void* data, s64 wait_us, Queue_option option)
{
	Result_with_string result;

	if(!queue)
		goto invalid_arg;

	LightLock_Lock(&util_queue_mutex);

	if(!queue->data || !queue->event_id)
		goto not_inited;

	if(queue->next_index >= queue->max_items && wait_us > 0)
	{
		//No spaces are available, wait for a space.
		LightLock_Unlock(&util_queue_mutex);
		LightEvent_WaitTimeout(&queue->send_wait_event, wait_us * 1000);
		LightLock_Lock(&util_queue_mutex);
	}

	if(queue->next_index >= queue->max_items)
	{
		//Queue is full.
		goto out_of_memory;
	}
	else
	{
		//Queue is not full.
		int index = 0;

		if(option & QUEUE_OPTION_DO_NOT_ADD_IF_EXIST)
		{
			//Don't add the event if the same event exist.
			for(int i = 0; i < queue->next_index; i++)
			{
				if(queue->event_id[i] == event_id)
					goto already_exist;
			}
		}

		if(option & QUEUE_OPTION_SEND_TO_FRONT)
		{
			//Move other data to back.
			for(int i = (queue->next_index - 1); i > -1; i--)
			{
				queue->data[i + 1] = queue->data[i];
				queue->event_id[i + 1] = queue->event_id[i];
			}

			index = 0;
		}
		else
			index = queue->next_index;

		queue->data[index] = (u32)data;
		queue->event_id[index] = event_id;
		queue->next_index++;

		LightEvent_Clear(&queue->send_wait_event);
		LightEvent_Signal(&queue->receive_wait_event);
	}

	LightLock_Unlock(&util_queue_mutex);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	not_inited:
	LightLock_Unlock(&util_queue_mutex);
	result.code = DEF_ERR_NOT_INITIALIZED;
	result.string = DEF_ERR_NOT_INITIALIZED_STR;
	return result;

	out_of_memory:
	LightLock_Unlock(&util_queue_mutex);
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	already_exist://Treat it as success.
	LightLock_Unlock(&util_queue_mutex);
	return result;
}

Result_with_string Util_queue_get(Queue* queue, u32* event_id, void** data, s64 wait_us)
{
	Result_with_string result;

	if(!queue || !event_id)
		goto invalid_arg;

	LightLock_Lock(&util_queue_mutex);

	if(!queue->data || !queue->event_id)
		goto not_inited;

	if(queue->next_index <= 0 && wait_us > 0)
	{
		//No messages are available, wait for a message.
		LightLock_Unlock(&util_queue_mutex);
		LightEvent_WaitTimeout(&queue->receive_wait_event, wait_us * 1000);
		LightLock_Lock(&util_queue_mutex);
	}

	if(queue->next_index <= 0)
	{
		//Queue is empty.
		goto try_again;
	}
	else
	{
		//Queue is not empty.
		*event_id = queue->event_id[0];
		if(queue->data[0])
		{
			if(data)
				*data = (void*)queue->data[0];
			else
				free((void*)queue->data[0]);
		}

		//Delete old data as it no longer necessary.
		queue->data[0] = 0;
		queue->event_id[0] = 0;

		//Move rest of the data to front.
		for(int i = 0; i < (queue->next_index - 1); i++)
		{
			queue->data[i] = queue->data[i + 1];
			queue->event_id[i] = queue->event_id[i + 1];
		}

		//Delete the last data.
		queue->data[queue->next_index - 1] = 0;
		queue->event_id[queue->next_index - 1] = 0;

		queue->next_index--;

		LightEvent_Clear(&queue->receive_wait_event);
		LightEvent_Signal(&queue->send_wait_event);
	}

	LightLock_Unlock(&util_queue_mutex);

	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	not_inited:
	LightLock_Unlock(&util_queue_mutex);
	result.code = DEF_ERR_NOT_INITIALIZED;
	result.string = DEF_ERR_NOT_INITIALIZED_STR;
	return result;

	try_again:
	LightLock_Unlock(&util_queue_mutex);
	result.code = DEF_ERR_TRY_AGAIN;
	result.string = DEF_ERR_TRY_AGAIN_STR;
	return result;
}

bool Util_queue_is_event_exist(Queue* queue, u32 event_id)
{
	bool exist = false;

	if(!queue || !queue->data || !queue->event_id)
		return false;

	LightLock_Lock(&util_queue_mutex);

	for(int i = 0; i < queue->next_index; i++)
	{
		if(queue->event_id[i] == event_id)
		{
			exist = true;
			break;
		}
	}

	LightLock_Unlock(&util_queue_mutex);

	return exist;
}

int Util_queue_get_free_space(Queue* queue)
{
	int free = 0;

	if(!queue || !queue->data || !queue->event_id)
		return 0;

	LightLock_Lock(&util_queue_mutex);

	free = queue->max_items - queue->next_index;

	LightLock_Unlock(&util_queue_mutex);

	return free;
}

void Util_queue_delete(Queue* queue)
{
	if(!queue || !queue->data || !queue->event_id)
		return;

	LightLock_Lock(&util_queue_mutex);

	for(int i = 0; i < queue->max_items; i++)
	{
		free((void*)queue->data[i]);
		queue->data[i] = 0;
		queue->event_id[i] = 0;
	}

	free(queue->data);
	free(queue->event_id);
	queue->data = NULL;
	queue->event_id = NULL;
	memset(queue, 0x0, sizeof(Queue));

	LightLock_Unlock(&util_queue_mutex);
}
