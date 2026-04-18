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
#include "util.h"
#include "etc-internal.h"
#include "sprite.h"

class Bitmap;
class Viewport;

typedef std::pair<std::string, int> BitmapKey;

class Particle : public Sprite
{
public:
	Particle(Viewport *viewport = 0);
	DECL_ATTR(Velocity, Vec2)
	DECL_ATTR(RadialVelocity, Vec3)
	DECL_ATTR(BasePosition, Vec2)
	DECL_ATTR(Life, float)

private:
	Vec2 velocity;
	Vec2 base_position;
	float life;
};

class ParticleSystem
{
public:
	ParticleSystem(Viewport *viewport = 0);
	~ParticleSystem();

	// Individual setters
	void setMaxParticles(int maxParticles);
	void setHue(int hue);
	void setHueVar(int hueVar);
	void setSizeVar(int sizeVar);
	void setInitialOpacity(int opacity);
	void setBaseZoom(float baseZoom);
	void setLifeTime(float lifeTime);
	void setZOffset(int zOffset);
	void setFilenames(const std::vector<std::string> &filenames);
	void setSpawnSpaces(const std::vector<std::pair<int, int>> &spawnSpace, const std::vector<std::pair<int, int>> &outlineSpace);
	void setVelocity(Vec2 velocity);
	void setRandomVelocity(Vec2 velocity);
	void setRadialVelocity(Vec3 velocity);	
	void setAcceleration(Vec2 acceleration);

	// Individual getters
	int getMaxParticles() const;
	int getHue() const;
	int getHueVar() const;
	int getSizeVar() const;
	int getInitialOpacity() const;
	float getBaseZoom() const;
	float getLifeTime() const;
	int getZOffset() const;
	const std::vector<std::string> &getFilenames() const;
	const std::vector<std::pair<int, int>> &getSpawnSpace() const;
	Vec2 getVelocity() const;
	Vec2 getRandomVelocity() const;
	Vec3 getRadialVelocity() const;
	Vec2 getAcceleration() const;

	void update(float deltaTime);
	void setScreenPosition(int x, int y);
	void setZ(int z);
	void refresh();
	void stop();
	void dispose();
	bool isDisposed() const;

private:
	Viewport *m_viewport;

	// Parameters
	int m_maxParticles;
	int m_hue;
	int m_hueVar;
	int m_sizeVar;
	int m_initialOpacity;
	float m_baseZoom;
	int m_zoffset;
	bool m_stopped;

	Vec2 m_velocity;
	Vec2 m_acceleration;
	Vec3 m_radial_velocity;
	Vec2 m_random_velocity;
	float m_lifeTime;

	// Particle data
	std::vector<Particle*> m_particles;

	// State
	float m_screenX;
	float m_screenY;

	// Bitmap cache
	std::map<BitmapKey, Bitmap*> m_bitmaps;
	std::vector<std::string> m_filenames;

	// Particle sampling
	std::vector<std::pair<int, int>> m_innerSpace;
	std::vector<std::pair<int, int>> m_outlineSpace;

	bool m_disposed;

	Bitmap *loadBitmap(const std::string &filename, int hue);
	void buildDefaultSpaces();
	std::pair<int, int> sampleFromSpace(bool useInner);

	const char *klassName() const { return "ParticleSystem"; }
};

#endif // PARTICLESYSTEM_H
