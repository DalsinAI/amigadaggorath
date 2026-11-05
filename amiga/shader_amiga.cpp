/*
 * The NTSC artifact-colour shader (src/shader.cpp) needs a GPU's frame
 * buffers and GLSL. On the Amiga there is none, so the shader manager
 * says it could not start, and the game turns artifact colours off and
 * draws in plain black and white, as upstream does on such a machine.
 * The AmiCradle Team. Like the game, under Douglas J. Morgan's licence.
 */
#include "shader.h"

ShaderManager shaderMgr;

ShaderManager::ShaderManager()
    : m_fbo(0), m_renderTexture(0), m_texWidth(0), m_texHeight(0),
      m_artifactProgram(0), m_vertexShader(0), m_fragmentShader(0),
      m_textureLoc(-1), m_resolutionLoc(-1), m_phaseFlipLoc(-1),
      m_quadVBO(0), m_initialized(false) {}

ShaderManager::~ShaderManager() {}

bool ShaderManager::init() { return false; }
void ShaderManager::shutdown() {}
bool ShaderManager::createRenderTarget(int, int) { return false; }
void ShaderManager::beginRenderToTexture() {}
void ShaderManager::endRenderToTexture() {}
bool ShaderManager::loadArtifactShader() { return false; }
void ShaderManager::applyArtifactEffect(bool) {}
bool ShaderManager::compileShader(GLuint, const char *) { return false; }
bool ShaderManager::linkProgram(GLuint) { return false; }
void ShaderManager::createFullscreenQuad() {}
