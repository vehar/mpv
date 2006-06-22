/**
 * \file gl_common.c
 * \brief OpenGL helper functions used by vo_gl.c and vo_gl2.c
 *
 * Common OpenGL routines.
 * Copyleft (C) Reimar Döffinger <Reimar.Doeffinger@stud.uni-karlsruhe.de>, 2005
 * Licensend under the GNU GPL v2.
 * Special thanks go to the xine team and Matthias Hopf, whose video_out_opengl.c
 * gave me lots of good ideas.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "gl_common.h"

/**
 * \defgroup glextfunctions OpenGL extension functions
 *
 * the pointers to these functions are acquired when the OpenGL
 * context is created
 * \{
 */
void (APIENTRY *GenBuffers)(GLsizei, GLuint *);
void (APIENTRY *DeleteBuffers)(GLsizei, const GLuint *);
void (APIENTRY *BindBuffer)(GLenum, GLuint);
GLvoid* (APIENTRY *MapBuffer)(GLenum, GLenum); 
GLboolean (APIENTRY *UnmapBuffer)(GLenum);
void (APIENTRY *BufferData)(GLenum, intptr_t, const GLvoid *, GLenum);
void (APIENTRY *CombinerParameterfv)(GLenum, const GLfloat *);
void (APIENTRY *CombinerParameteri)(GLenum, GLint);
void (APIENTRY *CombinerInput)(GLenum, GLenum, GLenum, GLenum, GLenum,
                               GLenum);
void (APIENTRY *CombinerOutput)(GLenum, GLenum, GLenum, GLenum, GLenum,
                                GLenum, GLenum, GLboolean, GLboolean,
                                GLboolean);
void (APIENTRY *BeginFragmentShader)(void);
void (APIENTRY *EndFragmentShader)(void);
void (APIENTRY *SampleMap)(GLuint, GLuint, GLenum);
void (APIENTRY *ColorFragmentOp2)(GLenum, GLuint, GLuint, GLuint, GLuint,
                                  GLuint, GLuint, GLuint, GLuint, GLuint);
void (APIENTRY *ColorFragmentOp3)(GLenum, GLuint, GLuint, GLuint, GLuint,
                                  GLuint, GLuint, GLuint, GLuint, GLuint,
                                  GLuint, GLuint, GLuint);
void (APIENTRY *SetFragmentShaderConstant)(GLuint, const GLfloat *);
void (APIENTRY *ActiveTexture)(GLenum);
void (APIENTRY *BindTexture)(GLenum, GLuint);
void (APIENTRY *MultiTexCoord2f)(GLenum, GLfloat, GLfloat);
void (APIENTRY *GenPrograms)(GLsizei, GLuint *);
void (APIENTRY *DeletePrograms)(GLsizei, const GLuint *);
void (APIENTRY *BindProgram)(GLenum, GLuint);
void (APIENTRY *ProgramString)(GLenum, GLenum, GLsizei, const GLvoid *);
void (APIENTRY *GetProgramiv)(GLenum, GLenum, GLint *);
void (APIENTRY *ProgramEnvParameter4f)(GLenum, GLuint, GLfloat, GLfloat,
                                       GLfloat, GLfloat);
int (APIENTRY *SwapInterval)(int);
void (APIENTRY *TexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei,
                            GLint, GLenum, GLenum, const GLvoid *);
/** \} */ // end of glextfunctions group

//! \defgroup glgeneral OpenGL general helper functions

//! \defgroup glcontext OpenGL context management helper functions

//! \defgroup gltexture OpenGL texture handling helper functions

//! \defgroup glconversion OpenGL conversion helper functions

/**
 * \brief adjusts the GL_UNPACK_ALIGNMENT to fit the stride.
 * \param stride number of bytes per line for which alignment should fit.
 * \ingroup glgeneral
 */
void glAdjustAlignment(int stride) {
  GLint gl_alignment;
  if (stride % 8 == 0)
    gl_alignment=8;
  else if (stride % 4 == 0)
    gl_alignment=4;
  else if (stride % 2 == 0)
    gl_alignment=2;
  else
    gl_alignment=1;
  glPixelStorei (GL_UNPACK_ALIGNMENT, gl_alignment);
}

struct gl_name_map_struct {
  GLint value;
  char *name;
};

#undef MAP
#define MAP(a) {a, #a}
//! mapping table for the glValName function
static const struct gl_name_map_struct gl_name_map[] = {
  // internal format
  MAP(GL_R3_G3_B2), MAP(GL_RGB4), MAP(GL_RGB5), MAP(GL_RGB8),
  MAP(GL_RGB10), MAP(GL_RGB12), MAP(GL_RGB16), MAP(GL_RGBA2),
  MAP(GL_RGBA4), MAP(GL_RGB5_A1), MAP(GL_RGBA8), MAP(GL_RGB10_A2),
  MAP(GL_RGBA12), MAP(GL_RGBA16), MAP(GL_LUMINANCE8),

  // format
  MAP(GL_RGB), MAP(GL_RGBA), MAP(GL_RED), MAP(GL_GREEN), MAP(GL_BLUE),
  MAP(GL_ALPHA), MAP(GL_LUMINANCE), MAP(GL_LUMINANCE_ALPHA),
  MAP(GL_COLOR_INDEX),
  // rest 1.2 only
  MAP(GL_BGR), MAP(GL_BGRA),

  //type
  MAP(GL_BYTE), MAP(GL_UNSIGNED_BYTE), MAP(GL_SHORT), MAP(GL_UNSIGNED_SHORT),
  MAP(GL_INT), MAP(GL_UNSIGNED_INT), MAP(GL_FLOAT), MAP(GL_DOUBLE),
  MAP(GL_2_BYTES), MAP(GL_3_BYTES), MAP(GL_4_BYTES),
  // rest 1.2 only
  MAP(GL_UNSIGNED_BYTE_3_3_2), MAP(GL_UNSIGNED_BYTE_2_3_3_REV),
  MAP(GL_UNSIGNED_SHORT_5_6_5), MAP(GL_UNSIGNED_SHORT_5_6_5_REV),
  MAP(GL_UNSIGNED_SHORT_4_4_4_4), MAP(GL_UNSIGNED_SHORT_4_4_4_4_REV),
  MAP(GL_UNSIGNED_SHORT_5_5_5_1), MAP(GL_UNSIGNED_SHORT_1_5_5_5_REV),
  MAP(GL_UNSIGNED_INT_8_8_8_8), MAP(GL_UNSIGNED_INT_8_8_8_8_REV),
  MAP(GL_UNSIGNED_INT_10_10_10_2), MAP(GL_UNSIGNED_INT_2_10_10_10_REV),
  {0, 0}
};
#undef MAP

/**
 * \brief return the name of an OpenGL constant
 * \param value the constant
 * \return name of the constant or "Unknown format!"
 * \ingroup glgeneral
 */
const char *glValName(GLint value)
{
  int i = 0;

  while (gl_name_map[i].name) {
    if (gl_name_map[i].value == value)
      return gl_name_map[i].name;
    i++;
  }
  return "Unknown format!";
}

//! always return this format as internal texture format in glFindFormat
#define TEXTUREFORMAT_ALWAYS GL_RGB8
#undef TEXTUREFORMAT_ALWAYS

/**
 * \brief find the OpenGL settings coresponding to format.
 *
 * All parameters may be NULL.
 * \param fmt MPlayer format to analyze.
 * \param bpp [OUT] bits per pixel of that format.
 * \param gl_texfmt [OUT] internal texture format that fits the
 * image format, not necessarily the best for performance.
 * \param gl_format [OUT] OpenGL format for this image format.
 * \param gl_type [OUT] OpenGL type for this image format.
 * \return 1 if format is supported by OpenGL, 0 if not.
 * \ingroup gltexture
 */
int glFindFormat(uint32_t fmt, int *bpp, GLint *gl_texfmt,
                  GLenum *gl_format, GLenum *gl_type)
{
  int supported = 1;
  int dummy1;
  GLenum dummy2;
  GLint dummy3;
  if (bpp == NULL) bpp = &dummy1;
  if (gl_texfmt == NULL) gl_texfmt = &dummy3;
  if (gl_format == NULL) gl_format = &dummy2;
  if (gl_type == NULL) gl_type = &dummy2;
  
  *bpp = IMGFMT_IS_BGR(fmt)?IMGFMT_BGR_DEPTH(fmt):IMGFMT_RGB_DEPTH(fmt);
  *gl_texfmt = 3;
  switch (fmt) {
    case IMGFMT_RGB24:
      *gl_format = GL_RGB;
      *gl_type = GL_UNSIGNED_BYTE;
      break;
    case IMGFMT_RGBA:
      *gl_texfmt = 4;
      *gl_format = GL_RGBA;
      *gl_type = GL_UNSIGNED_BYTE;
      break;
    case IMGFMT_YV12:
      supported = 0; // no native YV12 support
    case IMGFMT_Y800:
    case IMGFMT_Y8:
      *gl_texfmt = 1;
      *bpp = 8;
      *gl_format = GL_LUMINANCE;
      *gl_type = GL_UNSIGNED_BYTE;
      break;
#if 0
    // we do not support palettized formats, although the format the
    // swscale produces works
    case IMGFMT_RGB8:
      gl_format = GL_RGB;
      gl_type = GL_UNSIGNED_BYTE_2_3_3_REV;
      break;
#endif
    case IMGFMT_RGB15:
      *gl_format = GL_RGBA;
      *gl_type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
      break;
    case IMGFMT_RGB16:
      *gl_format = GL_RGB;
      *gl_type = GL_UNSIGNED_SHORT_5_6_5_REV;
      break;
#if 0
    case IMGFMT_BGR8:
      // special case as red and blue have a differen number of bits.
      // GL_BGR and GL_UNSIGNED_BYTE_3_3_2 isn't supported at least
      // by nVidia drivers, and in addition would give more bits to
      // blue than to red, which isn't wanted
      gl_format = GL_RGB;
      gl_type = GL_UNSIGNED_BYTE_3_3_2;
      break;
#endif
    case IMGFMT_BGR15:
      *gl_format = GL_BGRA;
      *gl_type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
      break;
    case IMGFMT_BGR16:
      *gl_format = GL_RGB;
      *gl_type = GL_UNSIGNED_SHORT_5_6_5;
      break;
    case IMGFMT_BGR24:
      *gl_format = GL_BGR;
      *gl_type = GL_UNSIGNED_BYTE;
      break;
    case IMGFMT_BGRA:
      *gl_texfmt = 4;
      *gl_format = GL_BGRA;
      *gl_type = GL_UNSIGNED_BYTE;
      break;
    default:
      *gl_texfmt = 4;
      *gl_format = GL_RGBA;
      *gl_type = GL_UNSIGNED_BYTE;
      supported = 0;
  }
#ifdef TEXTUREFORMAT_ALWAYS
  *gl_texfmt = TEXTUREFORMAT_ALWAYS;
#endif
  return supported;
}

static void *setNull(const GLubyte *s) {
  return NULL;
}

typedef struct {
  void **funcptr;
  char *extstr;
  char *funcnames[7];
} extfunc_desc_t;

static const extfunc_desc_t extfuncs[] = {
  {(void **)&GenBuffers, NULL, {"glGenBuffers", "glGenBuffersARB", NULL}},
  {(void **)&DeleteBuffers, NULL, {"glDeleteBuffers", "glDeleteBuffersARB", NULL}},
  {(void **)&BindBuffer, NULL, {"glBindBuffer", "glBindBufferARB", NULL}},
  {(void **)&MapBuffer, NULL, {"glMapBuffer", "glMapBufferARB", NULL}},
  {(void **)&UnmapBuffer, NULL, {"glUnmapBuffer", "glUnmapBufferARB", NULL}},
  {(void **)&BufferData, NULL, {"glBufferData", "glBufferDataARB", NULL}},
  {(void **)&CombinerParameterfv, "NV_register_combiners", {"glCombinerParameterfv", "glCombinerParameterfvNV", NULL}},
  {(void **)&CombinerParameteri, "NV_register_combiners", {"glCombinerParameteri", "glCombinerParameteriNV", NULL}},
  {(void **)&CombinerInput, "NV_register_combiners", {"glCombinerInput", "glCombinerInputNV", NULL}},
  {(void **)&CombinerOutput, "NV_register_combiners", {"glCombinerOutput", "glCombinerOutputNV", NULL}},
  {(void **)&BeginFragmentShader, "ATI_fragment_shader", {"glBeginFragmentShaderATI", NULL}},
  {(void **)&EndFragmentShader, "ATI_fragment_shader", {"glEndFragmentShaderATI", NULL}},
  {(void **)&SampleMap, "ATI_fragment_shader", {"glSampleMapATI", NULL}},
  {(void **)&ColorFragmentOp2, "ATI_fragment_shader", {"glColorFragmentOp2ATI", NULL}},
  {(void **)&ColorFragmentOp3, "ATI_fragment_shader", {"glColorFragmentOp3ATI", NULL}},
  {(void **)&SetFragmentShaderConstant, "ATI_fragment_shader", {"glSetFragmentShaderConstantATI", NULL}},
  {(void **)&ActiveTexture, NULL, {"glActiveTexture", "glActiveTextureARB", NULL}},
  {(void **)&BindTexture, NULL, {"glBindTexture", "glBindTextureARB", "glBindTextureEXT", NULL}},
  {(void **)&MultiTexCoord2f, NULL, {"glMultiTexCoord2f", "glMultiTexCoord2fARB", NULL}},
  {(void **)&GenPrograms, "_program", {"glGenPrograms", "glGenProgramsARB", "glGenProgramsNV", NULL}},
  {(void **)&DeletePrograms, "_program", {"glDeletePrograms", "glDeleteProgramsARB", "glDeleteProgramsNV", NULL}},
  {(void **)&BindProgram, "_program", {"glBindProgram", "glBindProgramARB", "glBindProgramNV", NULL}},
  {(void **)&ProgramString, "_program", {"glProgramString", "glProgramStringARB", "glProgramStringNV", NULL}},
  {(void **)&GetProgramiv, "_program", {"glGetProgramiv", "glGetProgramivARB", "glGetProgramivNV", NULL}},
  {(void **)&ProgramEnvParameter4f, "_program", {"glProgramEnvParameter4f", "glProgramEnvParameter4fARB", "glProgramEnvParameter4fNV", NULL}},
  {(void **)&SwapInterval, "_swap_control", {"glXSwapInterval", "glXSwapIntervalEXT", "glXSwapIntervalSGI", "wglSwapInterval", "wglSwapIntervalEXT", "wglSwapIntervalSGI", NULL}},
  {(void **)&TexImage3D, NULL, {"glTexImage3D", NULL}},
  {NULL}
};

/**
 * \brief find the function pointers of some useful OpenGL extensions
 * \param getProcAddress function to resolve function names, may be NULL
 * \param ext2 an extra extension string
 */
static void getFunctions(void *(*getProcAddress)(const GLubyte *),
                         const char *ext2) {
  const extfunc_desc_t *dsc;
  const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
  char *allexts;
  if (!extensions) extensions = "";
  if (!ext2) ext2 = "";
  allexts = (char *)malloc(strlen(extensions) + strlen(ext2) + 2);
  strcpy(allexts, extensions);
  strcat(allexts, " ");
  strcat(allexts, ext2);
  mp_msg(MSGT_VO, MSGL_V, "OpenGL extensions string:\n%s\n", allexts);
  if (!getProcAddress)
    getProcAddress = setNull;
  for (dsc = extfuncs; dsc->funcptr; dsc++) {
    void *ptr = NULL;
    int i;
    if (!dsc->extstr || strstr(allexts, dsc->extstr)) {
      for (i = 0; !ptr && dsc->funcnames[i]; i++)
        ptr = getProcAddress((const GLubyte *)dsc->funcnames[i]);
    }
    *(dsc->funcptr) = ptr;
  }
  free(allexts);
}

/**
 * \brief create a texture and set some defaults
 * \param target texture taget, usually GL_TEXTURE_2D
 * \param fmt internal texture format
 * \param filter filter used for scaling, e.g. GL_LINEAR
 * \param w texture width
 * \param h texture height
 * \param val luminance value to fill texture with
 * \ingroup gltexture
 */
void glCreateClearTex(GLenum target, GLenum fmt, GLint filter,
                      int w, int h, unsigned char val) {
  GLfloat fval = (GLfloat)val / 255.0;
  GLfloat border[4] = {fval, fval, fval, fval};
  GLenum clrfmt = (fmt == GL_ALPHA) ? GL_ALPHA : GL_LUMINANCE;
  char *init = (char *)malloc(w * h);
  memset(init, val, w * h);
  glAdjustAlignment(w);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
  glTexImage2D(target, 0, fmt, w, h, 0, clrfmt, GL_UNSIGNED_BYTE, init);
  glTexParameterf(target, GL_TEXTURE_PRIORITY, 1.0);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // Border texels should not be used with CLAMP_TO_EDGE
  // We set a sane default anyway.
  glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, border);
  free(init);
}

/**
 * \brief skips whitespace and comments
 * \param f file to read from
 */
static void ppm_skip(FILE *f) {
  int c, comment = 0;
  do {
    c = fgetc(f);
    if (c == '#')
      comment = 1;
    if (c == '\n')
      comment = 0;
  } while (c != EOF && (isspace(c) || comment));
  if (c != EOF)
    ungetc(c, f);
}

#define MAXDIM (16 * 1024)

/**
 * \brief creates a texture from a PPM file
 * \param target texture taget, usually GL_TEXTURE_2D
 * \param fmt internal texture format
 * \param filter filter used for scaling, e.g. GL_LINEAR
 * \param f file to read PPM from
 * \param width [out] width of texture
 * \param height [out] height of texture
 * \param maxval [out] maxval value from PPM file
 * \return 0 on error, 1 otherwise
 * \ingroup gltexture
 */
int glCreatePPMTex(GLenum target, GLenum fmt, GLint filter,
                   FILE *f, int *width, int *height, int *maxval) {
  unsigned w, h, m, val;
  char *data;
  ppm_skip(f);
  if (fgetc(f) != 'P' || fgetc(f) != '6')
    return 0;
  ppm_skip(f);
  if (fscanf(f, "%u", &w) != 1)
    return 0;
  ppm_skip(f);
  if (fscanf(f, "%u", &h) != 1)
    return 0;
  ppm_skip(f);
  if (fscanf(f, "%u", &m) != 1)
    return 0;
  val = fgetc(f);
  if (!isspace(val))
    return 0;
  if (w > MAXDIM || h > MAXDIM)
    return 0;
  data = (char *)malloc(w * h * 3);
  if (fread(data, w * 3, h, f) != h)
    return 0;
  glCreateClearTex(target, fmt, filter, w, h, 0);
  glUploadTex(target, GL_RGB, GL_UNSIGNED_BYTE, data, w * 3, 0, 0, w, h, 0);
  free(data);
  if (width) *width = w;
  if (height) *height = h;
  if (maxval) *maxval = m;
  return 1;
}

/**
 * \brief return the number of bytes per pixel for the given format
 * \param format OpenGL format
 * \param type OpenGL type
 * \return bytes per pixel
 * \ingroup glgeneral
 *
 * Does not handle all possible variants, just those used by MPlayer
 */
int glFmt2bpp(GLenum format, GLenum type) {
  switch (type) {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
      return 1;
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
      return 2;
  }
  if (type != GL_UNSIGNED_BYTE)
    return 0; //not implemented
  switch (format) {
    case GL_LUMINANCE:
    case GL_ALPHA:
      return 1;
    case GL_RGB:
    case GL_BGR:
      return 3;
    case GL_RGBA:
    case GL_BGRA:
      return 4;
  }
  return 0; // unknown
}

/**
 * \brief upload a texture, handling things like stride and slices
 * \param target texture target, usually GL_TEXTURE_2D
 * \param format OpenGL format of data
 * \param type OpenGL type of data
 * \param data data to upload
 * \param stride data stride
 * \param x x offset in texture
 * \param y y offset in texture
 * \param w width of the texture part to upload
 * \param h height of the texture part to upload
 * \param slice height of an upload slice, 0 for all at once
 * \ingroup gltexture
 */
void glUploadTex(GLenum target, GLenum format, GLenum type,
                 const void *data, int stride,
                 int x, int y, int w, int h, int slice) {
  int y_max = y + h;
  if (w <= 0 || h <= 0) return;
  if (slice <= 0)
    slice = h;
  if (stride < 0) {
    data += h * stride;
    stride = -stride;
  }
  // this is not always correct, but should work for MPlayer
  glAdjustAlignment(stride);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / glFmt2bpp(format, type));
  for (; y + slice <= y_max; y += slice) {
    glTexSubImage2D(target, 0, x, y, w, slice, format, type, data);
    data += stride * slice;
  }
  if (y < y_max)
    glTexSubImage2D(target, 0, x, y, w, y_max - y, format, type, data);
}

static void fillUVcoeff(GLfloat *ucoef, GLfloat *vcoef,
                        float uvcos, float uvsin) {
  int i;
  ucoef[0] = 0 * uvcos + 1.403 * uvsin;
  vcoef[0] = 0 * uvsin + 1.403 * uvcos;
  ucoef[1] = -0.344 * uvcos + -0.714 * uvsin;
  vcoef[1] = -0.344 * uvsin + -0.714 * uvcos;
  ucoef[2] = 1.770 * uvcos + 0 * uvsin;
  vcoef[2] = 1.770 * uvsin + 0 * uvcos;
  ucoef[3] = 0;
  vcoef[3] = 0;
  // Coefficients (probably) must be in [0, 1] range, whereas they originally
  // are in [-2, 2] range, so here comes the trick:
  // First put them in the [-0.5, 0.5] range, then add 0.5.
  // This can be undone with the HALF_BIAS and SCALE_BY_FOUR arguments
  // for CombinerInput and CombinerOutput (or the respective ATI variants)
  for (i = 0; i < 4; i++) {
    ucoef[i] = ucoef[i] * 0.25 + 0.5;
    vcoef[i] = vcoef[i] * 0.25 + 0.5;
  }
}

/**
 * \brief Setup register combiners for YUV to RGB conversion.
 * \param uvcos used for saturation and hue adjustment
 * \param uvsin used for saturation and hue adjustment
 */
static void glSetupYUVCombiners(float uvcos, float uvsin) {
  GLfloat ucoef[4];
  GLfloat vcoef[4];
  GLint i;
  if (!CombinerInput || !CombinerOutput ||
      !CombinerParameterfv || !CombinerParameteri) {
    mp_msg(MSGT_VO, MSGL_FATAL, "[gl] Combiner functions missing!\n");
    return;
  }
  glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV, &i);
  if (i < 2)
    mp_msg(MSGT_VO, MSGL_ERR,
           "[gl] 2 general combiners needed for YUV combiner support (found %i)\n", i);
  glGetIntegerv (GL_MAX_TEXTURE_UNITS, &i);
  if (i < 3)
    mp_msg(MSGT_VO, MSGL_ERR,
           "[gl] 3 texture units needed for YUV combiner support (found %i)\n", i);
  fillUVcoeff(ucoef, vcoef, uvcos, uvsin);
  CombinerParameterfv(GL_CONSTANT_COLOR0_NV, ucoef);
  CombinerParameterfv(GL_CONSTANT_COLOR1_NV, vcoef);

  // UV first, like this green component cannot overflow
  CombinerInput(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV,
                GL_TEXTURE1, GL_HALF_BIAS_NORMAL_NV, GL_RGB);
  CombinerInput(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV,
                GL_CONSTANT_COLOR0_NV, GL_HALF_BIAS_NORMAL_NV, GL_RGB);
  CombinerInput(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV,
                GL_TEXTURE2, GL_HALF_BIAS_NORMAL_NV, GL_RGB);
  CombinerInput(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV,
                GL_CONSTANT_COLOR1_NV, GL_HALF_BIAS_NORMAL_NV, GL_RGB);
  CombinerOutput(GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV,
                 GL_SPARE0_NV, GL_SCALE_BY_FOUR_NV, GL_NONE, GL_FALSE,
                 GL_FALSE, GL_FALSE);

  // stage 2
  CombinerInput(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_SPARE0_NV,
                GL_SIGNED_IDENTITY_NV, GL_RGB);
  CombinerInput(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO,
                 GL_UNSIGNED_INVERT_NV, GL_RGB);
  CombinerInput(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV,
                GL_TEXTURE0, GL_SIGNED_IDENTITY_NV, GL_RGB);
  CombinerInput(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO,
                GL_UNSIGNED_INVERT_NV, GL_RGB);
  CombinerOutput(GL_COMBINER1_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV,
                 GL_SPARE0_NV, GL_NONE, GL_NONE, GL_FALSE,
                 GL_FALSE, GL_FALSE);

  // leave final combiner stage in default mode
  CombinerParameteri(GL_NUM_GENERAL_COMBINERS_NV, 2);
}

/**
 * \brief Setup ATI version of register combiners for YUV to RGB conversion.
 * \param uvcos used for saturation and hue adjustment
 * \param uvsin used for saturation and hue adjustment
 *
 * ATI called this fragment shader, but the name is confusing in the
 * light of a very different OpenGL 2.0 extension with the same name
 */
static void glSetupYUVCombinersATI(float uvcos, float uvsin) {
  GLfloat ucoef[4];
  GLfloat vcoef[4];
  GLint i;
  if (!BeginFragmentShader || !EndFragmentShader ||
      !SetFragmentShaderConstant || !SampleMap ||
      !ColorFragmentOp2 || !ColorFragmentOp3) {
    mp_msg(MSGT_VO, MSGL_FATAL, "[gl] Combiner (ATI) functions missing!\n");
    return;
  }
  glGetIntegerv(GL_NUM_FRAGMENT_REGISTERS_ATI, &i);
  if (i < 3)
    mp_msg(MSGT_VO, MSGL_ERR,
           "[gl] 3 registers needed for YUV combiner (ATI) support (found %i)\n", i);
  glGetIntegerv (GL_MAX_TEXTURE_UNITS, &i);
  if (i < 3)
    mp_msg(MSGT_VO, MSGL_ERR,
           "[gl] 3 texture units needed for YUV combiner (ATI) support (found %i)\n", i);
  fillUVcoeff(ucoef, vcoef, uvcos, uvsin);
  BeginFragmentShader();
  SetFragmentShaderConstant(GL_CON_0_ATI, ucoef);
  SetFragmentShaderConstant(GL_CON_1_ATI, vcoef);
  SampleMap(GL_REG_0_ATI, GL_TEXTURE0, GL_SWIZZLE_STR_ATI);
  SampleMap(GL_REG_1_ATI, GL_TEXTURE1, GL_SWIZZLE_STR_ATI);
  SampleMap(GL_REG_2_ATI, GL_TEXTURE2, GL_SWIZZLE_STR_ATI);
  // UV first, like this green component cannot overflow
  ColorFragmentOp2(GL_MUL_ATI, GL_REG_1_ATI, GL_NONE, GL_NONE,
                   GL_REG_1_ATI, GL_NONE, GL_BIAS_BIT_ATI,
                   GL_CON_0_ATI, GL_NONE, GL_BIAS_BIT_ATI);
  ColorFragmentOp3(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_4X_BIT_ATI,
                   GL_REG_2_ATI, GL_NONE, GL_BIAS_BIT_ATI,
                   GL_CON_1_ATI, GL_NONE, GL_BIAS_BIT_ATI,
                   GL_REG_1_ATI, GL_NONE, GL_NONE);
  ColorFragmentOp2(GL_ADD_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
                   GL_REG_0_ATI, GL_NONE, GL_NONE,
                   GL_REG_2_ATI, GL_NONE, GL_NONE);
  EndFragmentShader();
}

static void store_weights(float x, GLfloat *dst) {
  float w0 = (((-1 * x + 3) * x - 3) * x + 1) / 6;
  float w1 = ((( 3 * x - 6) * x + 0) * x + 4) / 6;
  float w2 = (((-3 * x + 3) * x + 3) * x + 1) / 6;
  float w3 = ((( 1 * x + 0) * x + 0) * x + 0) / 6;
  *dst++ = 1 + x - w1 / (w0 + w1);
  *dst++ = 1 - x + w3 / (w2 + w3);
  *dst++ = w0 + w1;
  *dst++ = 0;
}

//! to avoid artefacts this should be rather large
#define LOOKUP_BSPLINE_RES (2 * 1024)
/**
 * \brief creates the 1D lookup texture needed for fast higher-order filtering
 * \param unit texture unit to attach texture to
 */
static void gen_spline_lookup_tex(GLenum unit) {
  GLfloat tex[4 * LOOKUP_BSPLINE_RES];
  GLfloat *tp = tex;
  int i;
  for (i = 0; i < LOOKUP_BSPLINE_RES; i++) {
    float x = (float)(i + 0.5) / LOOKUP_BSPLINE_RES;
    store_weights(x, tp);
    tp += 4;
  }
  store_weights(0, tex);
  store_weights(1, &tex[4 * (LOOKUP_BSPLINE_RES - 1)]);
  ActiveTexture(unit);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, LOOKUP_BSPLINE_RES, 0, GL_RGBA, GL_FLOAT, tex);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_PRIORITY, 1.0);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  ActiveTexture(GL_TEXTURE0);
}

static const char *bilin_filt_template =
  "TEX yuv.%c, fragment.texcoord[%c], texture[%c], %s;";

#define BICUB_FILT_MAIN(textype) \
  /* first y-interpolation */ \
  "ADD coord, fragment.texcoord[%c].xyxy, cdelta.xyxw;" \
  "ADD coord2, fragment.texcoord[%c].xyxy, cdelta.zyzw;" \
  "TEX a.r, coord.xyxy, texture[%c], "textype";" \
  "TEX a.g, coord.zwzw, texture[%c], "textype";" \
  /* second y-interpolation */ \
  "TEX b.r, coord2.xyxy, texture[%c], "textype";" \
  "TEX b.g, coord2.zwzw, texture[%c], "textype";" \
  "LRP a.b, parmy.b, a.rrrr, a.gggg;" \
  "LRP a.a, parmy.b, b.rrrr, b.gggg;" \
  /* x-interpolation */ \
  "LRP yuv.%c, parmx.b, a.bbbb, a.aaaa;"

static const char *bicub_filt_template_2D =
  "MAD coord.xy, fragment.texcoord[%c], {%f, %f}, {0.5, 0.5};"
  "TEX parmx, coord.x, texture[%c], 1D;"
  "MUL cdelta.xz, parmx.rrgg, {-%f, 0, %f, 0};"
  "TEX parmy, coord.y, texture[%c], 1D;"
  "MUL cdelta.yw, parmy.rrgg, {0, -%f, 0, %f};"
  BICUB_FILT_MAIN("2D");

static const char *bicub_filt_template_RECT =
  "ADD coord, fragment.texcoord[%c], {0.5, 0.5};"
  "TEX parmx, coord.x, texture[%c], 1D;"
  "MUL cdelta.xz, parmx.rrgg, {-1, 0, 1, 0};"
  "TEX parmy, coord.y, texture[%c], 1D;"
  "MUL cdelta.yw, parmy.rrgg, {0, -1, 0, 1};"
  BICUB_FILT_MAIN("RECT");

static const char *yuv_prog_template =
  "PARAM ycoef = {%.4f, %.4f, %.4f};"
  "PARAM ucoef = {%.4f, %.4f, %.4f};"
  "PARAM vcoef = {%.4f, %.4f, %.4f};"
  "PARAM offsets = {%.4f, %.4f, %.4f};"
  "TEMP res;"
  "MAD res.rgb, yuv.rrrr, ycoef, offsets;"
  "MAD res.rgb, yuv.gggg, ucoef, res;"
  "MAD result.color.rgb, yuv.bbbb, vcoef, res;"
  "END";

static const char *yuv_pow_prog_template =
  "PARAM ycoef = {%.4f, %.4f, %.4f};"
  "PARAM ucoef = {%.4f, %.4f, %.4f};"
  "PARAM vcoef = {%.4f, %.4f, %.4f};"
  "PARAM offsets = {%.4f, %.4f, %.4f};"
  "PARAM gamma = {%.4f, %.4f, %.4f};"
  "TEMP res;"
  "MAD res.rgb, yuv.rrrr, ycoef, offsets;"
  "MAD res.rgb, yuv.gggg, ucoef, res;"
  "MAD_SAT res.rgb, yuv.bbbb, vcoef, res;"
  "POW result.color.r, res.r, gamma.r;"
  "POW result.color.g, res.g, gamma.g;"
  "POW result.color.b, res.b, gamma.b;"
  "END";

static const char *yuv_lookup_prog_template =
  "PARAM ycoef = {%.4f, %.4f, %.4f, 0};"
  "PARAM ucoef = {%.4f, %.4f, %.4f, 0};"
  "PARAM vcoef = {%.4f, %.4f, %.4f, 0};"
  "PARAM offsets = {%.4f, %.4f, %.4f, 0.125};"
  "TEMP res;"
  "MAD res, yuv.rrrr, ycoef, offsets;"
  "MAD res.rgb, yuv.gggg, ucoef, res;"
  "MAD res.rgb, yuv.bbbb, vcoef, res;"
  "TEX result.color.r, res.raaa, texture[%c], 2D;"
  "ADD res.a, res.a, 0.25;"
  "TEX result.color.g, res.gaaa, texture[%c], 2D;"
  "ADD res.a, res.a, 0.25;"
  "TEX result.color.b, res.baaa, texture[%c], 2D;"
  "END";

static const char *yuv_lookup3d_prog_template =
  "TEX result.color, yuv, texture[%c], 3D;"
  "END";

static void create_scaler_textures(int scaler, int *texu, char *texs) {
  switch (scaler) {
    case YUV_SCALER_BILIN:
      break;
    case YUV_SCALER_BICUB:
      texs[0] = (*texu)++;
      gen_spline_lookup_tex(GL_TEXTURE0 + texs[0]);
      texs[0] += '0';
      break;
    default:
      mp_msg(MSGT_VO, MSGL_ERR, "[gl] unknown scaler type %i\n", scaler);
  }
}

static void gen_yuv2rgb_map(unsigned char *map, int size, float brightness,
                            float contrast, float uvcos, float uvsin,
                            float rgamma, float ggamma, float bgamma) {
  int i, j, k;
  float step = 1.0 / size;
  float y, u, v, u_, v_;
  float r, g, b;
  v = -0.5;
  for (i = -1; i <= size; i++) {
    u = -0.5;
    for (j = -1; j <= size; j++) {
      y = -(16.0 / 255.0);
      for (k = -1; k <= size; k++) {
        u_ = uvcos * u + uvsin * v;
        v_ = uvcos * v + uvsin * u;
        r = 1.164 * y              + 1.596 * v_;
        g = 1.164 * y - 0.391 * u_ - 0.813 * v_;
        b = 1.164 * y + 2.018 * u_             ;
        r = pow(contrast * (r - 0.5) + 0.5 + brightness, 1.0 / rgamma);
        g = pow(contrast * (g - 0.5) + 0.5 + brightness, 1.0 / ggamma);
        b = pow(contrast * (b - 0.5) + 0.5 + brightness, 1.0 / bgamma);
        if (r > 1) r = 1;
        if (r < 0) r = 0;
        if (g > 1) g = 1;
        if (g < 0) g = 0;
        if (b > 1) b = 1;
        if (b < 0) b = 0;
        *map++ = 255 * r;
        *map++ = 255 * g;
        *map++ = 255 * b;
        y += (k == -1 || k == size - 1) ? step / 2 : step;
      }
      u += (j == -1 || j == size - 1) ? step / 2 : step;
    }
    v += (i == -1 || i == size - 1) ? step / 2 : step;
  }
}

static void gen_gamma_map(unsigned char *map, int size, float gamma);

//! resolution of texture for gamma lookup table
#define LOOKUP_RES 512
//! resolution for 3D yuv->rgb conversion lookup table
#define LOOKUP_3DRES 32
static void create_conv_textures(int conv, int *texu, char *texs,
              float brightness, float contrast, float uvcos, float uvsin,
              float rgamma, float ggamma, float bgamma) {
  unsigned char *lookup_data = NULL;
  switch (conv) {
    case YUV_CONVERSION_FRAGMENT:
    case YUV_CONVERSION_FRAGMENT_POW:
     break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP:
      texs[0] = (*texu)++;
      ActiveTexture(GL_TEXTURE0 + texs[0]);
      lookup_data = malloc(4 * LOOKUP_RES);
      gen_gamma_map(lookup_data, LOOKUP_RES, rgamma);
      gen_gamma_map(&lookup_data[LOOKUP_RES], LOOKUP_RES, ggamma);
      gen_gamma_map(&lookup_data[2 * LOOKUP_RES], LOOKUP_RES, bgamma);
      glCreateClearTex(GL_TEXTURE_2D, GL_LUMINANCE8, GL_LINEAR,
                       LOOKUP_RES, 4, 0);
      glUploadTex(GL_TEXTURE_2D, GL_LUMINANCE, GL_UNSIGNED_BYTE, lookup_data,
                  LOOKUP_RES, 0, 0, LOOKUP_RES, 4, 0);
      ActiveTexture(GL_TEXTURE0);
      texs[0] += '0';
      break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP3D:
      {
        int sz = LOOKUP_3DRES + 2; // texture size including borders
        if (!TexImage3D) {
          mp_msg(MSGT_VO, MSGL_ERR, "[gl] Missing 3D texture function!\n");
          break;
        }
        texs[0] = (*texu)++;
        ActiveTexture(GL_TEXTURE0 + texs[0]);
        lookup_data = malloc(3 * sz * sz * sz);
        gen_yuv2rgb_map(lookup_data, LOOKUP_3DRES, brightness, contrast,
                        uvcos, uvsin, rgamma, ggamma, bgamma);
        glAdjustAlignment(sz);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        TexImage3D(GL_TEXTURE_3D, 0, 3, sz, sz, sz, 1,
                   GL_RGB, GL_UNSIGNED_BYTE, lookup_data);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_PRIORITY, 1.0);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        ActiveTexture(GL_TEXTURE0);
        texs[0] += '0';
      }
      break;
    default:
      mp_msg(MSGT_VO, MSGL_ERR, "[gl] unknown conversion type %i\n", conv);
  }
  if (lookup_data)
    free(lookup_data);
}

static void add_scaler(int scaler, char **prog_pos, int *remain, char *texs,
                    char in_tex, char out_comp, int rect, int texw, int texh) {
  switch (scaler) {
    case YUV_SCALER_BILIN:
      snprintf(*prog_pos, *remain, bilin_filt_template, out_comp, in_tex,
                 in_tex, rect ? "RECT" : "2D");
      break;
    case YUV_SCALER_BICUB:
      if (rect)
        snprintf(*prog_pos, *remain, bicub_filt_template_RECT,
                 in_tex, texs[0], texs[0],
                 in_tex, in_tex, in_tex, in_tex, in_tex, in_tex, out_comp);
      else
        snprintf(*prog_pos, *remain, bicub_filt_template_2D,
                 in_tex, (float)texw, (float)texh,
                 texs[0], (float)1.0 / texw, (float)1.0 / texw,
                 texs[0], (float)1.0 / texh, (float)1.0 / texh,
                 in_tex, in_tex, in_tex, in_tex, in_tex, in_tex, out_comp);
      break;
  }
  *remain -= strlen(*prog_pos);
  *prog_pos += strlen(*prog_pos);
}

static const struct {
  char *name;
  GLenum cur;
  GLenum max;
} progstats[] = {
  {"instructions", 0x88A0, 0x88A1},
  {"native instructions", 0x88A2, 0x88A3},
  {"temporaries", 0x88A4, 0x88A5},
  {"native temporaries", 0x88A6, 0x88A7},
  {"parameters", 0x88A8, 0x88A9},
  {"native parameters", 0x88AA, 0x88AB},
  {"attribs", 0x88AC, 0x88AD},
  {"native attribs", 0x88AE, 0x88AF},
  {"ALU instructions", 0x8805, 0x880B},
  {"TEX instructions", 0x8806, 0x880C},
  {"TEX indirections", 0x8807, 0x880D},
  {"native ALU instructions", 0x8808, 0x880E},
  {"native TEX instructions", 0x8809, 0x880F},
  {"native TEX indirections", 0x880A, 0x8810},
  {NULL, 0, 0}
};

int loadGPUProgram(GLenum target, char *prog) {
  int i;
  GLint cur = 0, max = 0, err = 0;
  if (!ProgramString) {
    mp_msg(MSGT_VO, MSGL_ERR, "[gl] Missing GPU program function\n");
    return 0;
  }
  ProgramString(target, GL_PROGRAM_FORMAT_ASCII, strlen(prog), prog);
  glGetIntegerv(GL_PROGRAM_ERROR_POSITION, &err);
  if (err != -1) {
    mp_msg(MSGT_VO, MSGL_ERR,
      "[gl] Error compiling fragment program, make sure your card supports\n"
      "[gl]   GL_ARB_fragment_program (use glxinfo to check).\n"
      "[gl]   Error message:\n  %s at %.10s\n",
      glGetString(GL_PROGRAM_ERROR_STRING), &prog[err]);
    return 0;
  }
  if (!GetProgramiv || !mp_msg_test(MSGT_VO, MSGL_V))
    return 1;
  mp_msg(MSGT_VO, MSGL_V, "[gl] Program statistics:\n");
  for (i = 0; progstats[i].name; i++) {
    GetProgramiv(target, progstats[i].cur, &cur);
    GetProgramiv(target, progstats[i].max, &max);
    mp_msg(MSGT_VO, MSGL_V, "[gl]   %s: %i/%i\n", progstats[i].name, cur, max);
  }
  return 1;
}

/**
 * \brief setup a fragment program that will do YUV->RGB conversion
 * \param brightness brightness adjustment offset
 * \param contrast contrast adjustment factor
 * \param uvcos used for saturation and hue adjustment
 * \param uvsin used for saturation and hue adjustment
 * \param lookup use fragment program that uses texture unit 4 to
 *               do additional conversion via lookup.
 */
static void glSetupYUVFragprog(float brightness, float contrast,
                        float uvcos, float uvsin, float rgamma,
                        float ggamma, float bgamma, int type, int rect,
                        int texw, int texh) {
  char yuv_prog[4000] =
    "!!ARBfp1.0\n"
    "OPTION ARB_precision_hint_fastest;"
    // all scaler variables must go here so they aren't defined
    // multiple times when the same scaler is used more than once
    "TEMP coord, coord2, cdelta, parmx, parmy, a, b, yuv;";
  int prog_remain = sizeof(yuv_prog) - strlen(yuv_prog);
  char *prog_pos = &yuv_prog[strlen(yuv_prog)];
  int cur_texu = 3;
  char lum_scale_texs[1];
  char chrom_scale_texs[1];
  char conv_texs[1];
  GLint i;
  // this is the conversion matrix, with y, u, v factors
  // for red, green, blue and the constant offsets
  float ry, ru, rv, rc;
  float gy, gu, gv, gc;
  float by, bu, bv, bc;
  create_scaler_textures(YUV_LUM_SCALER(type), &cur_texu, lum_scale_texs);
  if (YUV_CHROM_SCALER(type) == YUV_LUM_SCALER(type))
    memcpy(chrom_scale_texs, lum_scale_texs, sizeof(chrom_scale_texs));
  else
    create_scaler_textures(YUV_CHROM_SCALER(type), &cur_texu, chrom_scale_texs);
  create_conv_textures(YUV_CONVERSION(type), &cur_texu, conv_texs,
      brightness, contrast, uvcos, uvsin, rgamma, ggamma, bgamma);
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i);
  if (i < cur_texu)
    mp_msg(MSGT_VO, MSGL_ERR,
           "[gl] %i texture units needed for this type of YUV fragment support (found %i)\n",
           cur_texu, i);
  if (!ProgramString) {
    mp_msg(MSGT_VO, MSGL_FATAL, "[gl] ProgramString function missing!\n");
    return;
  }
  add_scaler(YUV_LUM_SCALER(type), &prog_pos, &prog_remain, lum_scale_texs,
             '0', 'r', rect, texw, texh);
  add_scaler(YUV_CHROM_SCALER(type), &prog_pos, &prog_remain, chrom_scale_texs,
             '1', 'g', rect, texw / 2, texh / 2);
  add_scaler(YUV_CHROM_SCALER(type), &prog_pos, &prog_remain, chrom_scale_texs,
             '2', 'b', rect, texw / 2, texh / 2);
  ry = 1.164 * contrast;
  gy = 1.164 * contrast;
  by = 1.164 * contrast;
  ru = 0 * uvcos + 1.596 * uvsin;
  rv = 0 * uvsin + 1.596 * uvcos;
  gu = -0.391 * uvcos + -0.813 * uvsin;
  gv = -0.391 * uvsin + -0.813 * uvcos;
  bu = 2.018 * uvcos + 0 * uvsin;
  bv = 2.018 * uvsin + 0 * uvcos;
  rc = (-16 * ry + (-128) * ru + (-128) * rv) / 255.0 + brightness;
  gc = (-16 * gy + (-128) * gu + (-128) * gv) / 255.0 + brightness;
  bc = (-16 * by + (-128) * bu + (-128) * bv) / 255.0 + brightness;
  // these "center" contrast control so that e.g. a contrast of 0
  // leads to a grey image, not a black one
  rc += 0.5 - contrast / 2.0;
  gc += 0.5 - contrast / 2.0;
  bc += 0.5 - contrast / 2.0;
  switch (YUV_CONVERSION(type)) {
    case YUV_CONVERSION_FRAGMENT:
      snprintf(prog_pos, prog_remain, yuv_prog_template,
               ry, gy, by, ru, gu, bu, rv, gv, bv, rc, gc, bc);
      break;
    case YUV_CONVERSION_FRAGMENT_POW:
      snprintf(prog_pos, prog_remain, yuv_pow_prog_template,
               ry, gy, by, ru, gu, bu, rv, gv, bv, rc, gc, bc,
               (float)1.0 / rgamma, (float)1.0 / bgamma, (float)1.0 / bgamma);
      break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP:
      snprintf(prog_pos, prog_remain, yuv_lookup_prog_template,
               ry, gy, by, ru, gu, bu, rv, gv, bv, rc, gc, bc,
               conv_texs[0], conv_texs[0], conv_texs[0]);
      break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP3D:
      snprintf(prog_pos, prog_remain, yuv_lookup3d_prog_template, conv_texs[0]);
      break;
    default:
      mp_msg(MSGT_VO, MSGL_ERR, "[gl] unknown conversion type %i\n", YUV_CONVERSION(type));
      break;
  }
  mp_msg(MSGT_VO, MSGL_V, "[gl] generated fragment program:\n%s\n", yuv_prog);
  loadGPUProgram(GL_FRAGMENT_PROGRAM, yuv_prog);
}

/**
 * \brief little helper function to create a lookup table for gamma
 * \param map buffer to create map into
 * \param size size of buffer
 * \param gamma gamma value
 */
static void gen_gamma_map(unsigned char *map, int size, float gamma) {
  int i;
  gamma = 1.0 / gamma;
  for (i = 0; i < size; i++) {
    float tmp = (float)i / (size - 1.0);
    tmp = pow(tmp, gamma);
    if (tmp > 1.0) tmp = 1.0;
    if (tmp < 0.0) tmp = 0.0;
    map[i] = 255 * tmp;
  }
}

/**
 * \brief setup YUV->RGB conversion
 * \param target texture target for Y, U and V textures (e.g. GL_TEXTURE_2D)
 * \param type YUV conversion type
 * \param brightness brightness adjustment offset
 * \param contrast contrast adjustment factor
 * \param hue hue adjustment angle
 * \param saturation saturation adjustment factor
 * \param rgamma gamma value for red channel
 * \param ggamma gamma value for green channel
 * \param bgamma gamma value for blue channel
 * \ingroup glconversion
 */
void glSetupYUVConversion(GLenum target, int type,
                          float brightness, float contrast,
                          float hue, float saturation,
                          float rgamma, float ggamma, float bgamma,
                          int texw, int texh) {
  float uvcos = saturation * cos(hue);
  float uvsin = saturation * sin(hue);
  switch (YUV_CONVERSION(type)) {
    case YUV_CONVERSION_COMBINERS:
      glSetupYUVCombiners(uvcos, uvsin);
      break;
    case YUV_CONVERSION_COMBINERS_ATI:
      glSetupYUVCombinersATI(uvcos, uvsin);
      break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP:
    case YUV_CONVERSION_FRAGMENT_LOOKUP3D:
    case YUV_CONVERSION_FRAGMENT:
    case YUV_CONVERSION_FRAGMENT_POW:
      glSetupYUVFragprog(brightness, contrast, uvcos, uvsin,
                         rgamma, ggamma, bgamma, type,
                         target == GL_TEXTURE_RECTANGLE,
                         texw, texh);
      break;
    default:
      mp_msg(MSGT_VO, MSGL_ERR, "[gl] unknown conversion type %i\n", YUV_CONVERSION(type));
  }
}

/**
 * \brief enable the specified YUV conversion
 * \param target texture target for Y, U and V textures (e.g. GL_TEXTURE_2D)
 * \param type type of YUV conversion
 * \ingroup glconversion
 */
void glEnableYUVConversion(GLenum target, int type) {
  if (type <= 0) return;
  switch (YUV_CONVERSION(type)) {
    case YUV_CONVERSION_COMBINERS:
      ActiveTexture(GL_TEXTURE1);
      glEnable(target);
      ActiveTexture(GL_TEXTURE2);
      glEnable(target);
      ActiveTexture(GL_TEXTURE0);
      glEnable(GL_REGISTER_COMBINERS_NV);
      break;
    case YUV_CONVERSION_COMBINERS_ATI:
      ActiveTexture(GL_TEXTURE1);
      glEnable(target);
      ActiveTexture(GL_TEXTURE2);
      glEnable(target);
      ActiveTexture(GL_TEXTURE0);
      glEnable(GL_FRAGMENT_SHADER_ATI);
      break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP3D:
    case YUV_CONVERSION_FRAGMENT_LOOKUP:
    case YUV_CONVERSION_FRAGMENT_POW:
    case YUV_CONVERSION_FRAGMENT:
      glEnable(GL_FRAGMENT_PROGRAM);
      break;
  }
}

/**
 * \brief disable the specified YUV conversion
 * \param target texture target for Y, U and V textures (e.g. GL_TEXTURE_2D)
 * \param type type of YUV conversion
 * \ingroup glconversion
 */
void glDisableYUVConversion(GLenum target, int type) {
  if (type <= 0) return;
  switch (YUV_CONVERSION(type)) {
    case YUV_CONVERSION_COMBINERS:
      ActiveTexture(GL_TEXTURE1);
      glDisable(target);
      ActiveTexture(GL_TEXTURE2);
      glDisable(target);
      ActiveTexture(GL_TEXTURE0);
      glDisable(GL_REGISTER_COMBINERS_NV);
      break;
    case YUV_CONVERSION_COMBINERS_ATI:
      ActiveTexture(GL_TEXTURE1);
      glDisable(target);
      ActiveTexture(GL_TEXTURE2);
      glDisable(target);
      ActiveTexture(GL_TEXTURE0);
      glDisable(GL_FRAGMENT_SHADER_ATI);
      break;
    case YUV_CONVERSION_FRAGMENT_LOOKUP3D:
    case YUV_CONVERSION_FRAGMENT_LOOKUP:
    case YUV_CONVERSION_FRAGMENT_POW:
    case YUV_CONVERSION_FRAGMENT:
      glDisable(GL_FRAGMENT_PROGRAM);
      break;
  }
}

/**
 * \brief draw a texture part at given 2D coordinates
 * \param x screen top coordinate
 * \param y screen left coordinate
 * \param w screen width coordinate
 * \param h screen height coordinate
 * \param tx texture top coordinate in pixels
 * \param ty texture left coordinate in pixels
 * \param tw texture part width in pixels
 * \param th texture part height in pixels
 * \param sx width of texture in pixels
 * \param sy height of texture in pixels
 * \param rect_tex whether this texture uses texture_rectangle extension
 * \param is_yv12 if set, also draw the textures from units 1 and 2
 * \param flip flip the texture upside down
 * \ingroup gltexture
 */
void glDrawTex(GLfloat x, GLfloat y, GLfloat w, GLfloat h,
               GLfloat tx, GLfloat ty, GLfloat tw, GLfloat th,
               int sx, int sy, int rect_tex, int is_yv12, int flip) {
  GLfloat tx2 = tx / 2, ty2 = ty / 2, tw2 = tw / 2, th2 = th / 2;
  if (!rect_tex) {
    tx /= sx; ty /= sy; tw /= sx; th /= sy;
    tx2 = tx, ty2 = ty, tw2 = tw, th2 = th;
  }
  if (flip) {
    y += h;
    h = -h;
  }
  glBegin(GL_QUADS);
  glTexCoord2f(tx, ty);
  if (is_yv12) {
    MultiTexCoord2f(GL_TEXTURE1, tx2, ty2);
    MultiTexCoord2f(GL_TEXTURE2, tx2, ty2);
  }
  glVertex2f(x, y);
  glTexCoord2f(tx, ty + th);
  if (is_yv12) {
    MultiTexCoord2f(GL_TEXTURE1, tx2, ty2 + th2);
    MultiTexCoord2f(GL_TEXTURE2, tx2, ty2 + th2);
  }
  glVertex2f(x, y + h);
  glTexCoord2f(tx + tw, ty + th);
  if (is_yv12) {
    MultiTexCoord2f(GL_TEXTURE1, tx2 + tw2, ty2 + th2);
    MultiTexCoord2f(GL_TEXTURE2, tx2 + tw2, ty2 + th2);
  }
  glVertex2f(x + w, y + h);
  glTexCoord2f(tx + tw, ty);
  if (is_yv12) {
    MultiTexCoord2f(GL_TEXTURE1, tx2 + tw2, ty2);
    MultiTexCoord2f(GL_TEXTURE2, tx2 + tw2, ty2);
  }
  glVertex2f(x + w, y);
  glEnd();
}

#ifdef GL_WIN32
#include "w32_common.h"
/**
 * \brief little helper since wglGetProcAddress definition does not fit our 
 *        getProcAddress
 * \param procName name of function to look up
 * \return function pointer returned by wglGetProcAddress
 */
static void *w32gpa(const GLubyte *procName) {
  return wglGetProcAddress(procName);
}

int setGlWindow(int *vinfo, HGLRC *context, HWND win)
{
  int new_vinfo;
  HDC windc = GetDC(win);
  HGLRC new_context = 0;
  int keep_context = 0;

  // should only be needed when keeping context, but not doing glFinish
  // can cause flickering even when we do not keep it.
  if (*context)
  glFinish();
  new_vinfo = GetPixelFormat(windc);
  if (*context && *vinfo && new_vinfo && *vinfo == new_vinfo) {
      // we can keep the wglContext
      new_context = *context;
      keep_context = 1;
  } else {
    // create a context
    new_context = wglCreateContext(windc);
    if (!new_context) {
      mp_msg(MSGT_VO, MSGL_FATAL, "[gl] Could not create GL context!\n");
      return SET_WINDOW_FAILED;
    }
  }

  // set context
  if (!wglMakeCurrent(windc, new_context)) {
    mp_msg (MSGT_VO, MSGL_FATAL, "[gl] Could not set GL context!\n");
    if (!keep_context) {
      wglDeleteContext(new_context);
    }
    return SET_WINDOW_FAILED;
  }

  // set new values
  vo_window = win;
  vo_hdc = windc;
  {
    RECT rect;
    GetClientRect(win, &rect);
    vo_dwidth = rect.right;
    vo_dheight = rect.bottom;
  }
  if (!keep_context) {
    if (*context)
      wglDeleteContext(*context);
    *context = new_context;
    *vinfo = new_vinfo;
    getFunctions(w32gpa, NULL);

    // and inform that reinit is neccessary
    return SET_WINDOW_REINIT;
  }
  return SET_WINDOW_OK;
}

void releaseGlContext(int *vinfo, HGLRC *context) {
  *vinfo = 0;
  if (*context) {
    wglMakeCurrent(0, 0);
    wglDeleteContext(*context);
  }
  *context = 0;
}

void swapGlBuffers() {
  SwapBuffers(vo_hdc);
}
#else
#ifdef HAVE_LIBDL
#include <dlfcn.h>
#endif
#include "x11_common.h"
/**
 * \brief find address of a linked function
 * \param s name of function to find
 * \return address of function or NULL if not found
 *
 * Copied from xine
 */
static void *getdladdr(const char *s) {
#ifdef HAVE_LIBDL
#if defined(__sun) || defined(__sgi)
  static void *handle = NULL;
  if (!handle)
    handle = dlopen(NULL, RTLD_LAZY);
  return dlsym(handle, s);
#else
  return dlsym(0, s);
#endif
#else
  return NULL;
#endif
}

/**
 * \brief Returns the XVisualInfo associated with Window win.
 * \param win Window whose XVisualInfo is returne.
 * \return XVisualInfo of the window. Caller must use XFree to free it.
 */
static XVisualInfo *getWindowVisualInfo(Window win) {
  XWindowAttributes xw_attr;
  XVisualInfo vinfo_template;
  int tmp;
  XGetWindowAttributes(mDisplay, win, &xw_attr);
  vinfo_template.visualid = XVisualIDFromVisual(xw_attr.visual);
  return XGetVisualInfo(mDisplay, VisualIDMask, &vinfo_template, &tmp);
}

/**
 * \brief Changes the window in which video is displayed.
 * If possible only transfers the context to the new window, otherwise
 * creates a new one, which must be initialized by the caller.
 * \param vinfo Currently used visual.
 * \param context Currently used context.
 * \param win window that should be used for drawing.
 * \return one of SET_WINDOW_FAILED, SET_WINDOW_OK or SET_WINDOW_REINIT.
 * In case of SET_WINDOW_REINIT the context could not be transfered
 * and the caller must initialize it correctly.
 * \ingroup glcontext
 */
int setGlWindow(XVisualInfo **vinfo, GLXContext *context, Window win)
{
  XVisualInfo *new_vinfo;
  GLXContext new_context = NULL;
  int keep_context = 0;

  // should only be needed when keeping context, but not doing glFinish
  // can cause flickering even when we do not keep it.
  if (*context)
  glFinish();
  new_vinfo = getWindowVisualInfo(win);
  if (*context && *vinfo && new_vinfo &&
      (*vinfo)->visualid == new_vinfo->visualid) {
      // we can keep the GLXContext
      new_context = *context;
      XFree(new_vinfo);
      new_vinfo = *vinfo;
      keep_context = 1;
  } else {
    // create a context
    new_context = glXCreateContext(mDisplay, new_vinfo, NULL, True);
    if (!new_context) {
      mp_msg(MSGT_VO, MSGL_FATAL, "[gl] Could not create GLX context!\n");
      XFree(new_vinfo);
      return SET_WINDOW_FAILED;
    }
  }

  // set context
  if (!glXMakeCurrent(mDisplay, vo_window, new_context)) {
    mp_msg (MSGT_VO, MSGL_FATAL, "[gl] Could not set GLX context!\n");
    if (!keep_context) {
      glXDestroyContext (mDisplay, new_context);
      XFree(new_vinfo);
    }
    return SET_WINDOW_FAILED;
  }

  // set new values
  vo_window = win;
  {
    Window root;
    int tmp;
    unsigned utmp;
    XGetGeometry(mDisplay, vo_window, &root, &tmp, &tmp,
        (unsigned *)&vo_dwidth, (unsigned *)&vo_dheight, &utmp, &utmp);
  }
  if (!keep_context) {
    void *(*getProcAddress)(const GLubyte *);
    const char *(*glXExtStr)(Display *, int);
    if (*context)
      glXDestroyContext(mDisplay, *context);
    *context = new_context;
    if (*vinfo)
      XFree(*vinfo);
    *vinfo = new_vinfo;
      getProcAddress = getdladdr("glXGetProcAddress");
    if (!getProcAddress)
      getProcAddress = getdladdr("glXGetProcAddressARB");
    if (!getProcAddress)
      getProcAddress = (void *)getdladdr;
    glXExtStr = getdladdr("glXQueryExtensionsString");
    getFunctions(getProcAddress, !glXExtStr ? NULL :
                 glXExtStr(mDisplay, DefaultScreen(mDisplay)));

    // and inform that reinit is neccessary
    return SET_WINDOW_REINIT;
  }
  return SET_WINDOW_OK;
}

/**
 * \brief free the VisualInfo and GLXContext of an OpenGL context.
 * \ingroup glcontext
 */
void releaseGlContext(XVisualInfo **vinfo, GLXContext *context) {
  if (*vinfo)
    XFree(*vinfo);
  *vinfo = NULL;
  if (*context)
  {
    glFinish();
    glXMakeCurrent(mDisplay, None, NULL);
    glXDestroyContext(mDisplay, *context);
  }
  *context = 0;
}

void swapGlBuffers(void) {
  glXSwapBuffers(mDisplay, vo_window);
}
#endif

