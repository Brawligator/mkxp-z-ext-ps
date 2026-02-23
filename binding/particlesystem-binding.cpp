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

RB_METHOD(particleSystemSetMaxParticles) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int maxParticles;
	rb_get_args(argc, argv, "i", &maxParticles RB_ARG_END);
	ps->setMaxParticles(maxParticles);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetMaxParticles) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int result = ps->getMaxParticles();
	GFX_UNLOCK;
	return INT2NUM(result);
}

RB_METHOD(particleSystemSetHue) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int hue;
	rb_get_args(argc, argv, "i", &hue RB_ARG_END);
	ps->setHue(hue);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetHue) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int result = ps->getHue();
	GFX_UNLOCK;
	return INT2NUM(result);
}

RB_METHOD(particleSystemSetHueVar) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int hueVar;
	rb_get_args(argc, argv, "i", &hueVar RB_ARG_END);
	ps->setHueVar(hueVar);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetHueVar) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int result = ps->getHueVar();
	GFX_UNLOCK;
	return INT2NUM(result);
}

RB_METHOD(particleSystemSetSizeVar) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int sizeVar;
	rb_get_args(argc, argv, "i", &sizeVar RB_ARG_END);
	ps->setSizeVar(sizeVar);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetSizeVar) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int result = ps->getSizeVar();
	GFX_UNLOCK;
	return INT2NUM(result);
}

RB_METHOD(particleSystemSetInitialOpacity) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int opacity;
	rb_get_args(argc, argv, "i", &opacity RB_ARG_END);
	ps->setInitialOpacity(opacity);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetInitialOpacity) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int result = ps->getInitialOpacity();
	GFX_UNLOCK;
	return INT2NUM(result);
}

RB_METHOD(particleSystemSetBaseZoom) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	double baseZoom;
	rb_get_args(argc, argv, "f", &baseZoom RB_ARG_END);
	ps->setBaseZoom((float)baseZoom);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetBaseZoom) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	float result = ps->getBaseZoom();
	GFX_UNLOCK;
	return DBL2NUM(result);
}

RB_METHOD(particleSystemSetLifeTime) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	double lifeTime;
	rb_get_args(argc, argv, "f", &lifeTime RB_ARG_END);
	ps->setLifeTime((float)lifeTime);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetLifeTime) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	float result = ps->getLifeTime();
	GFX_UNLOCK;
	return DBL2NUM(result);
}

RB_METHOD(particleSystemSetZOffset) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int zOffset;
	rb_get_args(argc, argv, "i", &zOffset RB_ARG_END);
	ps->setZOffset(zOffset);
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemGetZOffset) {
	GFX_LOCK;
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	int result = ps->getZOffset();
	GFX_UNLOCK;
	return INT2NUM(result);
}

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
	int x, y;
	rb_get_args(argc, argv, "ii", &x, &y RB_ARG_END);
	ps->setVelocity(Vec2(x, y));
	
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
	int x, y;
	rb_get_args(argc, argv, "ii", &x, &y RB_ARG_END);
	ps->setAcceleration(x, y);
	
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
	_rb_define_method(klass, "max_particles=", particleSystemSetMaxParticles);
	_rb_define_method(klass, "max_particles", particleSystemGetMaxParticles);
	_rb_define_method(klass, "hue=", particleSystemSetHue);
	_rb_define_method(klass, "hue", particleSystemGetHue);
	_rb_define_method(klass, "hue_var=", particleSystemSetHueVar);
	_rb_define_method(klass, "hue_var", particleSystemGetHueVar);
	_rb_define_method(klass, "size_var=", particleSystemSetSizeVar);
	_rb_define_method(klass, "size_var", particleSystemGetSizeVar);
	_rb_define_method(klass, "initial_opacity=", particleSystemSetInitialOpacity);
	_rb_define_method(klass, "initial_opacity", particleSystemGetInitialOpacity);
	_rb_define_method(klass, "base_zoom=", particleSystemSetBaseZoom);
	_rb_define_method(klass, "base_zoom", particleSystemGetBaseZoom);
	_rb_define_method(klass, "life_time=", particleSystemSetLifeTime);
	_rb_define_method(klass, "life_time", particleSystemGetLifeTime);
	_rb_define_method(klass, "z_offset=", particleSystemSetZOffset);
	_rb_define_method(klass, "z_offset", particleSystemGetZOffset);
	_rb_define_method(klass, "filenames=", particleSystemSetFilenames);
	_rb_define_method(klass, "filenames", particleSystemGetFilenames);
	_rb_define_method(klass, "set_spawn_space", particleSystemSetSpawnSpace);
	_rb_define_method(klass, "spawn_space", particleSystemGetSpawnSpace);
	_rb_define_method(klass, "velocity=", particleSystemSetVelocity);
	_rb_define_method(klass, "velocity", particleSystemGetVelocity);
	_rb_define_method(klass, "acceleration=", particleSystemSetAcceleration);
	_rb_define_method(klass, "acceleration", particleSystemGetAcceleration);
	
	_rb_define_method(klass, "set_xy", particleSystemSetXY);
	_rb_define_method(klass, "set_z", particleSystemSetZ);
	
	_rb_define_method(klass, "update", particleSystemUpdate);
	_rb_define_method(klass, "dispose", particleSystemDispose);
	_rb_define_method(klass, "refresh", particleSystemRefresh);
}
