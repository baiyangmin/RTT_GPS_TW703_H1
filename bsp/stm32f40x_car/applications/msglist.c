/*
消息格式，包括收发的统一
 */
#include <stdlib.h>
#include <rtthread.h>
#include "msglist.h"



MsgListNode* msglist_node_create(void* data)
{
	MsgListNode* node = rt_malloc(sizeof(MsgListNode));

	if(node != RT_NULL)
	{
		node->prev = RT_NULL;
		node->next = RT_NULL;
		node->data = data;
	}

	return node;
}

void msglist_node_destroy(MsgListNode* node)
{
	if(node != RT_NULL)
	{
		node->next = RT_NULL;
		node->prev = RT_NULL;
		rt_free(node);
	}

	return;
}

MsgList* msglist_create(void)
{
	MsgList* thiz = rt_malloc(sizeof(MsgList));

	if(thiz != RT_NULL)
	{
		thiz->first = RT_NULL;
	}

	return thiz;
}

MsgListRet msglist_prepend(MsgList* thiz, void* data)
{
	MsgListNode* node = RT_NULL;
	MsgListNode* iter=RT_NULL;

	if((node = msglist_node_create(data)) == RT_NULL)
	{
		return MSGLIST_RET_OOM; 
	}

	if(thiz->first == RT_NULL)
	{
		thiz->first = node;
		return MSGLIST_RET_OK;
	}
	
	iter= thiz->first;
	while(iter->prev != RT_NULL)
	{
		iter = iter->prev;
	}
	iter->prev=node;
	node->next=iter;
	thiz->first=node;
	return MSGLIST_RET_OK;

}


MsgListRet msglist_append(MsgList* thiz, void* data)
{
	MsgListNode* node = RT_NULL;
	MsgListNode* iter=RT_NULL;

	if((node = msglist_node_create(data)) == RT_NULL)
	{
		return MSGLIST_RET_OOM; 
	}

	if(thiz->first == RT_NULL)
	{
		thiz->first = node;
		return MSGLIST_RET_OK;
	}
	
	iter= thiz->first;
	while(iter->next != RT_NULL)
	{
		iter = iter->next;
	}
	iter->next=node;
	node->prev=iter;
	return MSGLIST_RET_OK;
}





size_t   msglist_length(MsgList* thiz)
{
	size_t length = 0;
	MsgListNode* iter = thiz->first;

	while(iter != RT_NULL)
	{
		length++;
		iter = iter->next;
	}

	return length;
}

MsgListRet msglist_foreach(MsgList* thiz, MsgListDataVisitFunc visit, void* ctx)
{
	MsgListRet ret = MSGLIST_RET_OK;
	MsgListNode* iter = thiz->first;

	while(iter != RT_NULL && ret != MSGLIST_RET_STOP)
	{
		ret = visit(ctx, iter);
		iter = iter->next;
	}

	return ret;
}

int      msglist_find(MsgList* thiz, MsgListDataCompareFunc cmp, void* ctx)
{
	int i = 0;
	MsgListNode* iter = thiz->first;

	while(iter != RT_NULL)
	{
		if(cmp(ctx, iter) == 0)
		{
			break;
		}
		i++;
		iter = iter->next;
	}

	return i;
}

void msglist_destroy(MsgList* thiz)
{
	MsgListNode* iter = thiz->first;
	MsgListNode* next = RT_NULL;

	while(iter != RT_NULL)
	{
		next = iter->next;
		msglist_node_destroy(iter);
		iter = next;
	}

	thiz->first = RT_NULL;
	rt_free(thiz);

	return;
}


