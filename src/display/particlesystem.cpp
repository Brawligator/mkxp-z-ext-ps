/*
** particlesystem.cpp
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

#include "particlesystem.h"
#include "sprite.h"
#include "bitmap.h"
#include "viewport.h"
#include "sharedstate.h"
#include "filesystem/filesystem.h"
#include "util/exception.h"
#include "util/util.h"
#include <cmath>
#include <random>
#include <cstring>
#include "etc-internal.h"

static std::mt19937 g_rng;

static int randomInt(int max) {
	if (max <= 0) return 0;
	std::uniform_int_distribution<int> dist(0, max - 1);
	return dist(g_rng);
}

static float randomFloat(float max) {
	if (max <= 0.0f) return 0.0f;
	std::uniform_real_distribution<float> dist(0.0f, max);
	return dist(g_rng);
}

static float randomFloatRange(float minVal, float maxVal) {
	std::uniform_real_distribution<float> dist(minVal, maxVal);
	return dist(g_rng);
}


Particle::Particle(Viewport *viewport)
	: Sprite(viewport),
	  velocity(0, 0),
	  radial_velocity(0, 0, 0),
	  life(0)
{
}

Vec2 Particle::getVelocity() const {
	return velocity;
}

Vec3 Particle::getRadialVelocity() const {
	return radial_velocity;
}

void Particle::setVelocity(Vec2 value) {
	velocity = value;
}

void Particle::setRadialVelocity(Vec3 value) {
	radial_velocity = value;
}

ParticleSystem::ParticleSystem(Viewport *viewport)
	: m_viewport(viewport),
	  m_maxParticles(0),
	  m_hue(0),
	  m_slowdown(1.0f),
	  m_xgravity(0.0f),
	  m_ygravity(0.0f),
	  m_opacityVar(1),
	  m_hueVar(0),
	  m_sizeVar(0),
	  m_fadesize(false),
	  m_initialOpacity(255),
	  m_baseZoom(1.0f),
	  m_velocity(0, 0),	  m_acceleration(0, 0),
	  m_lifeTime(1.0f),	  m_zoffset(-1),
	  m_screenX(0),
	  m_screenY(0),
	  m_disposed(false)
{
	g_rng.seed(std::random_device{}());
}

ParticleSystem::~ParticleSystem() {
	dispose();
}

Bitmap *ParticleSystem::loadBitmap(const std::string &filename, int hue) {
	BitmapKey key(filename, hue);
	auto it = m_bitmaps.find(key);
	
	if (it != m_bitmaps.end() && it->second && !it->second->isDisposed()) {
		return it->second;
	}
	
	try {
		Bitmap *bitmap = new Bitmap(filename.c_str());
		bitmap->hueChange(hue);
		m_bitmaps[key] = bitmap;
		return bitmap;
	} catch (const std::exception &e) {
		return nullptr;
	}
}

std::pair<int, int> ParticleSystem::sampleFromSpace(bool useInner) {
	const auto &space = useInner ? m_innerSpace : m_outlineSpace;
	
	if (space.empty()) {
		return std::make_pair(0, 0);
	}
	
	int idx = randomInt(space.size());
	return space[idx];
}

void ParticleSystem::buildDefaultSpaces() {
	const int DIAMETER = 32;
	const int RADIUS = DIAMETER / 2;
	const float RADIUS_SQ = RADIUS * RADIUS;

	m_innerSpace.clear();
	m_outlineSpace.clear();

	// Generate coordinates within a circle of diameter 32
	for (int x = -RADIUS; x < RADIUS; ++x) {
		for (int y = -RADIUS; y < RADIUS; ++y) {
			float distSq = (float)(x * x + y * y);
			if (distSq <= RADIUS_SQ) {
				m_innerSpace.push_back(std::make_pair(x, y));
			}
		}
	}

	// If no inner space, use center as fallback
	if (m_innerSpace.empty()) {
		m_innerSpace.push_back(std::make_pair(0, 0));
	}

	// Outline space uses the same circle for now
	m_outlineSpace = m_innerSpace;
}

void ParticleSystem::refresh() {

	// Clean up existing particles
	for (auto particle : m_particles) {
		if (particle) {
			particle->dispose();
			delete particle;
		}
	}
	m_particles.clear();

	if (m_innerSpace.empty() && m_outlineSpace.empty()) {
		buildDefaultSpaces();
	}

	int bmwidth = 0;
	int bmheight = 0;

	double innerThreshold = m_maxParticles * 0.9;

	for (int i = 0; i < m_maxParticles; ++i) {
		bool useInner = i >= innerThreshold;
		auto startPos = sampleFromSpace(useInner);

		Particle *particle = new Particle(m_viewport);
		m_particles.push_back(particle);

		if (!m_filenames.empty()) {
			const std::string &filename = m_filenames[randomInt(m_filenames.size())];
			int particleHue = m_hue + randomFloatRange(-m_hueVar, m_hueVar);
			Bitmap *bitmap = loadBitmap(filename, particleHue);
			
			if (bitmap) {
				particle->setBitmap(bitmap);

				particle->setOX(bitmap->width() / 2);
				particle->setOY(bitmap->height() / 2);
			}
		}

		particle->setX(m_screenX + startPos.first);
		particle->setY(m_screenY + startPos.second);
		particle->setZ(m_zoffset);
		particle->setVelocity(m_velocity);

		float startLifetime = randomFloatRange(1f);
		int particleOpacity = (int)(m_initialOpacity * (1 - startLifetime));
		particle->setOpacity(particleOpacity);
		
		particle->setZoomX(m_baseZoom * (1.0f - startLifetime));
		particle->setZoomY(m_baseZoom * (1.0f - startLifetime));
	}
}

void ParticleSystem::update(float deltaTime) {
	/* if (m_viewport && (m_viewport->getRect().x >= 640 || m_viewport->getRect().y >= 480)) {
		return;
	} */

	// int randN = randomInt(m_opacityVar);
	double iThresh = m_maxParticles * 0.9;

	static const int OFFSETS[] = {-1, 1};

	for (int i = 0; i < m_maxParticles; ++i) {
		if (i >= m_particles.size()) break;

		Particle *particle = m_particles[i];
		
		if (!particle) continue;

		Vec2 newVelocity = Vec2(particle->getVelocity().x + m_acceleration.x * deltaTime,
		                       particle->getVelocity().y + m_acceleration.y * deltaTime);
		particle->setVelocity(newVelocity);

		particle->setX(particle->getX() + (particle->getVelocity().x /* + cos(particle->getRadialVelocity().x + sin(particle->getRadialVelocity().z)) */) * deltaTime);
		particle->setY(particle->getY() + (particle->getVelocity().y /* + sin(particle->getRadialVelocity().y + cos(particle->getRadialVelocity().z)) */) * deltaTime);

		int particleZOffset = (i >= iThresh) ? 15 : -15;
		particle->setZ(m_zoffset + particleZOffset);

		// int randI = ((randN + i) % 2);

		/* float xo = xOff * OFFSETS[randI & 1]; */

		int newOpacity = particle->getOpacity() - (int)(deltaTime * 255.0f / m_lifeTime);

		if (newOpacity <= 0) {
			particle->setOpacity(m_initialOpacity);
			bool useInner = i >= iThresh;
			auto startPos = sampleFromSpace(useInner);
			particle->setX(m_screenX + startPos.first);
			particle->setY(m_screenY + startPos.second);
			particle->setZoomX(m_baseZoom);
			particle->setZoomY(m_baseZoom);
			particle->setVelocity(m_velocity);
			continue;
		}

		float zoom = (newOpacity / 255.0f) * m_baseZoom;
		if (zoom < 0.0f) zoom = 0.0f;
		
		particle->setZoomX(zoom);
		particle->setZoomY(zoom);
		particle->setOpacity(newOpacity);
	}
}

void ParticleSystem::setScreenPosition(int x, int y)
{
	float oldX = m_screenX;
	float oldY = m_screenY;

	m_screenX = x;
	m_screenY = y;

	for (auto particle : m_particles) {
		if (particle) {
			particle->setX(particle->getX() + (m_screenX - oldX));
			particle->setY(particle->getY() + (m_screenY - oldY));
		}
	}
}

void ParticleSystem::setZ(int z)
{
	m_zoffset = z;
}

void ParticleSystem::dispose() {
	for (auto particle : m_particles) {
		if (particle) {
			if (!particle->isDisposed())
				particle->dispose();
			delete particle;
		}
	}
	m_particles.clear();

	for (auto &pair : m_bitmaps) {
		if (pair.second && !pair.second->isDisposed()) {
			pair.second->dispose();
		}
	}
	m_bitmaps.clear();

	m_disposed = true;
}

bool ParticleSystem::isDisposed() const {
	return m_disposed;
}
// Individual setters
void ParticleSystem::setMaxParticles(int maxParticles) {
	m_maxParticles = maxParticles;
}

void ParticleSystem::setHue(int hue) {
	m_hue = hue;
}

void ParticleSystem::setSlowdown(float slowdown) {
	m_slowdown = slowdown;
}

void ParticleSystem::setXGravity(float xgravity) {
	m_xgravity = xgravity;
}

void ParticleSystem::setYGravity(float ygravity) {
	m_ygravity = ygravity;
}

void ParticleSystem::setOpacityVar(int opacityVar) {
	m_opacityVar = opacityVar;
}

void ParticleSystem::setHueVar(int hueVar) {
	m_hueVar = hueVar;
}

void ParticleSystem::setSizeVar(int sizeVar) {
	m_sizeVar = sizeVar;
}

void ParticleSystem::setFadeSize(bool fadesize) {
	m_fadesize = fadesize;
}

void ParticleSystem::setInitialOpacity(int opacity) {
	m_initialOpacity = opacity;
}

void ParticleSystem::setBaseZoom(float baseZoom) {
	m_baseZoom = baseZoom;
}

void ParticleSystem::setLifeTime(float lifeTime) {
	m_lifeTime = lifeTime;
}

void ParticleSystem::setZOffset(int zOffset) {
	m_zoffset = zOffset;
}

void ParticleSystem::setFilenames(const std::vector<std::string> &filenames) {
	m_filenames = filenames;
}

void ParticleSystem::setSpawnSpaces(const std::vector<std::pair<int, int>> &innerSpace, const std::vector<std::pair<int, int>> &outlineSpace) {
	m_innerSpace = innerSpace;
	m_outlineSpace = outlineSpace;
}

void ParticleSystem::setVelocity(Vec2 velocity) {
	m_velocity = velocity;
}

void ParticleSystem::setAcceleration(Vec2 acceleration) {
	m_acceleration = acceleration;
}

// Individual getters
int ParticleSystem::getMaxParticles() const {
	return m_maxParticles;
}

int ParticleSystem::getHue() const {
	return m_hue;
}

float ParticleSystem::getSlowdown() const {
	return m_slowdown;
}

float ParticleSystem::getXGravity() const {
	return m_xgravity;
}

float ParticleSystem::getYGravity() const {
	return m_ygravity;
}

int ParticleSystem::getOpacityVar() const {
	return m_opacityVar;
}

int ParticleSystem::getHueVar() const {
	return m_hueVar;
}

int ParticleSystem::getSizeVar() const {
	return m_sizeVar;
}

bool ParticleSystem::getFadeSize() const {
	return m_fadesize;
}

int ParticleSystem::getInitialOpacity() const {
	return m_initialOpacity;
}

float ParticleSystem::getBaseZoom() const {
	return m_baseZoom;
}

float ParticleSystem::getLifeTime() const {
	return m_lifeTime;
}

int ParticleSystem::getZOffset() const {
	return m_zoffset;
}

const std::vector<std::string> &ParticleSystem::getFilenames() const {
	return m_filenames;
}

const std::vector<std::pair<int, int>> &ParticleSystem::getSpawnSpace() const {
	return m_innerSpace;
}

Vec2 ParticleSystem::getVelocity() const {
	return m_velocity;
}

Vec2 ParticleSystem::getAcceleration() const {
	return m_acceleration;
}