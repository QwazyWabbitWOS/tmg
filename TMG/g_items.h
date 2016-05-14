#ifndef G_ITEMS_H
#define G_ITEMS_H


//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);

#define	ITEM_INDEX(x) ((x)-itemlist)

edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);


void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean	Pickup_Health (edict_t *ent, edict_t *other);
qboolean	Pickup_Ammo (edict_t *ent, edict_t *other);
qboolean	Pickup_Armor (edict_t *ent, edict_t *other);


//extern int	jacket_armor_index;
//extern int	combat_armor_index;
//extern int	body_armor_index;
//extern int	power_screen_index;
//extern int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

#endif //G_ITEMS_H
