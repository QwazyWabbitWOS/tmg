//
//  g_target.h
//

#ifndef G_TARGET_H
#define G_TARGET_H

extern void SP_target_speaker (edict_t *ent);

extern void SP_target_temp_entity (edict_t *ent);

extern void SP_target_help(edict_t *ent);
extern void SP_target_secret (edict_t *ent);
extern void SP_target_goal (edict_t *ent);
extern void SP_target_explosion (edict_t *ent);
extern void SP_target_changelevel (edict_t *ent);
extern void SP_target_splash (edict_t *self);
extern void SP_target_spawner (edict_t *self);
extern void SP_target_blaster (edict_t *self);
extern void SP_target_crosslevel_trigger (edict_t *self);
extern void SP_target_crosslevel_target (edict_t *self);
extern void SP_target_laser (edict_t *self);
extern void SP_target_lightramp (edict_t *self);
extern void SP_target_earthquake (edict_t *self);

#endif /* G_TARGET_H */
