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

ParticleSystem::ParticleSystem(Viewport *viewport)
	: m_viewport(viewport),
	  m_maxParticles(0),
	  m_hue(0),
	  m_slowdown(1.0f),
	  m_xgravity(0.0f),
	  m_ygravity(0.0f),
	  m_xoffset(0.0f),
	  m_yoffset(0.0f),
	  m_opacityVar(1),
	  m_hueVar(0),
	  m_sizeVar(0),
	  m_fadesize(false),
	  m_initialOpacity(255),
	  m_zoffset(-1),
	  m_startingX(0),
	  m_startingY(0),
	  m_screenX(0),
	  m_screenY(0),
	  m_realX(0),
	  m_realY(0),
	  m_offsetX(0),
	  m_offsetY(0),
	  m_bmwidth(32),
	  m_bmheight(32),
	  m_disposed(false)
{
	g_rng.seed(std::random_device{}());
}

ParticleSystem::~ParticleSystem() {
	dispose();
}

void ParticleSystem::setParameters(int maxParticles, int hue, float slowdown,
                                   float xgravity, float ygravity, float xoffset, float yoffset,
                                   int opacityVar, const std::vector<std::string> &filenames,
                                   int opacity, int zOffset, int hueVar, 
                                   int sizeVar, bool fadesize) {
	m_maxParticles = maxParticles;
	m_hue = hue;
	m_slowdown = slowdown;
	m_xgravity = xgravity;
	m_ygravity = ygravity;
	m_xoffset = xoffset;
	m_yoffset = yoffset;
	m_opacityVar = opacityVar;
	m_hueVar = hueVar;
	m_sizeVar = sizeVar;
	m_fadesize = fadesize;
	m_initialOpacity = opacity;
	m_zoffset = zOffset;
	m_filenames = filenames;

	initParticles(filenames, opacity, zOffset);
}

Bitmap *ParticleSystem::loadBitmap(const std::string &filename, int hue) {
	BitmapKey key(filename, hue);
	auto it = m_bitmaps.find(key);
	
	if (it != m_bitmaps.end() && it->second && !it->second->isDisposed()) {
		return it->second;
	}

	// Load bitmap
	std::string fullPath = filename;
	
	try {
		Bitmap *bitmap = new Bitmap(fullPath.c_str());
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

void ParticleSystem::buildParticleSpaces() {
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

void ParticleSystem::initParticles(const std::vector<std::string> &filenames, int opacity, int zOffset) {
	// Clean up existing particles
	for (auto sprite : m_particles) {
		if (sprite) {
			sprite->dispose();
			delete sprite;
		}
	}
	m_particles.clear();
	m_particlesStartX.clear();
	m_particlesStartY.clear();
	m_particleX.clear();
	m_particleY.clear();
	m_opacity.clear();

	buildParticleSpaces();

	m_startingX = m_screenX + m_xoffset;
	m_startingY = m_screenY + m_yoffset;

	m_bmwidth = 32;
	m_bmheight = 32;

	double innerThreshold = m_maxParticles * 0.9;

	for (int i = 0; i < m_maxParticles; ++i) {
		bool useInner = i >= innerThreshold;
		auto startPos = sampleFromSpace(useInner);

		m_particlesStartX.push_back(startPos.first);
		m_particlesStartY.push_back(startPos.second);
		m_particleX.push_back(-m_xoffset);
		m_particleY.push_back(-m_yoffset);

		Sprite *particle = new Sprite(m_viewport);
		m_particles.push_back(particle);

		if (!filenames.empty()) {
			const std::string &filename = filenames[randomInt(filenames.size())];
			int particleHue = m_hue + randomFloatRange(-m_hueVar, m_hueVar);
			Bitmap *bitmap = loadBitmap(filename, particleHue);
			
			if (bitmap) {
				particle->setBitmap(bitmap);
				if (i == 0) {
					m_bmwidth = bitmap->width();
					m_bmheight = bitmap->height();
				}
			}
		}

		particle->setOX(m_bmwidth / 2);
		particle->setOY(m_bmheight / 2);
		particle->setY(m_startingY + m_particlesStartX[i]);
		particle->setX(m_startingX + m_particlesStartY[i]);
		particle->setZ(m_zoffset);
		
		int zoomVar = m_sizeVar > 0 ? randomFloatRange(-m_sizeVar, m_sizeVar) : 0;
		particle->setZoomX(1.0f + zoomVar / 100.0f);
		particle->setZoomY(1.0f + zoomVar / 100.0f);

		int particleOpacity = randomInt(opacity);
		m_opacity.push_back(particleOpacity);
		particle->setOpacity(particleOpacity);

		if (m_fadesize) {
			float opacityFactor = (particleOpacity / 255.0f + 0.2f);
			if (opacityFactor > 1.0f) opacityFactor = 1.0f;
			if (opacityFactor < 0.0f) opacityFactor = 0.0f;
			particle->setZoomX((1.0f + zoomVar / 100.0f) * opacityFactor);
			particle->setZoomY((1.0f + zoomVar / 100.0f) * opacityFactor);
		}
	}
}

void ParticleSystem::update() {
	/* if (m_viewport && (m_viewport->getRect().x >= 640 || m_viewport->getRect().y >= 480)) {
		return;
	} */

	m_startingX = m_screenX + m_xoffset;
	m_startingY = m_screenY + m_yoffset;

	m_offsetX = 0;
	m_offsetY = 0;

	int randN = randomInt(m_opacityVar);
	int particleZ = m_zoffset;
	float xSum = m_startingX + m_xoffset;
	float ySum = m_startingY + m_yoffset;
	float xOff = m_xgravity * m_slowdown;
	float yOff = -m_ygravity * m_slowdown;
	double iThresh = m_maxParticles * 0.9;

	static const int OFFSETS[] = {-1, 1};

	for (int i = 0; i < m_maxParticles; ++i) {
		Sprite *particle = m_particles[i];
		
		if (!particle) continue;

		int particleZOffset = (i >= iThresh) ? 15 : -15;
		particle->setZ(particleZ + particleZOffset);

		if (m_opacity[i] <= 0) {
			m_opacity[i] = 255;
			bool useInner = i >= iThresh;
			auto startPos = sampleFromSpace(useInner);
			m_particlesStartX[i] = startPos.first;
			m_particlesStartY[i] = startPos.second;
			particle->setX(xSum);
			particle->setY(ySum);
			m_particleX[i] = 0.0f;
			m_particleY[i] = 0.0f;
			particle->setZoomX(1.0f);
			particle->setZoomY(1.0f);
			continue;
		}

		int randI = ((randN + i) % m_opacityVar);

		float xo = xOff * OFFSETS[randI & 1];
		float yo = yOff;
		
		m_particleX[i] += xo;
		m_particleY[i] += yo;
		m_particleX[i] -= m_offsetX;
		m_particleY[i] -= m_offsetY;

		particle->setX(m_particleX[i] + xSum + m_particlesStartX[i]);
		particle->setY(m_particleY[i] + ySum + m_particlesStartY[i]);

		m_opacity[i] = m_opacity[i] - (randI * m_slowdown);
		
		float zoom = (m_opacity[i] / 255.0f + 0.2f);
		if (zoom > 1.0f) zoom = 1.0f;
		if (zoom < 0.0f) zoom = 0.0f;
		
		particle->setZoomX(zoom);
		particle->setZoomY(zoom);
		particle->setOpacity(m_opacity[i]);
	}
}

void ParticleSystem::setScreenPosition(int x, int y)
{
	m_screenX = x;
	m_screenY = y;
}

void ParticleSystem::setZ(int z)
{
	m_zoffset = z;
}

void ParticleSystem::dispose() {
	for (auto sprite : m_particles) {
		if (sprite) {
			if (!sprite->isDisposed())
				sprite->dispose();
			delete sprite;
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
