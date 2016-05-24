//
//  g_func.h
//  tmg
//

#ifndef g_func_h
#define g_func_h

extern void SP_func_rotating (edict_t *ent);
extern void SP_func_button (edict_t *ent);
extern void SP_func_door (edict_t *ent);
extern void SP_func_door_rotating (edict_t *ent);
extern void SP_func_water (edict_t *self);
extern void func_train_find (edict_t *self);
extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void SP_func_train (edict_t *self);
extern void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator);
extern void SP_trigger_elevator (edict_t *self);
extern void SP_func_timer (edict_t *self);
extern void SP_func_conveyor (edict_t *self);
extern void SP_func_door_secret (edict_t *ent);
extern void SP_func_killbox (edict_t *ent);

#endif /* g_func_h */
