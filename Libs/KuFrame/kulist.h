#ifndef __Ku_List__
#define __Ku_List__

typedef struct __KuListItem KuListItem;

struct __KuListItem
{
	KuListItem *next;
	KuListItem *prev;
	void *data;
};
typedef struct
{
	KuListItem *cur;
	int count;
} KuList;

extern void list_init(KuList *list);
extern KuListItem *list_first(KuList *list);
extern KuListItem *list_last(KuList *list);
extern KuListItem *list_at(KuList *list, int index);
extern KuListItem *list_next(KuList *list);
extern KuListItem *list_prev(KuList *list);
extern void list_add(KuList *list, KuListItem *item);
extern void list_insert(KuList *list, int index, KuListItem *item);
extern KuListItem *list_remove(KuList *list, int index);

#endif
