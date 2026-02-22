/*
** particlesystem.h
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

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <string>
#include <map>
#include <vector>
#include <utility>
#include "util/disposable.h"

class Sprite;
class Bitmap;
class Viewport;

typedef std::pair<std::string, int> BitmapKey;

class ParticleSystem
{
public:
	ParticleSystem(Viewport *viewport = 0);
	~ParticleSystem();

	void setParameters(int maxParticles, int hue, float slowdown,
	                   float xgravity, float ygravity, float xoffset, float yoffset,
	                   int opacityVar, const std::vector<std::string> &filenames,
	                   int opacity, int zOffset = -1, int hueVar = 0, 
	                   int sizeVar = 0, bool fadesize = false);

	void update();
	void setScreenPosition(int x, int y);
	void setZ(int z);
	void dispose();
	bool isDisposed() const;

private:
	Viewport *m_viewport;

	// Parameters
	int m_maxParticles;
	int m_hue;
	float m_slowdown;
	float m_xgravity;
	float m_ygravity;
	float m_xoffset;
	float m_yoffset;
	int m_opacityVar;
	int m_hueVar;
	int m_sizeVar;
	bool m_fadesize;
	int m_initialOpacity;
	int m_zoffset;

	// Particle data
	std::vector<Sprite*> m_particles;
	std::vector<int> m_particlesStartX;
	std::vector<int> m_particlesStartY;
	std::vector<float> m_particleX;
	std::vector<float> m_particleY;
	std::vector<int> m_opacity;

	// State
	float m_startingX;
	float m_startingY;
	float m_screenX;
	float m_screenY;
	int m_realX;
	int m_realY;
	int m_offsetX;
	int m_offsetY;

	// Bitmap cache
	std::map<BitmapKey, Bitmap*> m_bitmaps;

	// Particle sampling
	std::vector<std::pair<int, int>> m_innerSpace;
	std::vector<std::pair<int, int>> m_outlineSpace;

	// Filenames
	std::vector<std::string> m_filenames;
	int m_bmwidth;
	int m_bmheight;
	bool m_disposed;

	Bitmap *loadBitmap(const std::string &filename, int hue);
	void initParticles(const std::vector<std::string> &filenames, int opacity, int zOffset);
	void buildParticleSpaces();
	std::pair<int, int> sampleFromSpace(bool useInner);

	const char *klassName() const { return "ParticleSystem"; }
};

#endif // PARTICLESYSTEM_H
