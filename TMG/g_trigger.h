//
//  g_trigger.h
//

#ifndef g_trigger_h
#define g_trigger_h

extern void SP_trigger_multiple (edict_t *ent);
extern void SP_trigger_once(edict_t *ent);
extern void SP_trigger_relay (edict_t *self);
extern void SP_trigger_key (edict_t *self);
extern void SP_trigger_counter (edict_t *self);
extern void SP_trigger_always (edict_t *ent);
extern void SP_trigger_push (edict_t *self);
extern void SP_trigger_hurt (edict_t *self);
extern void SP_trigger_gravity (edict_t *self);
extern void SP_trigger_monsterjump (edict_t *self);

#endif /* g_trigger_h */
