/*****************************************************************************
The Dark Mod GPL Source Code

This file is part of the The Dark Mod Source Code, originally based
on the Doom 3 GPL Source Code as published in 2011.

The Dark Mod Source Code is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version. For details, see LICENSE.TXT.

Project: The Dark Mod (http://www.thedarkmod.com/)

******************************************************************************/
#pragma once
#include "../tr_local.h"
#include "DrawBatchExecutor.h"

class GLSLProgram;
class TiledCustomMipmapStage;

class InteractionStage {
public:
	InteractionStage();

	void Init();
	void Shutdown();

	void DrawInteractions( const viewDef_t *viewDef, viewLight_t *vLight, const drawSurf_t *interactionSurfs, const TiledCustomMipmapStage *stencilShadowMipmaps );

private:
	struct Uniforms;
	enum TextureUnits;

	GLSLProgram *stencilInteractionShader = nullptr;
	GLSLProgram *shadowMapInteractionShader = nullptr;
	GLSLProgram *ambientInteractionShader = nullptr;
	GLSLProgram *interactionShader = nullptr;

	Uniforms *uniforms = nullptr;
	idList<idVec2> poissonSamples;

	const viewDef_t *viewDef = nullptr;
	viewLight_t *vLight = nullptr;

	void LoadInteractionShader(GLSLProgram *shader, const idStr &baseName);
	void BindShadowTexture( const TiledCustomMipmapStage *stencilShadowMipmaps );
	void ChooseInteractionProgram( viewLight_t *vLight, bool translucent );
	void ProcessSingleSurface( viewLight_t *vLight, const shaderStage_t *lightStage, const drawSurf_t *surf );
	void PrepareDrawCommand( drawInteraction_t * inter );

	void PreparePoissonSamples();
};
