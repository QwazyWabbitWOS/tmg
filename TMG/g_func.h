//
//  g_func.h
//

#ifndef G_FUNC_H
#define G_FUNC_H

void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *self);
void func_train_find (edict_t *self);
void train_use (edict_t *self, edict_t *other, edict_t *activator);
void SP_func_train (edict_t *self);
void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator);
void SP_trigger_elevator (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_conveyor (edict_t *self);
void SP_func_door_secret (edict_t *ent);
void SP_func_killbox (edict_t *ent);
void SP_func_plat (edict_t *ent);

void Move_Done (edict_t *ent);
void Move_Final (edict_t *ent);
void Move_Begin (edict_t *ent);
void Move_Calc (edict_t *ent, vec3_t dest, void(*func)(edict_t*));
void AngleMove_Done (edict_t *ent);
void AngleMove_Final (edict_t *ent);
void AngleMove_Begin (edict_t *ent);
void AngleMove_Calc (edict_t *ent, void(*func)(edict_t*));
void plat_CalcAcceleratedMove(moveinfo_t *moveinfo);
void plat_Accelerate (moveinfo_t *moveinfo);
void Think_AccelMove (edict_t *ent);

void plat_hit_top (edict_t *ent);
void plat_hit_bottom (edict_t *ent);
void plat_go_down (edict_t *ent);
void plat_go_up (edict_t *ent);
void plat_blocked (edict_t *self, edict_t *other);

void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);
void Touch_Plat_Center (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void plat_spawn_inside_trigger (edict_t *ent);

void rotating_blocked (edict_t *self, edict_t *other);
void rotating_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void rotating_use (edict_t *self, edict_t *other, edict_t *activator);

#endif /* G_FUNC_H */
