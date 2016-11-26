//
//  g_trigger.h
//

#ifndef G_TRIGGER_H
#define G_TRIGGER_H

void SP_trigger_multiple (edict_t *ent);
void SP_trigger_once(edict_t *ent);
void SP_trigger_relay (edict_t *self);
void SP_trigger_key (edict_t *self);
void SP_trigger_counter (edict_t *self);
void SP_trigger_always (edict_t *ent);
void SP_trigger_push (edict_t *self);
void SP_trigger_hurt (edict_t *self);
void SP_trigger_gravity (edict_t *self);
void SP_trigger_monsterjump (edict_t *self);

#endif /* G_TRIGGER_H */
