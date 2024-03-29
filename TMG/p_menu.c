#include "g_local.h"

//#define DEBUG_NT 1
//#define DEBUG_UNIX 2

#ifdef DEBUG_NT
#include <crtdbg.h>
#endif

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, qboolean usekeys, qboolean showbackground)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;
	int i;

	if (!ent->client)
		return;

	if (ent->client->menu)
	{
		gi.dprintf("warning, ent already has a menu\n");
		PMenu_Close(ent);
	}

	hnd = gi.TagMalloc(sizeof(*hnd), TAG_LEVEL);

	hnd->entries = entries;
	hnd->num = num;
	hnd->MenuTimeout = 0;	//Default to no timeout
	hnd->UseNumberKeys = usekeys;	//Can select items with number keys?
	hnd->ShowBackground = showbackground;

	if (cur < 0 || !entries[cur].SelectFunc)
	{
		for (i = 0, p = entries; i < num; i++, p++)
			if (p->SelectFunc)
				break;
	}
	else
		i = cur;

	if (i >= num)
		hnd->cur = -1;
	else
		hnd->cur = i;

	ent->client->showscores = true;
	ent->client->inmenu = true;
	ent->client->menu = hnd;

	PMenu_Update(ent);
	gi.unicast (ent, true);
}

void PMenu_Close(edict_t *ent)
{
	if (!ent->client->menu)
		return;

	gi.TagFree(ent->client->menu);
	ent->client->menu = NULL;
	ent->client->showscores = false;
}

void PMenu_Update(edict_t *ent)
{
	char string[1400];
	int i;
	pmenu_t *p;
	int x;
	pmenuhnd_t *hnd;
	char *t;
	qboolean alt = false;

	if (!ent->client->menu)
	{
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;
	
	if (hnd->ShowBackground)
		strcpy(string, "xv 32 yv 8 picn inventory ");
	else
		strcpy(string, "");

	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++) 
	{
		if (!p->text || !*(p->text))
			continue; // blank line
		t = p->text;
		if (*t == '*')
		{
			alt = true;
			t++;
		}

		//WF - made inventory screen larger
		sprintf(string + strlen(string),"yv %d ", 32 + i * 8);
		if (p->align == PMENU_ALIGN_CENTER)
			x = 196/2 - (int) strlen(t)*4 + 64;
//			x = 240/2 - strlen(t)*4 + 64;
		else if (p->align == PMENU_ALIGN_RIGHT)
//			x = 64 + (196 - strlen(t)*8);
			x = 64 + (240 - (int) strlen(t)*8);
		else
			x = 64;

		sprintf(string + strlen(string), "xv %d ",
			x - ((hnd->cur == i) ? 8 : 0));

		if (hnd->cur == i)
			sprintf(string + strlen(string), "string2 \"\x0d%s\" ", t);
		else if (alt)
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		else
			sprintf(string + strlen(string), "string \"%s\" ", t);
		alt = false;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void PMenu_Next(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu)
	{
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		i++; p++;
		if (i == hnd->num)
		{
			i = 0;
			p = hnd->entries;
		}
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
	gi.unicast(ent, true);
}

void PMenu_Prev(edict_t *ent)
{
	pmenuhnd_t *hnd;
	int i;
	pmenu_t *p;

	if (!ent->client->menu)
	{
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do {
		if (i == 0) {
			i = hnd->num - 1;
			p = hnd->entries + i;
		}
		else
		{
			i--;
			p--;
		}
		if (p->SelectFunc)
			break;
	} while (i != hnd->cur);

	hnd->cur = i;

	PMenu_Update(ent);
	gi.unicast (ent, true);
}

void PMenu_Select(edict_t *ent)
{
	pmenuhnd_t *hnd;
	pmenu_t *p;

	if (!ent->client->menu)
	{
		gi.dprintf("warning:  ent has no menu\n");
		return;
	}

	hnd = ent->client->menu;

	if (hnd->cur < 0)
		return; // no selectable entries

	p = hnd->entries + hnd->cur;

	if (p->SelectFunc)
		p->SelectFunc(ent, p);
}
