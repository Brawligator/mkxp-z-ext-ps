/*
 ** particlesystem-binding.cpp
 **
 ** This file is part of mkxp.
 **
 ** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
 **
 ** mkxp is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** mkxp is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "binding-types.h"
#include "binding-util.h"
#include "disposable-binding.h"
#include "sharedstate.h"
#include "display/particlesystem.h"
#include "display/bitmap.h"
#include "display/viewport.h"
#include <vector>
#include "etc-internal.h"

#if RAPI_FULL > 187
DEF_TYPE_CUSTOMNAME(ParticleSystem, "ParticleSystem");
#else
DEF_ALLOCFUNC(ParticleSystem);
#endif

RB_METHOD(particleSystemInitialize) {
	GFX_LOCK;
	
	VALUE viewportObj = Qnil;
	
	rb_get_args(argc, argv, "|o", &viewportObj RB_ARG_END);
	
	Viewport *viewport = NULL;
	if (!NIL_P(viewportObj)) {
		viewport = getPrivateData<Viewport>(viewportObj);
	}
	
	ParticleSystem *ps = new ParticleSystem(viewport);
	setPrivateData(self, ps);
	
	GFX_UNLOCK;
	return self;
}

DEF_GFX_PROP_I(ParticleSystem, MaxParticles)
DEF_GFX_PROP_I(ParticleSystem, Hue)
DEF_GFX_PROP_I(ParticleSystem, HueVar)
DEF_GFX_PROP_I(ParticleSystem, SizeVar)
DEF_GFX_PROP_I(ParticleSystem, InitialOpacity)
DEF_GFX_PROP_F(ParticleSystem, BaseZoom)
DEF_GFX_PROP_F(ParticleSystem, LifeTime)
DEF_GFX_PROP_I(ParticleSystem, ZOffset)

RB_METHOD(particleSystemSetFilenames) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	VALUE filenamesObj;
	rb_get_args(argc, argv, "o", &filenamesObj RB_ARG_END);
	
	// Convert Ruby array of filenames to std::vector<std::string>
	std::vector<std::string> filenames;
	if (RB_TYPE_P(filenamesObj, RUBY_T_ARRAY)) {
		long len = RARRAY_LEN(filenamesObj);
		for (long i = 0; i < len; ++i) {
			VALUE elem = rb_ary_entry(filenamesObj, i);
			if (!NIL_P(elem)) {
				const char *str = StringValueCStr(elem);
				filenames.push_back(std::string(str));
			}
		}
	} else if (TYPE(filenamesObj) == T_STRING) {
		// Single filename as string
		filenames.push_back(std::string(StringValueCStr(filenamesObj)));
	}
	
	ps->setFilenames(filenames);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetFilenames) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	const std::vector<std::string> &filenames = ps->getFilenames();
	
	VALUE rbArray = rb_ary_new2(filenames.size());
	for (size_t i = 0; i < filenames.size(); ++i) {
		rb_ary_push(rbArray, rb_str_new_cstr(filenames[i].c_str()));
	}
	
	GFX_UNLOCK;
	return rbArray;
}

RB_METHOD(particleSystemSetSpawnSpace) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	VALUE innerSpaceObj;
	VALUE outlineSpaceObj;
	rb_get_args(argc, argv, "oo", &innerSpaceObj, &outlineSpaceObj RB_ARG_END);
	
	// Convert Ruby array of pairs to std::vector<std::pair<int, int>>
	std::vector<std::pair<int, int>> innerSpace;
	if (RB_TYPE_P(innerSpaceObj, RUBY_T_ARRAY)) {
		long len = RARRAY_LEN(innerSpaceObj);
		for (long i = 0; i < len; ++i) {
			VALUE pairObj = rb_ary_entry(innerSpaceObj, i);
			if (RB_TYPE_P(pairObj, RUBY_T_ARRAY) && RARRAY_LEN(pairObj) == 2) {
				int x = NUM2INT(rb_ary_entry(pairObj, 0));
				int y = NUM2INT(rb_ary_entry(pairObj, 1));
				innerSpace.push_back(std::make_pair(x, y));
			}
		}
	}

	std::vector<std::pair<int, int>> outlineSpace;
	if (RB_TYPE_P(outlineSpaceObj, RUBY_T_ARRAY)) {
		long len = RARRAY_LEN(outlineSpaceObj);
		for (long i = 0; i < len; ++i) {
			VALUE pairObj = rb_ary_entry(outlineSpaceObj, i);
			if (RB_TYPE_P(pairObj, RUBY_T_ARRAY) && RARRAY_LEN(pairObj) == 2) {
				int x = NUM2INT(rb_ary_entry(pairObj, 0));
				int y = NUM2INT(rb_ary_entry(pairObj, 1));
				outlineSpace.push_back(std::make_pair(x, y));
			}
		}
	}
	
	ps->setSpawnSpaces(innerSpace, outlineSpace);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetSpawnSpace) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	const std::vector<std::pair<int, int>> &spawnSpace = ps->getSpawnSpace();
	
	VALUE rbArray = rb_ary_new2(spawnSpace.size());
	for (size_t i = 0; i < spawnSpace.size(); ++i) {
		VALUE pair = rb_ary_new2(2);
		rb_ary_push(pair, INT2NUM(spawnSpace[i].first));
		rb_ary_push(pair, INT2NUM(spawnSpace[i].second));
		rb_ary_push(rbArray, pair);
	}
	
	GFX_UNLOCK;
	return rbArray;
}

RB_METHOD(particleSystemSetVelocity) {
	GFX_LOCK;

	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	VALUE velocity;
	rb_get_args(argc, argv, "o", &velocity RB_ARG_END);
	
	// Convert Ruby array of velocities to Vec2
	Vec2 vel;
	if (RB_TYPE_P(velocity, RUBY_T_ARRAY) && RARRAY_LEN(velocity) == 2) {
		vel.x = NUM2INT(rb_ary_entry(velocity, 0));
		vel.y = NUM2INT(rb_ary_entry(velocity, 1));
	}
	ps->setVelocity(vel);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetVelocity) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	Vec2 velocity = ps->getVelocity();
	VALUE rbPair = rb_ary_new2(2);
	rb_ary_push(rbPair, INT2NUM(velocity.x));
	rb_ary_push(rbPair, INT2NUM(velocity.y));

	GFX_UNLOCK;
	return rbPair;
}

RB_METHOD(particleSystemSetAcceleration) {
	GFX_LOCK;

	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	VALUE acceleration;
	rb_get_args(argc, argv, "o", &acceleration RB_ARG_END);
	
	// Convert Ruby array of velocities to Vec2
	Vec2 acc;
	if (RB_TYPE_P(acceleration, RUBY_T_ARRAY) && RARRAY_LEN(acceleration) == 2) {
		acc.x = NUM2INT(rb_ary_entry(acceleration, 0));
		acc.y = NUM2INT(rb_ary_entry(acceleration, 1));
	}
	ps->setAcceleration(acc);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetAcceleration) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	Vec2 acceleration = ps->getAcceleration();
	VALUE rbPair = rb_ary_new2(2);
	rb_ary_push(rbPair, INT2NUM(acceleration.x));
	rb_ary_push(rbPair, INT2NUM(acceleration.y));

	GFX_UNLOCK;
	return rbPair;
}

RB_METHOD(particleSystemSetRandomVelocity) {
	GFX_LOCK;

	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	VALUE velocity;
	rb_get_args(argc, argv, "o", &velocity RB_ARG_END);
	
	// Convert Ruby array of velocities to Vec2
	Vec2 vel;
	if (RB_TYPE_P(velocity, RUBY_T_ARRAY) && RARRAY_LEN(velocity) == 2) {
		vel.x = NUM2INT(rb_ary_entry(velocity, 0));
		vel.y = NUM2INT(rb_ary_entry(velocity, 1));
	}
	ps->setRandomVelocity(vel);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetRandomVelocity) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	Vec2 velocity = ps->getRandomVelocity();
	VALUE rbPair = rb_ary_new2(2);
	rb_ary_push(rbPair, INT2NUM(velocity.x));
	rb_ary_push(rbPair, INT2NUM(velocity.y));

	GFX_UNLOCK;
	return rbPair;
}

RB_METHOD(particleSystemSetRadialVelocity) {
	GFX_LOCK;

	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	VALUE radial_velocity;
	rb_get_args(argc, argv, "o", &radial_velocity RB_ARG_END);
	
	// Convert Ruby array of velocities to Vec2
	Vec3 radial_vel;
	if (RB_TYPE_P(radial_velocity, RUBY_T_ARRAY) && RARRAY_LEN(radial_velocity) == 3) {
		radial_vel.x = NUM2INT(rb_ary_entry(radial_velocity, 0));
		radial_vel.y = NUM2INT(rb_ary_entry(radial_velocity, 1));
		radial_vel.z = NUM2INT(rb_ary_entry(radial_velocity, 2));
	}
	ps->setRadialVelocity(radial_vel);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetRadialVelocity) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	Vec3 radialVelocity = ps->getRadialVelocity();
	VALUE rbTriplet = rb_ary_new2(3);
	rb_ary_push(rbTriplet, INT2NUM(radialVelocity.x));
	rb_ary_push(rbTriplet, INT2NUM(radialVelocity.y));
	rb_ary_push(rbTriplet, INT2NUM(radialVelocity.z));

	GFX_UNLOCK;
	return rbTriplet;
}

RB_METHOD(particleSystemUpdate) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	double deltaTime;
	rb_get_args(argc, argv, "f", &deltaTime RB_ARG_END);
	ps->update((float)deltaTime);
	
	GFX_UNLOCK;
	return Qnil;
}

RB_METHOD(particleSystemDispose) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	ps->dispose();
	
	GFX_UNLOCK;
	return Qnil;
}

RB_METHOD(particleSystemRefresh) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	ps->refresh();
	
	GFX_UNLOCK;
	return Qnil;
}

RB_METHOD(particleSystemSetXY) {
	GFX_LOCK;

	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int x, y;
	rb_get_args(argc, argv, "ii", &x, &y RB_ARG_END);
	ps->setScreenPosition(x, y);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemSetZ) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int z;
	rb_get_args(argc, argv, "i", &z RB_ARG_END);
	ps->setZ(z);
	
	GFX_UNLOCK;
	return self;
}

void particleSystemBindingInit() {
	VALUE klass = rb_define_class("ParticleSystem", rb_cObject);
#if RAPI_FULL > 187
	rb_define_alloc_func(klass, classAllocate<&ParticleSystemType>);
#else
	rb_define_alloc_func(klass, ParticleSystemAllocate);
#endif
	
	disposableBindingInit<ParticleSystem>(klass);

	_rb_define_method(klass, "initialize", particleSystemInitialize);
	
	// Individual setters and getters
	INIT_PROP_BIND(klass, MaxParticles,"max_particles");
	INIT_PROP_BIND(klass, Hue,"hue");
	INIT_PROP_BIND(klass, HueVar,"hue_var");
	INIT_PROP_BIND(klass, SizeVar,"size_var");
	INIT_PROP_BIND(klass, InitialOpacity,"initial_opacity");
	INIT_PROP_BIND(klass, BaseZoom,"base_zoom");
	INIT_PROP_BIND(klass, LifeTime,"life_time");
	INIT_PROP_BIND(klass, ZOffset,"z_offset");
	INIT_PROP_BIND(klass, Filenames,"filenames");
	INIT_PROP_BIND(klass, Velocity,"velocity");
	INIT_PROP_BIND(klass, Acceleration,"acceleration");
	INIT_PROP_BIND(klass, RandomVelocity,"random_velocity");
	INIT_PROP_BIND(klass, RadialVelocity,"radial_velocity");
	_rb_define_method(klass, "set_spawn_space", particleSystemSetSpawnSpace);
	_rb_define_method(klass, "spawn_space", particleSystemGetSpawnSpace);
	
	_rb_define_method(klass, "set_xy", particleSystemSetXY);
	_rb_define_method(klass, "set_z", particleSystemSetZ);
	
	_rb_define_method(klass, "update", particleSystemUpdate);
	_rb_define_method(klass, "dispose", particleSystemDispose);
	_rb_define_method(klass, "refresh", particleSystemRefresh);
}
