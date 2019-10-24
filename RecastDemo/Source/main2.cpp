//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <cstdio>
#define _USE_MATH_DEFINES
#include <cmath>

#include "SDL.h"
#include "SDL_opengl.h"
#ifdef __APPLE__
#	include <OpenGL/glu.h>
#else
#	include <GL/glu.h>
#endif

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "imgui.h"
#include "imguiRenderGL.h"

#include "Recast.h"
#include "RecastDebugDraw.h"
#include "InputGeom.h"
#include "TestCase.h"
#include "Filelist.h"
#include "Sample_SoloMesh.h"
#include "Sample_TileMesh.h"
#include "Sample_TempObstacles.h"
#include "Sample_Debug.h"


using std::string;
using std::vector;


int main_ori(int argc, char** argv);


struct MyParams 
{
	int m_isTitleMesh;
	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	int m_partitionType;

	std::map<string, string> mKVs;

	string& trim(string &s)
	{
		if (s.empty())
		{
			return s;
		}
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" \r\n") + 1);
		return s;
	}

	bool read(const std::string & name, int & val)
	{
		if (mKVs.find(name) != mKVs.end())
		{
			val = atoi(mKVs[name].c_str());
			printf("read %s=%d\n", name.c_str(), val);
			return true;
		}
		return false;
	}

	bool read(const std::string & name, float & val)
	{
		if (mKVs.find(name) != mKVs.end())
		{
			val = (float)atof(mKVs[name].c_str());
			printf("read %s=%f\n", name.c_str(), val);
			return true;
		}
		return false;
	}

	bool LoadSettings(const char * filePath)
	{
		using namespace std;
		ifstream  is(filePath);
		string line;
	
		while (getline(is, line))
		{
			if (line[0] == '#')
				continue;
			size_t pos = line.find_first_of('=');
			if (pos == string::npos || pos == 0 || pos == line.size() - 1)
				continue;

			string key = line.substr(0, pos);
			string val = line.substr(pos + 1);
			key = trim(key);
			val = trim(val);
			if (key.empty() || val.empty())
				continue;

			mKVs[key] = val;
		}
		read("isTitleMesh", m_isTitleMesh);
		read("cellSize", m_cellSize);
		read("cellHeight", m_cellHeight);
		read("agentHeight", m_agentHeight);
		read("agentRadius", m_agentRadius);
		read("agentMaxClimb", m_agentMaxClimb);
		read("agentMaxSlope", m_agentMaxSlope);
		read("regionMinSize", m_regionMinSize);
		read("regionMergeSize", m_regionMergeSize);
		read("edgeMaxLen", m_edgeMaxLen);
		read("edgeMaxError", m_edgeMaxError);
		read("vertsPerPoly", m_vertsPerPoly);
		read("detailSampleDist", m_detailSampleDist);
		read("detailSampleMaxError", m_detailSampleMaxError);
		read("partitionType", m_partitionType);
		return true;
	}
};

class MySoloMesh : public Sample_SoloMesh
{
public:
	virtual void ApplyMyParam(const MyParams & param)
	{
		m_cellSize = param.m_cellSize;
		m_cellHeight = param.m_cellHeight;
		m_agentHeight = param.m_agentHeight;
		m_agentRadius = param.m_agentRadius;
		m_agentMaxClimb = param.m_agentMaxClimb;
		m_agentMaxSlope = param.m_agentMaxSlope;
		m_regionMinSize = param.m_regionMinSize;
		m_regionMergeSize = param.m_regionMergeSize;
		m_edgeMaxLen = param.m_edgeMaxLen;
		m_edgeMaxError = param.m_edgeMaxError;
		m_vertsPerPoly = param.m_vertsPerPoly;
		m_detailSampleDist = param.m_detailSampleDist;
		m_detailSampleMaxError = param.m_detailSampleMaxError;
		m_partitionType = param.m_partitionType;
	}

};

inline unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}
class MyTitleMesh : public Sample_TileMesh
{

public:
	virtual void ApplyMyParam(const MyParams & param)
	{
		m_cellSize = param.m_cellSize;
		m_cellHeight = param.m_cellHeight;
		m_agentHeight = param.m_agentHeight;
		m_agentRadius = param.m_agentRadius;
		m_agentMaxClimb = param.m_agentMaxClimb;
		m_agentMaxSlope = param.m_agentMaxSlope;
		m_regionMinSize = param.m_regionMinSize;
		m_regionMergeSize = param.m_regionMergeSize;
		m_edgeMaxLen = param.m_edgeMaxLen;
		m_edgeMaxError = param.m_edgeMaxError;
		m_vertsPerPoly = param.m_vertsPerPoly;
		m_detailSampleDist = param.m_detailSampleDist;
		m_detailSampleMaxError = param.m_detailSampleMaxError;
		m_partitionType = param.m_partitionType;

		m_buildAll = true;
		int gw = 0, gh = 0;
		const float* bmin = m_geom->getNavMeshBoundsMin();
		const float* bmax = m_geom->getNavMeshBoundsMax();
		rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
		const int ts = (int)m_tileSize;
		const int tw = (gw + ts - 1) / ts;
		const int th = (gh + ts - 1) / ts;
		

		// Max tiles and max polys affect how the tile IDs are caculated.
		// There are 22 bits available for identifying a tile and a polygon.
		int tileBits = rcMin((int)ilog2(nextPow2(tw*th)), 14);
		if (tileBits > 14) tileBits = 14;
		int polyBits = 22 - tileBits;
		m_maxTiles = 1 << tileBits;
		m_maxPolysPerTile = 1 << polyBits;
	}

};


int main(int argc, char** argv)
{
	if(argc<3)
	{
		printf("USAGE: %s inputObjFilePath  outputNavFilePath [settingFile]",argv[0]);
		return main_ori(argc,argv);
	}

	BuildContext ctx;
	InputGeom* geom = new InputGeom();

	if(!geom->load(&ctx, argv[1]))
	{
		printf("Failed to load objFile [%s]",argv[1]);
		return -1;
	}

	Sample * sample = 0;
	if (argc > 3)
	{
		MyParams parms;
		if (!parms.LoadSettings(argv[3]))
		{
			printf("Failed to load setting [%s]", argv[1]);
			return -1;
		}
		if (parms.m_isTitleMesh)
		{
			MyTitleMesh * p = new MyTitleMesh();
			p->setContext(&ctx);
			p->handleMeshChanged(geom);
			p->ApplyMyParam(parms);
			sample = p;
		}
		else
		{ 
			MySoloMesh *p = new MySoloMesh();
			p->setContext(&ctx);
			p->handleMeshChanged(geom);
			p->ApplyMyParam(parms);
			sample = p;
		}
	}
	else
	{
		sample = new MySoloMesh();
		sample->setContext(&ctx);
		sample->handleMeshChanged(geom);
	}

	sample->handleBuild();
	sample->saveAll(argv[2], sample->getNavMesh());

	delete sample;
	delete geom;

	return 0;
}
