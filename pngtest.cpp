#include <GLES/gl.h>
// #include<gl/glu.h>
#include <stdio.h>
struct pnginfo {
	int w, h, s;
	unsigned char *d;
};
void freepng(pnginfo png);
pnginfo readpng__001(const char *name);
extern "C" unsigned int d_makeTexture(int w, int h, void *d, int size) {
	unsigned int Format = GL_RGB;
	if (size == 4) {
		Format = GL_RGBA;
	} else if (size == 1) {
		Format = GL_ALPHA;
	}
	if (d) {
		GLuint t = 0;
		glGenTextures(1, &t);
		glBindTexture(GL_TEXTURE_2D, t);
		if (glIsTexture(t) == 0)
			return 0;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if 0
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST_MIPMAP_NEAREST);
#elif 0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#endif
#ifdef GLUTOOL
		int tw = w - 1, nw = 1;
		while (tw > 0) {
			tw = tw >> 1;
			nw = nw << 1;
		}
		int th = h - 1, nh = 1;
		while (th > 0) {
			th = th >> 1;
			nh = nh << 1;
		}
		printf("PNG size is {%i,%i}\n", nw, nh);
		unsigned char *nd = new unsigned char[nw * nh * size];
		gluScaleImage(Format, w, h, GL_UNSIGNED_BYTE, d, nw, nh, GL_UNSIGNED_BYTE, nd);
		glTexImage2D(GL_TEXTURE_2D, 0, Format, nw, nh, 0, Format, GL_UNSIGNED_BYTE, nd);
		delete nd;
#else
		glTexImage2D(GL_TEXTURE_2D, 0, Format, w, h, 0, Format, GL_UNSIGNED_BYTE, d);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// glTexImage2D(GL_TEXTURE_2D, 0,3, bl.mWidth, bl.mHeight, 0, Format,
		// GL_UNSIGNED_BYTE,bl.mRGB);
		// /gluBuild2DMipmaps(GL_TEXTURE_2D,bl.channels,bl.mWidth,
		// bl.mHeight,Format, GL_UNSIGNED_BYTE,bl.mRGB);
		return t;
	}
	return 0;
}
extern "C" unsigned int d_makeTextureFromFile(const char *name, unsigned int *w, unsigned int *h) {
	pnginfo png = readpng__001(name);
	if (png.d == 0)
		return 0;
	printf("PNG (%s)size is %i\n", name, png.s);
	unsigned int ret = d_makeTexture(png.w, png.h, png.d, png.s);
	freepng(png);
	if (w)
		*w = png.w;
	if (h)
		*h = png.h;
	return ret;
}
