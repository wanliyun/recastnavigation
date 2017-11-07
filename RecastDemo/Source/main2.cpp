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

class MyMesh : public Sample_SoloMesh
{
public:
	bool LoadSettings(const char * filePath)
	{
		using namespace std;
		ifstream  is(filePath);
		string line;
		char buf[256];
		while(getline(is, line))
		{
			if(line[0] == '#')
				continue;
			if(line[0] == 's')
			{
				sscanf(line.c_str(), "%s %f %f %f %f %f %f %f %f %f %f %f %f %f %d",buf,
					&m_cellSize,
					&m_cellHeight,
					&m_agentHeight,
					&m_agentRadius,
					&m_agentMaxClimb,
					&m_agentMaxSlope,
					&m_regionMinSize,
					&m_regionMergeSize,
					&m_edgeMaxLen,
					&m_edgeMaxError,
					&m_vertsPerPoly,
					&m_detailSampleDist,
					&m_detailSampleMaxError,
					&m_partitionType);
					break;
			}
		}
		return true;
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

	MyMesh * sample = new MyMesh();

	sample->setContext(&ctx);
	sample->handleMeshChanged(geom);
	sample->m_navDataSavePath = argv[2];

	if(argc > 3)
	{
		if(!sample->LoadSettings(argv[3]))
		{
			printf("Failed to load setting [%s]",argv[1]);
			return -1;
		}
	}

	sample->handleBuild();

	delete sample;
	delete geom;

	return 0;
}
