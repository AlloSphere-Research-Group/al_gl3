#include "al/core/graphics/al_Graphics.hpp"
#include "al/core/graphics/al_Shapes.hpp"

#include <stdio.h>
#include <iostream>

namespace al {

Graphics::ColoringMode Graphics::mColoringMode = ColoringMode::UNIFORM;
ShaderProgram Graphics::mesh_shader;
ShaderProgram Graphics::color_shader;
ShaderProgram Graphics::tex_shader;
ShaderProgram Graphics::lighting_color_shader;
ShaderProgram Graphics::lighting_mesh_shader;
ShaderProgram Graphics::lighting_tex_shader;
ShaderProgram Graphics::lighting_material_shader;
int Graphics::color_location = 0;
int Graphics::color_tint_location = 0;
int Graphics::mesh_tint_location = 0;
int Graphics::tex_tint_location = 0;
int Graphics::lighting_color_location = 0;
int Graphics::lighting_color_tint_location = 0;
int Graphics::lighting_mesh_tint_location = 0;
int Graphics::lighting_tex_tint_location = 0;
int Graphics::lighting_material_tint_location = 0;
bool Graphics::mRenderModeChanged = true;
bool Graphics::mUniformChanged = true;

void Graphics::blendMode(BlendFunc src, BlendFunc dst, BlendEq eq) {
  glBlendEquation(eq);
  glBlendFunc(src, dst);
}

void Graphics::capability(Capability cap, bool v) {
  v ? enable(cap) : disable(cap);
}

void Graphics::blending(bool b) { capability(BLEND, b); }
void Graphics::colorMask(bool r, bool g, bool b, bool a) {
  glColorMask(r ? GL_TRUE : GL_FALSE, g ? GL_TRUE : GL_FALSE,
              b ? GL_TRUE : GL_FALSE, a ? GL_TRUE : GL_FALSE);
}
void Graphics::colorMask(bool b) { colorMask(b, b, b, b); }
void Graphics::depthMask(bool b) { glDepthMask(b ? GL_TRUE : GL_FALSE); }
void Graphics::depthTesting(bool b) { capability(DEPTH_TEST, b); }
void Graphics::scissorTest(bool b) { capability(SCISSOR_TEST, b); }
void Graphics::cullFace(bool b) { capability(CULL_FACE, b); }
void Graphics::cullFace(bool b, Face face) {
  capability(CULL_FACE, b);
  glCullFace(face);
}

// void Graphics::lineWidth(float v) { glLineWidth(v); }
void Graphics::pointSize(float v) { glPointSize(v); }
void Graphics::polygonMode(PolygonMode m, Face f) { glPolygonMode(f, m); }

void Graphics::scissor(int left, int bottom, int width, int height) {
  glScissor(left, bottom, width, height);
}

void Graphics::setClearColor(float r, float g, float b, float a) {
  mClearColor.set(r, g, b, a);
}

void Graphics::setClearColor(Color const& c) { mClearColor = c; }

void Graphics::clearColorBuffer(int drawbuffer) {
  glClearBufferfv(GL_COLOR, drawbuffer, mClearColor.components);
}

void Graphics::clearColorBuffer(float r, float g, float b, float a, int drawbuffer) {
  setClearColor(r, g, b, a);
  clearColorBuffer(drawbuffer);
}

void Graphics::setClearDepth(float d) { mClearDepth = d; }

void Graphics::clearDepth() { glClearBufferfv(GL_DEPTH, 0, &mClearDepth); }

void Graphics::clearDepth(float d) {
  setClearDepth(d);
  clearDepth();
}

void Graphics::clearBuffer(int drawbuffer) {
  clearColorBuffer(drawbuffer);
  clearDepth();
}

void Graphics::clearBuffer(float r, float g, float b, float a, float d, int drawbuffer) {
  clearColorBuffer(r, g, b, a, drawbuffer);
  clearDepth(d);
}

void Graphics::init() {
  static bool initialized = false;
  if (initialized) return;

  compileDefaultShader(color_shader, ShaderType::COLOR);
  compileDefaultShader(mesh_shader, ShaderType::MESH);
  compileDefaultShader(tex_shader, ShaderType::TEXTURE);
  compileDefaultShader(lighting_color_shader, ShaderType::LIGHTING_COLOR);
  compileDefaultShader(lighting_mesh_shader, ShaderType::LIGHTING_MESH);
  compileDefaultShader(lighting_tex_shader, ShaderType::LIGHTING_TEXTURE);
  compileDefaultShader(lighting_material_shader, ShaderType::LIGHTING_MATERIAL);

  color_location = color_shader.getUniformLocation("col0");
  color_tint_location = color_shader.getUniformLocation("tint");
  tex_tint_location = tex_shader.getUniformLocation("tint");
  mesh_tint_location = mesh_shader.getUniformLocation("tint");

  lighting_color_location = lighting_color_shader.getUniformLocation("col0");
  lighting_color_tint_location =
      lighting_color_shader.getUniformLocation("tint");
  lighting_mesh_tint_location = lighting_mesh_shader.getUniformLocation("tint");
  lighting_tex_tint_location = lighting_tex_shader.getUniformLocation("tint");
  lighting_material_tint_location =
      lighting_material_shader.getUniformLocation("tint");

  tex_shader.begin();
  tex_shader.uniform("tex0", 0);
  tex_shader.end();

  lighting_tex_shader.begin();
  lighting_tex_shader.uniform("tex0", 0);
  lighting_tex_shader.end();

  initialized = true;
}

void Graphics::quad(Texture& tex, float x, float y, float w, float h) {
  static Mesh m = []() {
    Mesh m{Mesh::TRIANGLE_STRIP};
    m.vertex(0, 0, 0);
    m.vertex(0, 0, 0);
    m.vertex(0, 0, 0);
    m.vertex(0, 0, 0);
    m.texCoord(0, 0);
    m.texCoord(1, 0);
    m.texCoord(0, 1);
    m.texCoord(1, 1);
    return m;
  }();

  auto& verts = m.vertices();
  verts[0].set(x, y, 0);
  verts[1].set(x + w, y, 0);
  verts[2].set(x, y + h, 0);
  verts[3].set(x + w, y + h, 0);

  tex.bind(0);
  texture();
  draw(m);
  tex.unbind(0);
}

void Graphics::quadViewport(Texture& tex, float x, float y, float w, float h) {
  pushCamera();
  camera(Viewpoint::IDENTITY);
  quad(tex, x, y, w, h);
  popCamera();
}

void Graphics::update() {
  if (mRenderModeChanged) {
    switch (mColoringMode) {
      case ColoringMode::UNIFORM:
        RenderManager::shader(mLightingEnabled ? lighting_color_shader : color_shader);
        break;
      case ColoringMode::MESH:
        RenderManager::shader(mLightingEnabled ? lighting_mesh_shader : mesh_shader);
        break;
      case ColoringMode::TEXTURE:
        RenderManager::shader(mLightingEnabled ? lighting_tex_shader : tex_shader);
        break;
      case ColoringMode::MATERIAL:
        RenderManager::shader(mLightingEnabled ? lighting_material_shader : color_shader);
        break;
      case ColoringMode::CUSTOM:
        // do nothing
        break;
    }
    mRenderModeChanged = false;
    mUniformChanged = true;
  }

  if (mUniformChanged) {
    auto& s = RenderManager::shader();
    switch (mColoringMode) {
      case ColoringMode::UNIFORM:
        if (mLightingEnabled) {
          send_uniforms(s, mLight);
          s.uniform4v(lighting_color_location, mColor.components);
          s.uniform4v(lighting_color_tint_location, mTint.components);
        } else {
          s.uniform4v(color_location, mColor.components);
          s.uniform4v(color_tint_location, mTint.components);
        }
        break;
      case ColoringMode::MESH:
        if (mLightingEnabled) {
          send_uniforms(s, mLight);
          s.uniform4v(lighting_mesh_tint_location, mTint.components);
        } else {
          s.uniform4v(mesh_tint_location, mTint.components);
        }
        break;
      case ColoringMode::TEXTURE:
        if (mLightingEnabled) {
          send_uniforms(s, mLight);
          s.uniform4v(lighting_tex_tint_location, mTint.components);
        } else {
          s.uniform4v(tex_tint_location, mTint.components);
        }
        break;
      case ColoringMode::MATERIAL:
        if (mLightingEnabled) {
          send_uniforms(s, mMaterial);
          send_uniforms(s, mLight);
          s.uniform4v(lighting_material_tint_location, mTint.components);
        } else {
          s.uniform4v(color_location, mColor.components);
          s.uniform4v(color_tint_location, mTint.components);
        }
        break;
      case ColoringMode::CUSTOM:
        // do nothing
       break;
    }
    mUniformChanged = false;
  }

  RenderManager::update();
}

}  // namespace al