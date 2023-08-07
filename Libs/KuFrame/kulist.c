#include "kulist.h"

void item_addbefore(KuListItem *list, KuListItem *item);
void item_addafter(KuListItem *list, KuListItem *item);
void item_remove(KuListItem *item);

void list_init(KuList *list)
{
	list->cur = 0;
	list->count = 0;
}

KuListItem *list_first(KuList *list)
{
	KuListItem *result = list->cur;
	if (!result)
		return 0;
	while (result->prev)
		result = result->prev;
	list->cur = result;
	return result;
}
KuListItem *list_last(KuList *list)
{
	KuListItem *result = list->cur;
	if (!result)
		return 0;
	while (result->next)
		result = result->next;
	list->cur = result;
	return result;
}
KuListItem *list_at(KuList *list, int index)
{
	KuListItem *result = list_first(list);
	if (!result)
		return 0;
	while (index-- > 0)
	{
		if (!result->next)
			return 0;
		else
			result = result->next;
	}
	list->cur = result;
	return result;
}
KuListItem *list_next(KuList *list)
{
	KuListItem *result = list->cur;
	if (!result)
		return 0;
	if (!result->next)
		return 0;
	list->cur = result->next;
	return result;
}
KuListItem *list_prev(KuList *list)
{
	KuListItem *result = list->cur;
	if (!result)
		return 0;
	if (!result->prev)
		return 0;
	list->cur = result->prev;
	return result;
}

void list_insert(KuList *list, int index, KuListItem *item)
{
	KuListItem *result = list_at(list, index);
	if (result != 0)
		item_addbefore(result, item);
	else
		list_add(list, item);
	list->count++;
}
void list_add(KuList *list, KuListItem *item)
{
	KuListItem *result = list_last(list);
	if (result != 0)
	{
		item_addafter(result, item);
		list->count++;
	}
	else
	{
		item->prev = 0;
		item->next = 0;
		list->cur = item;
		list->count = 1;
	}
}

KuListItem *list_remove(KuList *list, int index)
{
	KuListItem *result = list_at(list, index);
	if (result != 0)
	{
		item_remove(result);
		list->count--;
	}
	return result;
}

void item_addbefore(KuListItem *list, KuListItem *item)
{
	item->prev = list->prev;
	item->next = list;
	if (list->prev)
		list->prev->next = item;
	list->prev = item;
}
void item_addafter(KuListItem *cur, KuListItem *item)
{
	item->next = cur->next;
	item->prev = cur;
	if (cur->next)
		cur->next->prev = item;
	cur->next = item;
}
void item_remove(KuListItem *item)
{
	if (item->prev != 0)
		item->prev->next = item->next;
	if (item->next != 0)
		item->next->prev = item->prev;
}
