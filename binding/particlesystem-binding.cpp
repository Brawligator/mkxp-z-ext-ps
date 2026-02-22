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

RB_METHOD(particleSystemSetParameters) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	
	VALUE filenamesObj;
	int maxParticles, hue, opacityVar, opacity, zOffset = -1;
	double slowdown, xgravity, ygravity, xoffset, yoffset;
	int hueVar = 0, sizeVar = 0;
	VALUE fadesize = Qfalse;
	
	rb_get_args(argc, argv, "iidddddioi|iiio",
	            &maxParticles, &hue, &slowdown,
	            &xgravity, &ygravity, &xoffset, &yoffset,
	            &opacityVar, &filenamesObj, &opacity,
	            &zOffset, &hueVar, &sizeVar, &fadesize RB_ARG_END);
	
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
	
	ps->setParameters(maxParticles, hue, (float)slowdown,
	                  (float)xgravity, (float)ygravity, (float)xoffset, (float)yoffset,
	                  opacityVar, filenames, opacity, zOffset,
	                  hueVar, sizeVar, RTEST(fadesize));
	
	GFX_UNLOCK;
	return self;
}

RB_METHOD(particleSystemUpdate) {
	GFX_LOCK;
	
	ParticleSystem *ps = getPrivateData<ParticleSystem>(self);
	ps->update();
	
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

void particleSystemBindingInit() {
	VALUE klass = rb_define_class("ParticleSystem", rb_cObject);
#if RAPI_FULL > 187
	rb_define_alloc_func(klass, classAllocate<&ParticleSystemType>);
#else
	rb_define_alloc_func(klass, ParticleSystemAllocate);
#endif
	
	disposableBindingInit<ParticleSystem>(klass);

	_rb_define_method(klass, "initialize", particleSystemInitialize);
	_rb_define_method(klass, "set_parameters", particleSystemSetParameters);
	_rb_define_method(klass, "update", particleSystemUpdate);
	_rb_define_method(klass, "dispose", particleSystemDispose);
}
