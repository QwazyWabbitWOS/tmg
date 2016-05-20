#ifndef G_ITEMS_H
#define G_ITEMS_H


#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

#define	ITEM_INDEX(x) ((x)-itemlist)

//
// g_items.c
//
extern gitem_t	*GetItemByIndex (int index);
extern gitem_t	*FindItemByClassname (char *classname);
extern gitem_t	*FindItem (char *pickup_name);
extern void DoRespawn (edict_t *ent);
extern void SetRespawn (edict_t *ent, float delay);
extern qboolean Pickup_Powerup (edict_t *ent, edict_t *other);
extern void Drop_General (edict_t *ent, gitem_t *item);
extern qboolean Pickup_Navi (edict_t *ent, edict_t *other);
extern qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other);
extern qboolean Pickup_AncientHead (edict_t *ent, edict_t *other);
extern qboolean Pickup_Bandolier (edict_t *ent, edict_t *other);
extern qboolean Pickup_Pack (edict_t *ent, edict_t *other);
extern void Use_Quad (edict_t *ent, gitem_t *item);
extern void Use_Breather (edict_t *ent, gitem_t *item);
extern void Use_Envirosuit (edict_t *ent, gitem_t *item);
extern void	Use_Invulnerability (edict_t *ent, gitem_t *item);
extern void	Use_Silencer (edict_t *ent, gitem_t *item);
extern qboolean Pickup_Key (edict_t *ent, edict_t *other);
extern qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
extern qboolean	Pickup_Ammo (edict_t *ent, edict_t *other);
extern void Drop_Ammo (edict_t *ent, gitem_t *item);
extern void MegaHealth_think (edict_t *self);
extern qboolean	Pickup_Health (edict_t *ent, edict_t *other);
extern int ArmorIndex (edict_t *ent);
extern qboolean	Pickup_Armor (edict_t *ent, edict_t *other);
extern int PowerArmorType (edict_t *ent);
extern void Use_PowerArmor (edict_t *ent, gitem_t *item);
extern qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other);
extern void Drop_PowerArmor (edict_t *ent, gitem_t *item);
extern void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
extern edict_t *Drop_Item (edict_t *ent, gitem_t *item);
extern void Use_Item (edict_t *ent, edict_t *other, edict_t *activator);

extern void droptofloor (edict_t *ent);
extern void droptofloor2 (edict_t *ent);

extern void PrecacheItem (gitem_t *it);

extern void SpawnItem (edict_t *ent, gitem_t *item);

//QW
extern void SpawnItem2 (edict_t *ent, gitem_t *item);
extern void SpawnItem3 (edict_t *ent, gitem_t *item);
//QW

extern void SP_item_health (edict_t *self);
extern void SP_item_health_small (edict_t *self);
extern void SP_item_health_large (edict_t *self);
extern void SP_item_health_mega (edict_t *self);

extern void InitItems (void);
/**
 Called by SP_worldspawn
 */
extern void SetItemNames (void);

//QW// in p_weapon.c
extern void ChangeWeapon (edict_t *ent);
extern void Think_Weapon (edict_t *ent);
extern void		Use_Weapon (edict_t *ent, gitem_t *inv);
extern void		Drop_Weapon (edict_t *ent, gitem_t *inv);
extern qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);

//QW used only in g_items.c
extern int	jacket_armor_index;
extern int	combat_armor_index;
extern int	body_armor_index;
extern int	power_screen_index;
extern int	power_shield_index;

#endif //G_ITEMS_H
