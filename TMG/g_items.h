#ifndef G_ITEMS_H
#define G_ITEMS_H


#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

#define	ITEM_INDEX(x) ((x)-itemlist)

//
// g_items.c
//
// dependency on g_local.h
extern gitem_t	*GetItemByIndex (int index);
extern gitem_t	*FindItemByClassname (char *classname);
extern gitem_t	*FindItem (char *pickup_name);

void DoRespawn (edict_t *ent);
void SetRespawn (edict_t *ent, float delay);
qboolean Pickup_Powerup (edict_t *ent, edict_t *other);
void Drop_General (edict_t *ent, gitem_t *item);
qboolean Pickup_Navi (edict_t *ent, edict_t *other);
qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other);
qboolean Pickup_AncientHead (edict_t *ent, edict_t *other);
qboolean Pickup_Bandolier (edict_t *ent, edict_t *other);
qboolean Pickup_Pack (edict_t *ent, edict_t *other);
void Use_Quad (edict_t *ent, gitem_t *item);
void Use_Breather (edict_t *ent, gitem_t *item);
void Use_Envirosuit (edict_t *ent, gitem_t *item);
void	Use_Invulnerability (edict_t *ent, gitem_t *item);
void	Use_Silencer (edict_t *ent, gitem_t *item);
qboolean Pickup_Key (edict_t *ent, edict_t *other);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
qboolean	Pickup_Ammo (edict_t *ent, edict_t *other);
void Drop_Ammo (edict_t *ent, gitem_t *item);
void MegaHealth_think (edict_t *self);
qboolean	Pickup_Health (edict_t *ent, edict_t *other);
int ArmorIndex (edict_t *ent);
qboolean	Pickup_Armor (edict_t *ent, edict_t *other);
int PowerArmorType (edict_t *ent);
void Use_PowerArmor (edict_t *ent, gitem_t *item);
qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other);
void Drop_PowerArmor (edict_t *ent, gitem_t *item);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void Use_Item (edict_t *ent, edict_t *other, edict_t *activator);

void droptofloor (edict_t *ent);
void droptofloor2 (edict_t *ent);

void PrecacheItem (gitem_t *it);

void SpawnItem (edict_t *ent, gitem_t *item);

//QW FIXME: Give these guys descriptive names
void SpawnItem2 (edict_t *ent, gitem_t *item);
void SpawnItem3 (edict_t *ent, gitem_t *item);
//QW

void SP_item_health (edict_t *self);
void SP_item_health_small (edict_t *self);
void SP_item_health_large (edict_t *self);
void SP_item_health_mega (edict_t *self);

void InitItems (void);
/**
 Called by SP_worldspawn
 */
void SetItemNames (void);

//QW// in p_weapon.c
void ChangeWeapon (edict_t *ent);
void Think_Weapon (edict_t *ent);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);
qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);

//QW used only in g_items.c
extern int	jacket_armor_index;
extern int	combat_armor_index;
extern int	body_armor_index;
extern int	power_screen_index;
extern int	power_shield_index;

#endif //G_ITEMS_H
