//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment
Copyright (C) 2000, 2003 - Matt Saettler (EDuke Enhancements)
Copyright (C) 2020 - Christoph Oelckers

This file is part of Enhanced Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms

EDuke enhancements integrated: 04/13/2003 - Matt Saettler

Note: EDuke source was in transition.  Changes are in-progress in the
source as it is released.

*/
//-------------------------------------------------------------------------

#include <utility>
#include "ns.h"
#include "global.h"
#include "sounds.h"
#include "dukeactor.h"
#include "interpolate.h"

// PRIMITIVE
BEGIN_DUKE_NS

static int interptype[] = { Interp_Sect_Floorz, Interp_Sect_Ceilingz, Interp_Wall_X, Interp_Wall_Y };

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------
static bool haltsoundhack;

int callsound(int sn, DDukeActor* whatsprite)
{
	if (!isRRRA() && haltsoundhack)
	{
		haltsoundhack = 0;
		return -1;
	}

	DukeSectIterator it(sn);
	while (auto act = it.Next())
	{
		auto si = act->s;
		if (si->picnum == MUSICANDSFX && si->lotag < 1000)
		{
			if (whatsprite == nullptr) whatsprite = act;

			int snum = si->lotag;
			auto flags = S_GetUserFlags(snum);

			// Reset if the desired actor isn't playing anything.
			bool hival = S_IsSoundValid(si->hitag);
			if (act->temp_data[0] == 1 && !hival)
			{
				if (!S_CheckActorSoundPlaying(act->temp_actor, snum))
					act->temp_data[0] = 0;
			}

			if (act->temp_data[0] == 0)
			{
				if ((flags & (SF_GLOBAL | SF_DTAG)) != SF_GLOBAL)
				{
					if (snum)
					{
						if (si->hitag && snum != si->hitag)
							S_StopSound(si->hitag, act->temp_actor);
						S_PlayActorSound(snum, whatsprite);
						act->temp_actor = whatsprite;
					}

					if ((si->sector()->lotag & 0xff) != ST_22_SPLITTING_DOOR)
						act->temp_data[0] = 1;
				}
			}
			else if (si->hitag < 1000)
			{
				if ((flags & SF_LOOP) || (si->hitag && si->hitag != si->lotag))
					S_StopSound(si->lotag, act->temp_actor);
				if (si->hitag) S_PlayActorSound(si->hitag, whatsprite);
				act->temp_data[0] = 0;
				act->temp_actor = whatsprite;
			}
			return si->lotag;
		}
	}
	return -1;
}


//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int check_activator_motion(int lotag)
{
	DukeStatIterator it(STAT_ACTIVATOR);
	while (auto act = it.Next())
	{
		if (act->s->lotag == lotag)
		{
			for (int j = animatecnt - 1; j >= 0; j--)
				if (act->s->sectnum == animatesect[j])
					return(1);

			DukeStatIterator it1(STAT_EFFECTOR);
			while (auto act2 = it1.Next())
			{
				if (act->s->sectnum == act2->s->sectnum)
					switch (act2->s->lotag)
					{
					case SE_11_SWINGING_DOOR:
					case SE_30_TWO_WAY_TRAIN:
						if (act2->temp_data[4])
							return(1);
						break;
					case SE_18_INCREMENTAL_SECTOR_RISE_FALL:
						if (isRRRA()) break;
					case SE_20_STRETCH_BRIDGE:
					case SE_31_FLOOR_RISE_FALL:
					case SE_32_CEILING_RISE_FALL:
						if (act2->temp_data[0])
							return(1);
						break;
					}

			}
		}
	}
	return(0);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

bool isanunderoperator(int lotag)
{
	switch (lotag & 0xff)
	{
	case ST_15_WARP_ELEVATOR:
	case ST_16_PLATFORM_DOWN:
	case ST_17_PLATFORM_UP:
	case ST_18_ELEVATOR_DOWN:
	case ST_19_ELEVATOR_UP:
	case ST_26_SPLITTING_ST_DOOR:
		return true;
	case ST_22_SPLITTING_DOOR:
		return !isRR();
	}
	return false;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

bool isanearoperator(int lotag)
{
	switch (lotag & 0xff)
	{
	case ST_9_SLIDING_ST_DOOR:
	case ST_15_WARP_ELEVATOR:
	case ST_16_PLATFORM_DOWN:
	case ST_17_PLATFORM_UP:
	case ST_18_ELEVATOR_DOWN:
	case ST_19_ELEVATOR_UP:
	case ST_20_CEILING_DOOR:
	case ST_21_FLOOR_DOOR:
	case ST_22_SPLITTING_DOOR:
	case ST_23_SWINGING_DOOR:
	case ST_25_SLIDING_DOOR:
	case ST_26_SPLITTING_ST_DOOR:
	case ST_29_TEETH_DOOR:
		return true;
	case 41:
		return isRR();
	}
	return false;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int findplayer(const DDukeActor* actor, int* d)
{
	int j, closest_player;
	int x, closest;
	auto s = &actor->s->pos;

	if (ud.multimode < 2)
	{
		if (d) *d = abs(ps[myconnectindex].oposx - s->x) + abs(ps[myconnectindex].oposy - s->y) + ((abs(ps[myconnectindex].oposz - s->z + (28 << 8))) >> 4);
		return myconnectindex;
	}

	closest = 0x7fffffff;
	closest_player = 0;

	for (j = connecthead; j >= 0; j = connectpoint2[j])
	{
		x = abs(ps[j].oposx - s->x) + abs(ps[j].oposy - s->y) + ((abs(ps[j].oposz - s->z + (28 << 8))) >> 4);
		if (x < closest && ps[j].GetActor()->s->extra > 0)
		{
			closest_player = j;
			closest = x;
		}
	}

	if (d) *d = closest;
	return closest_player;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int findotherplayer(int p, int* d)
{
	int j, closest_player;
	int x, closest;

	closest = 0x7fffffff;
	closest_player = p;

	for (j = connecthead; j >= 0; j = connectpoint2[j])
		if (p != j && ps[j].GetActor()->s->extra > 0)
		{
			x = abs(ps[j].oposx - ps[p].pos.x) + abs(ps[j].oposy - ps[p].pos.y) + (abs(ps[j].oposz - ps[p].pos.z) >> 4);

			if (x < closest)
			{
				closest_player = j;
				closest = x;
			}
		}

	*d = closest;
	return closest_player;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int* animateptr(int type, int index)
{
	static int scratch;
	switch (type)
	{
	case anim_floorz:
		return &sector[index].floorz;
	case anim_ceilingz:
		return &sector[index].ceilingz;
	case anim_vertexx:
		return &wall[index].x;
	case anim_vertexy:
		return &wall[index].y;
	default:
		assert(false);
		return &scratch;
	}
}

int* animateptr(int i)
{
	return animateptr(animatetype[i], animatetarget[i]);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void doanimations(void)
{
	int i, a, p, v, dasect;

	for (i = animatecnt - 1; i >= 0; i--)
	{
		a = *animateptr(i);
		v = animatevel[i] * TICSPERFRAME;
		dasect = animatesect[i];

		if (a == animategoal[i])
		{
			StopInterpolation(animatetarget[i], interptype[animatetype[i]]);

			animatecnt--;
			animatetype[i] = animatetype[animatecnt];
			animatetarget[i] = animatetarget[animatecnt];
			animategoal[i] = animategoal[animatecnt];
			animatevel[i] = animatevel[animatecnt];
			animatesect[i] = animatesect[animatecnt];
			if (sector[animatesect[i]].lotag == ST_18_ELEVATOR_DOWN || sector[animatesect[i]].lotag == ST_19_ELEVATOR_UP)
				if (animatetype[i] == anim_ceilingz)
					continue;

			if ((sector[dasect].lotag & 0xff) != ST_22_SPLITTING_DOOR)
				callsound(dasect, nullptr);

			continue;
		}

		if (v > 0) { a = min(a + v, animategoal[i]); }
		else { a = max(a + v, animategoal[i]); }

		if (animatetype[i] == anim_floorz)
		{
			for (p = connecthead; p >= 0; p = connectpoint2[p])
				if (ps[p].cursectnum == dasect)
					if ((sector[dasect].floorz - ps[p].pos.z) < (64 << 8))
						if (ps[p].GetActor()->GetOwner() != nullptr)
						{
							ps[p].pos.z += v;
							ps[p].poszv = 0;
						}

			DukeSectIterator it(dasect);
			while (auto act = it.Next())
			{
				if (act->s->statnum != STAT_EFFECTOR)
				{
					act->s->backupz();
					act->s->z += v;
					act->floorz = sector[dasect].floorz + v;
				}
			}
		}

		*animateptr(i) = a;
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int getanimationgoal(int animtype, int animtarget)
{
	int i, j;

	j = -1;
	for (i = animatecnt - 1; i >= 0; i--)
		if (animtype == animatetype[i] && animtarget == animatetarget[i])
		{
			j = i;
			break;
		}
	return(j);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

int setanimation(int animsect, int animtype, int animtarget, int thegoal, int thevel)
{
	int i, j;

	if (animatecnt >= MAXANIMATES - 1)
		return(-1);

	j = animatecnt;
	for (i = 0; i < animatecnt; i++)
		if (animtype == animatetype[i] && animtarget == animatetarget[i])
		{
			j = i;
			break;
		}

	auto animptr = animateptr(animtype, animtarget);
	animatesect[j] = animsect;
	animatetype[j] = animtype;
	animatetarget[j] = animtarget;
	animategoal[j] = thegoal;
	if (thegoal >= *animptr)
		animatevel[j] = thevel;
	else
		animatevel[j] = -thevel;

	if (j == animatecnt) animatecnt++;

	StartInterpolation(animatetarget[i], interptype[animatetype[i]]);
	return(j);
}


//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

bool activatewarpelevators(DDukeActor* actor, int d) //Parm = sectoreffectornum
{
	int sn = actor->s->sectnum;

	// See if the sector exists

	DukeStatIterator it(STAT_EFFECTOR);
	DDukeActor *act2;
	while ((act2 = it.Next()))
	{
		if (act2->s->lotag == SE_17_WARP_ELEVATOR || (isRRRA() && act2->s->lotag == SE_18_INCREMENTAL_SECTOR_RISE_FALL))
			if (act2->s->hitag == actor->s->hitag)
				if ((abs(sector[sn].floorz - actor->temp_data[2]) > act2->s->yvel) ||
					(act2->getSector()->hitag == (sector[sn].hitag - d)))
					break;
	}

	if (act2 == nullptr)
	{
		d = 0;
		return 1; // No find
	}
	else
	{
		if (d == 0)
			S_PlayActorSound(ELEVATOR_OFF, actor);
		else S_PlayActorSound(ELEVATOR_ON, actor);
	}


	it.Reset(STAT_EFFECTOR);
	while ((act2 = it.Next()))
	{
		if (act2->s->lotag == SE_17_WARP_ELEVATOR || (isRRRA() && act2->s->lotag == SE_18_INCREMENTAL_SECTOR_RISE_FALL))
			if (act2->s->hitag == actor->s->hitag)
			{
				act2->temp_data[0] = d;
				act2->temp_data[1] = d; //Make all check warp
			}
	}
	return 0;
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st09(int sn, DDukeActor* actor)
{
	int dax, day, dax2, day2, sp;
	int wallfind[2];
	sectortype* sptr = &sector[sn];

	int startwall = sptr->wallptr;
	int endwall = startwall + sptr->wallnum - 1;

	sp = sptr->extra >> 4;

	//first find center point by averaging all points
	dax = 0L, day = 0L;
	for (int i = startwall; i <= endwall; i++)
	{
		dax += wall[i].x;
		day += wall[i].y;
	}
	dax /= (endwall - startwall + 1);
	day /= (endwall - startwall + 1);

	//find any points with either same x or same y coordinate
	//  as center (dax, day) - should be 2 points found.
	wallfind[0] = -1;
	wallfind[1] = -1;
	for (int i = startwall; i <= endwall; i++)
		if ((wall[i].x == dax) || (wall[i].y == day))
		{
			if (wallfind[0] == -1)
				wallfind[0] = i;
			else wallfind[1] = i;
		}

	for (int j = 0; j < 2; j++)
	{
		if ((wall[wallfind[j]].x == dax) && (wall[wallfind[j]].y == day))
		{
			//find what direction door should open by averaging the
			//  2 neighboring points of wallfind[0] & wallfind[1].
			int i = wallfind[j] - 1; if (i < startwall) i = endwall;
			dax2 = ((wall[i].x + wall[wall[wallfind[j]].point2].x) >> 1) - wall[wallfind[j]].x;
			day2 = ((wall[i].y + wall[wall[wallfind[j]].point2].y) >> 1) - wall[wallfind[j]].y;
			if (dax2 != 0)
			{
				dax2 = wall[wall[wall[wallfind[j]].point2].point2].x;
				dax2 -= wall[wall[wallfind[j]].point2].x;
				setanimation(sn, anim_vertexx, wallfind[j], wall[wallfind[j]].x + dax2, sp);
				setanimation(sn, anim_vertexx, i, wall[i].x + dax2, sp);
				setanimation(sn, anim_vertexx, wall[wallfind[j]].point2, wall[wall[wallfind[j]].point2].x + dax2, sp);
				callsound(sn, actor);
			}
			else if (day2 != 0)
			{
				day2 = wall[wall[wall[wallfind[j]].point2].point2].y;
				day2 -= wall[wall[wallfind[j]].point2].y;
				setanimation(sn, anim_vertexy, wallfind[j], wall[wallfind[j]].y + day2, sp);
				setanimation(sn, anim_vertexy, i, wall[i].y + day2, sp);
				setanimation(sn, anim_vertexy, wall[wallfind[j]].point2, wall[wall[wallfind[j]].point2].y + day2, sp);
				callsound(sn, actor);
			}
		}
		else
		{
			int i = wallfind[j] - 1; if (i < startwall) i = endwall;
			dax2 = ((wall[i].x + wall[wall[wallfind[j]].point2].x) >> 1) - wall[wallfind[j]].x;
			day2 = ((wall[i].y + wall[wall[wallfind[j]].point2].y) >> 1) - wall[wallfind[j]].y;
			if (dax2 != 0)
			{
				setanimation(sn, anim_vertexx, wallfind[j], dax, sp);
				setanimation(sn, anim_vertexx, i, dax + dax2, sp);
				setanimation(sn, anim_vertexx, wall[wallfind[j]].point2, dax + dax2, sp);
				callsound(sn, actor);
			}
			else if (day2 != 0)
			{
				setanimation(sn, anim_vertexy, wallfind[j], day, sp);
				setanimation(sn, anim_vertexy, i, day + day2, sp);
				setanimation(sn, anim_vertexy, wall[wallfind[j]].point2, day + day2, sp);
				callsound(sn, actor);
			}
		}
	}

}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st15(int sn, DDukeActor* actor)
{
	if (actor->s->picnum != TILE_APLAYER) return;

	sectortype* sptr = &sector[sn];
	DukeSectIterator it(sn);
	DDukeActor* a2;
	while ((a2 = it.Next()))
	{
		if (a2->s->picnum == SECTOREFFECTOR && a2->s->lotag == ST_17_PLATFORM_UP) break;
	}
	if (!a2) return;

	if (actor->s->sectnum == sn)
	{
		if (activatewarpelevators(a2, -1))
			activatewarpelevators(a2, 1);
		else if (activatewarpelevators(a2, 1))
			activatewarpelevators(a2, -1);
		return;
	}
	else
	{
		if (sptr->floorz > a2->s->z)
			activatewarpelevators(a2, -1);
		else
			activatewarpelevators(a2, 1);
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st16(int sn, DDukeActor* actor)
{
	sectortype* sptr = &sector[sn];
	int i = getanimationgoal(anim_floorz, sn);
	int j;

	if (i == -1)
	{
		i = nextsectorneighborz(sn, sptr->floorz, 1, 1);
		if (i == -1)
		{
			i = nextsectorneighborz(sn, sptr->floorz, 1, -1);
			if (i == -1) return;
			j = sector[i].floorz;
			setanimation(sn, anim_floorz, sn, j, sptr->extra);
		}
		else
		{
			j = sector[i].floorz;
			setanimation(sn, anim_floorz, sn, j, sptr->extra);
		}
		callsound(sn, actor);
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st18(int sn, DDukeActor* actor)
{
	sectortype* sptr = &sector[sn];
	int i = getanimationgoal(anim_floorz, sn);

	if (i == -1)
	{
		i = nextsectorneighborz(sn, sptr->floorz, 1, -1);
		if (i == -1) i = nextsectorneighborz(sn, sptr->floorz, 1, 1);
		if (i == -1) return;
		int j = sector[i].floorz;
		int q = sptr->extra;
		int l = sptr->ceilingz - sptr->floorz;
		setanimation(sn, anim_floorz, sn, j, q);
		setanimation(sn, anim_ceilingz, sn, j + l, q);
		callsound(sn, actor);
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st29(int sn, DDukeActor* actor)
{
	sectortype* sptr = &sector[sn];
	int j;

	if (sptr->lotag & 0x8000)
		j = sector[nextsectorneighborz(sn, sptr->ceilingz, 1, 1)].floorz;
	else
		j = sector[nextsectorneighborz(sn, sptr->ceilingz, -1, -1)].ceilingz;

	DukeStatIterator it(STAT_EFFECTOR);
	while (auto act2 = it.Next())
	{
		if ((act2->s->lotag == 22) &&
			(act2->s->hitag == sptr->hitag))
		{
			act2->getSector()->extra = -act2->getSector()->extra;

			act2->temp_data[0] = sn;
			act2->temp_data[1] = 1;
		}
	}

	sptr->lotag ^= 0x8000;

	setanimation(sn, anim_ceilingz, sn, j, sptr->extra);

	callsound(sn, actor);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st20(int sn, DDukeActor* actor)
{
	sectortype* sptr = &sector[sn];
	int j;
REDODOOR:

	if (sptr->lotag & 0x8000)
	{
		DDukeActor* a2;
		DukeSectIterator it(sn);
		while ((a2 = it.Next()))
		{
			if (a2->s->statnum == 3 && a2->s->lotag == 9)
			{
				j = a2->s->z;
				break;
			}
		}
		if (a2 == nullptr)
			j = sptr->floorz;
	}
	else
	{
		j = nextsectorneighborz(sn, sptr->ceilingz, -1, -1);

		if (j >= 0) j = sector[j].ceilingz;
		else
		{
			sptr->lotag |= 32768;
			goto REDODOOR;
		}
	}

	sptr->lotag ^= 0x8000;

	setanimation(sn, anim_ceilingz, sn, j, sptr->extra);
	callsound(sn, actor);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st21(int sn, DDukeActor* actor)
{
	sectortype* sptr = &sector[sn];
	int i = getanimationgoal(anim_floorz, sn);
	int j;
	if (i >= 0)
	{
		if (animategoal[i] == sptr->ceilingz)
			animategoal[i] = sector[nextsectorneighborz(sn, sptr->ceilingz, 1, 1)].floorz;
		else animategoal[i] = sptr->ceilingz;
		j = animategoal[i];
	}
	else
	{
		if (sptr->ceilingz == sptr->floorz)
			j = sector[nextsectorneighborz(sn, sptr->ceilingz, 1, 1)].floorz;
		else j = sptr->ceilingz;

		sptr->lotag ^= 0x8000;

		if (setanimation(sn, anim_floorz, sn, j, sptr->extra) >= 0)
			callsound(sn, actor);
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st22(int sn, DDukeActor* actor)
{
	sectortype* sptr = &sector[sn];
	int j, q;
	if ((sptr->lotag & 0x8000))
	{
		q = (sptr->ceilingz + sptr->floorz) >> 1;
		j = setanimation(sn, anim_floorz, sn, q, sptr->extra);
		j = setanimation(sn, anim_ceilingz, sn, q, sptr->extra);
	}
	else
	{
		q = sector[nextsectorneighborz(sn, sptr->floorz, 1, 1)].floorz;
		j = setanimation(sn, anim_floorz, sn, q, sptr->extra);
		q = sector[nextsectorneighborz(sn, sptr->ceilingz, -1, -1)].ceilingz;
		j = setanimation(sn, anim_ceilingz, sn, q, sptr->extra);
	}

	sptr->lotag ^= 0x8000;

	callsound(sn, actor);

}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st23(int sn, DDukeActor* actor)
{
	int q = 0;
	
	DukeStatIterator it(STAT_EFFECTOR);
	DDukeActor* act2;
	while ((act2 = it.Next()))
	{
		if (act2->s->lotag == SE_11_SWINGING_DOOR && act2->s->sectnum == sn && !act2->temp_data[4])
		{
			break;
		}
	}
	if (!act2) return;

	int l = act2->getSector()->lotag & 0x8000;

	if (act2)
	{
		DukeStatIterator it(STAT_EFFECTOR);

		while (auto act3 = it.Next())
		{
			if (l == (act3->getSector()->lotag & 0x8000) && act3->s->lotag == SE_11_SWINGING_DOOR && act2->s->hitag == act3->s->hitag && act3->temp_data[4])
			{
				return;
			}
		}

		it.Reset(STAT_EFFECTOR);
		while (auto act3 = it.Next())
		{
			if (l == (act3->getSector()->lotag & 0x8000) && act3->s->lotag == SE_11_SWINGING_DOOR && act2->s->hitag == act3->s->hitag)
			{
				if (act3->getSector()->lotag & 0x8000) act3->getSector()->lotag &= 0x7fff;
				else act3->getSector()->lotag |= 0x8000;
				act3->temp_data[4] = 1;
				act3->temp_data[3] = -act3->temp_data[3];
				if (q == 0)
				{
					callsound(sn, act3);
					q = 1;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st25(int sn, DDukeActor* actor)
{
	DukeStatIterator it(STAT_EFFECTOR);
	DDukeActor* act2;
	while ((act2 = it.Next()))
	{
		if (act2->s->lotag == 15 && act2->s->sectnum == sn)
		{
			break;
		}
	}
	
	if (act2 == nullptr)
		return;

	it.Reset(STAT_EFFECTOR);
	while (auto act3 = it.Next())
	{
		if (act3->s->hitag == act2->s->hitag)
		{
			if (act3->s->lotag == 15)
			{
				act3->getSector()->lotag ^= 0x8000; // Toggle the open or close
				act3->s->ang += 1024;
				if (act3->temp_data[4]) callsound(act3->s->sectnum, act3);
				callsound(act3->s->sectnum, act3);
				if (act3->getSector()->lotag & 0x8000) act3->temp_data[4] = 1;
				else act3->temp_data[4] = 2;
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st27(int sn, DDukeActor* actor)
{
	DukeStatIterator it(STAT_EFFECTOR);
	while (auto act2 = it.Next())
	{
		if ((act2->s->lotag & 0xff) == 20 && act2->s->sectnum == sn) //Bridge
		{

			sector[sn].lotag ^= 0x8000;
			if (sector[sn].lotag & 0x8000) //OPENING
				act2->temp_data[0] = 1;
			else act2->temp_data[0] = 2;
			callsound(sn, actor);
			break;
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

static void handle_st28(int sn, DDukeActor* actor)
{
	//activate the rest of them
	int j = -1;
	DukeSectIterator it(sn);
	while (auto a2 = it.Next())
	{
		if (a2->s->statnum == 3 && (a2->s->lotag & 0xff) == 21)
		{
			j = a2->s->hitag;
			break; //Found it
		}
	}

	if (j == -1) return;
	DukeStatIterator it1(STAT_EFFECTOR);
	while (auto act3 = it.Next())
	{
		if ((act3->s->lotag & 0xff) == 21 && !act3->temp_data[0] &&
			(act3->s->hitag) == j)
			act3->temp_data[0] = 1;
	}
	callsound(sn, actor);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void operatesectors(int sn, DDukeActor *actor)
{
	int j=0, startwall, endwall;
	int i;
	sectortype* sptr;

	sptr = &sector[sn];

	switch (sptr->lotag & (0x3fff))
	{

	case 41:
		if (isRR()) operatejaildoors(sptr->hitag);
		break;

	case 7:
		if (!isRR()) break;
		startwall = sptr->wallptr;
		endwall = startwall + sptr->wallnum;
		for (j = startwall; j < endwall; j++)
		{
			setanimation(sn, anim_vertexx, j, wall[j].x + 1024, 4);
			setanimation(sn, anim_vertexx, wall[j].nextwall, wall[wall[j].nextwall].x + 1024, 4);
		}
		break;

	case ST_30_ROTATE_RISE_BRIDGE:
	{
		auto act = ScriptIndexToActor(sptr->hitag);
		if (!act) break;
		if (act->tempang == 0 || act->tempang == 256) callsound(sn, actor);
		if (act->s->extra == 1) act->s->extra = 3;
		else act->s->extra = 1;
		break;
	}

	case ST_31_TWO_WAY_TRAIN:
	{
		auto act = ScriptIndexToActor(sptr->hitag);
		if (!act) break;
		if (act->temp_data[4] == 0)
			act->temp_data[4] = 1;

		callsound(sn, actor);
		break;
	}
	case ST_26_SPLITTING_ST_DOOR: //The split doors
		i = getanimationgoal(anim_ceilingz, sn);
		if (i == -1) //if the door has stopped
		{
			haltsoundhack = 1;
			sptr->lotag &= 0xff00;
			sptr->lotag |= ST_22_SPLITTING_DOOR;
			operatesectors(sn, actor);
			sptr->lotag &= 0xff00;
			sptr->lotag |= ST_9_SLIDING_ST_DOOR;
			operatesectors(sn, actor);
			sptr->lotag &= 0xff00;
			sptr->lotag |= ST_26_SPLITTING_ST_DOOR;
		}
		return;

	case ST_9_SLIDING_ST_DOOR:
		handle_st09(sn, actor);
		return;

	case ST_15_WARP_ELEVATOR://Warping elevators
		handle_st15(sn, actor);
		return;

	case ST_16_PLATFORM_DOWN:
	case ST_17_PLATFORM_UP:
		handle_st16(sn, actor);
		return;

	case ST_18_ELEVATOR_DOWN:
	case ST_19_ELEVATOR_UP:
		handle_st18(sn, actor);
		return;

	case ST_29_TEETH_DOOR:
		handle_st29(sn, actor);
		return;
	case ST_20_CEILING_DOOR:
		handle_st20(sn, actor);
		return;

	case ST_21_FLOOR_DOOR:
		handle_st21(sn, actor);
		return;

	case ST_22_SPLITTING_DOOR:
		handle_st22(sn, actor);
		return;

	case ST_23_SWINGING_DOOR: //Swingdoor
		handle_st23(sn, actor);
		return;

	case ST_25_SLIDING_DOOR: //Subway type sliding doors
	{
		handle_st25(sn, actor);
		return;
	}
	case ST_27_STRETCH_BRIDGE:  //Extended bridge
		handle_st27(sn, actor);
		return;

	case ST_28_DROP_FLOOR:
		handle_st28(sn, actor);
		return;
	}
}


//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void operateactivators(int low, int plnum)
{
	int i, j, k;
	Cycler * p;
	walltype* wal;

	for (i = numcyclers - 1; i >= 0; i--)
	{
		p = &cyclers[i];


		if (p->hitag == low)
		{
			auto sect = p->sector();
			p->state = !p->state;

			sect->floorshade = sect->ceilingshade = (int8_t)p->shade2;
			wal = sect->firstWall();
			for (j = sect->wallnum; j > 0; j--, wal++)
				wal->shade = (int8_t)p->shade2;
		}
	}

	k = -1;
	DukeStatIterator it(STAT_ACTIVATOR);
	while (auto act = it.Next())
	{
		if (act->s->lotag == low)
		{
			if (act->s->picnum == ACTIVATORLOCKED)
			{
				act->getSector()->lotag ^= 16384;

				if (plnum >= 0)
				{
					if (act->getSector()->lotag & 16384)
						FTA(4, &ps[plnum]);
					else FTA(8, &ps[plnum]);
				}
			}
			else
			{
				switch (act->s->hitag)
				{
				case 0:
					break;
				case 1:
					if (act->getSector()->floorz != act->getSector()->ceilingz)
					{
						continue;
					}
					break;
				case 2:
					if (act->getSector()->floorz == act->getSector()->ceilingz)
					{
						continue;
					}
					break;
				}

				if (act->getSector()->lotag < 3)
				{
					DukeSectIterator it(act->s->sectnum);
					while (auto a2 = it.Next())
					{
						if (a2->s->statnum == 3) switch (a2->s->lotag)
						{
						case SE_18_INCREMENTAL_SECTOR_RISE_FALL:
							if (isRRRA()) break;
						case SE_36_PROJ_SHOOTER:
						case SE_31_FLOOR_RISE_FALL:
						case SE_32_CEILING_RISE_FALL:
							a2->temp_data[0] = 1 - a2->temp_data[0];
							callsound(act->s->sectnum, a2);
							break;
						}
					}
				}

				if (k == -1 && (act->getSector()->lotag & 0xff) == 22)
					k = callsound(act->s->sectnum, act);

				operatesectors(act->s->sectnum, act);
			}
		}
	}

	fi.operaterespawns(low);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void operatemasterswitches(int low)
{
	DukeStatIterator it(STAT_STANDABLE);
	while (auto act2 = it.Next())
	{
		if (act2->s->picnum == MASTERSWITCH && act2->s->lotag == low && act2->s->yvel == 0)
			act2->s->yvel = 1;
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void operateforcefields_common(DDukeActor *effector, int low, const std::initializer_list<int> &tiles)
{
	int i, p;

	for (p = numanimwalls; p >= 0; p--)
	{
		i = animwall[p].wallnum;

		if (low == wall[i].lotag || low == -1)
			if (isIn(wall[i].overpicnum, tiles))
			{
				animwall[p].tag = 0;

				if (wall[i].cstat)
				{
					wall[i].cstat = 0;

					if (effector && effector->s->picnum == SECTOREFFECTOR && effector->s->lotag == 30)
						wall[i].lotag = 0;
				}
				else
					wall[i].cstat = 85;
			}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void breakwall(int newpn, DDukeActor* spr, int dawallnum)
{
	wall[dawallnum].picnum = newpn;
	S_PlayActorSound(VENT_BUST, spr);
	S_PlayActorSound(GLASS_HEAVYBREAK, spr);
	lotsofglass(spr, dawallnum, 10);
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void allignwarpelevators(void)
{
	DukeStatIterator it(STAT_EFFECTOR);
	while (auto act = it.Next())
	{
		if (act->s->lotag == SE_17_WARP_ELEVATOR && act->s->shade > 16)
		{
			DukeStatIterator it1(STAT_EFFECTOR);
			while (auto act2 = it1.Next())
			{
				if ((act2->s->lotag) == SE_17_WARP_ELEVATOR && act != act2 && act->s->hitag == act2->s->hitag)
				{
					act2->getSector()->floorz = act->getSector()->floorz;
					act2->getSector()->ceilingz = act->getSector()->ceilingz;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void moveclouds(double smoothratio)
{
	// The math here is very messy.. :(
	int myclock = smoothratio < 32768? PlayClock-2 : PlayClock;
	if (myclock > cloudclock || myclock < (cloudclock - 7))
	{
		cloudclock = myclock + 6;

		// cloudx/y were an array, but all entries were always having the same value so a single pair is enough.
		cloudx += (float)ps[screenpeek].angle.ang.fcos() * 0.5f;
		cloudy += (float)ps[screenpeek].angle.ang.fsin() * 0.5f;
		for (int i = 0; i < numclouds; i++)
		{
			if (!testnewrenderer)
			{
				sector[clouds[i]].setceilingxpan(cloudx);
				sector[clouds[i]].setceilingypan(cloudy);
			}
			else
			{ 
				// no clamping here!
				sector[clouds[i]].ceilingxpan_ = cloudx;
				sector[clouds[i]].ceilingypan_ = cloudy;
			}
			sector[clouds[i]].exflags |= SECTOREX_CLOUDSCROLL;
		}
	}
}




END_DUKE_NS
