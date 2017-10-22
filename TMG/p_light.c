// Flashlight 

#include "g_local.h"
/*8===============>
FL_makemake 
<===============8*/

void FL_make(edict_t *self) 
{
	vec3_t	start,forward,right,end;

	if ( self->flashlight )
	{
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
		safe_centerprintf (self, "Flashlight is OFF\n");
		return;
	}

	if(self->flastime > level.time){
		safe_cprintf (self, PRINT_HIGH, "Button must be stuck, wait a few seconds\nand try again\n");
		return;
	}

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(end, 100, 0, 0);
	G_ProjectSource (self->s.origin, end, forward, right, start);
	self->flashlight = G_Spawn ();
	self->flashlight->owner = self;
	self->flashlight->movetype = MOVETYPE_NOCLIP;
	self->flashlight->solid = SOLID_NOT;
	self->flashlight->classname = "flashlight";
	//self->flashlight->s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2/*sprites/sight2.sp2*/");	// HEY KIDDYS NOTE THIS
	self->flashlight->s.modelindex = gi.modelindex ("models/objects/flash/tris.md2");
	self->flashlight->s.skinnum = 0;
	self->s.sound = gi.soundindex("world/x_light.wav");
	self->flashlight->s.effects |= EF_HYPERBLASTER;
	// Other effects can be used here, such as flag1, but these look corney and
	// dull. Try stuff and tell me if you find anything cool
	self->flashlight->think = FL_think;
	self->flashlight->nextthink = level.time + 0.1;
	safe_centerprintf (self, "Flashlight is ON\n");
	self->flastime = level.time +7;
}


// Congratulations, and welcome to the middle of the file.
/*8===============>
FL_makemake the dang thing
<===============8*/
void FL_think (edict_t *self)

{
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;
	trace_t tr;
	AngleVectors (self->owner->client->v_angle, forward, right, up);
	VectorSet(offset, 24, 6, self->owner->viewheight-7);
	G_ProjectSource (self->owner->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace (start, NULL, NULL, end,
		self->owner,
		CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	if (tr.fraction != 1)
	{
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);

	}
	if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
	{
		if ((tr.ent->takedamage) && (tr.ent != self->owner))
		{
			self->s.skinnum = 1;
		}
	}
	else
		self->s.skinnum = 0;
	vectoangles(tr.plane.normal,self->s.angles);
	VectorCopy(tr.endpos,self->s.origin);
	gi.linkentity (self);
	self->nextthink = level.time + 0.1;
}


