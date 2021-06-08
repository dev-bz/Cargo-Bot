#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/log.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <errno.h>
#include <jni.h>
#include <locale.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "CARGO-BOT", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "CARGO-BOT", __VA_ARGS__))
#ifdef __cplusplus
extern "C" {
#endif
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#ifdef __cplusplus
}
#endif
#include <math.h>
struct saved_state {
	int32_t x;
	int32_t y;
	int32_t z;
};
/**
 * Shared state for our app.
 */
struct engine {
	struct android_app *app;
	ASensorManager *sensorManager;
	const ASensor *accelerometerSensor;
	ASensorEventQueue *sensorEventQueue;
	int animating;
	EGLDisplay display;
	EGLSurface surface, _surface;
	EGLContext context;
	float time;
	int32_t width;
	int32_t height;
	struct saved_state state;
};
void initPhysicsLib(lua_State *L);
void freePhysics(lua_State *L);
void drawPhysics(lua_State *L);
// #define FILLSIZE 512.0f void ResizeGL(int w, int h);
void InitLua(lua_State *L, struct engine *engine);
extern unsigned int d_makeTextureFrom(const char *name, const unsigned char *buf, size_t len, unsigned int *w, unsigned int *h);
// GLuint tex = 0;
lua_State *L;
int RenderGLInit(const char *fontpath, const unsigned char *ptr, size_t size);
int drawText(unsigned char c, float *cood, float *pos, float height);
void ResizeGL(int w, int h) {
#ifndef FULL_SCREEN
	int y = (h - w * 4 / 3) / 2;
	h = h - y - y;
	glViewport(0, y, w, h);
#else
	glViewport(0, 0, w, h);
#endif
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifndef FILLSIZE
#define FILLSIZE 384
#endif
	// glScalef((float)h / w / FILLSIZE, 1.0f / FILLSIZE, 1);
	glScalef(1.0f / FILLSIZE, (float)w / h / FILLSIZE, 1);
	glMatrixMode(GL_MODELVIEW);
}
int pushMatrix(lua_State *L) {
	glPushMatrix();
	// ++pushLevel;
	return 0;
	struct lconv *v = localeconv();
	char a = v->decimal_point[0];
}
int popMatrix(lua_State *L) {
	glPopMatrix();
	// --pushLevel;
	// printf("pushLevel= %i\n",pushLevel);
	return 0;
}
int translate(lua_State *L) {
	float x = lua_tonumber(L, -2);
	float y = lua_tonumber(L, -1);
	glTranslatef(x, y, 0);
	return 0;
}
int rotate(lua_State *L) {
	float r = lua_tonumber(L, -1);
	glRotatef(r, 0, 0, 1);
	return 0;
}
int runCount = 0;
float quadVertices[36] = {0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
float quadTexCoords[36] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
unsigned int cVertices[] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xff0000ff};
GLuint mode;
#define glVertex2f(c, d, a, b) quadVertices[c] = a, quadVertices[d] = b
#define glTexCoord2f(c, d, a, b) quadTexCoords[c] = a, quadTexCoords[d] = b
#define glBegin(m) mode = m
#define glEnd(n) glDrawArrays(mode, 0, n);
float strokec[4] = {0, 0, 0, 1};
int stroke(lua_State *L) {
	int top = lua_gettop(L);
	float r = lua_tonumber(L, -4);
	float g = lua_tonumber(L, -3);
	float b = lua_tonumber(L, -2);
	float a = lua_tonumber(L, -1);
	if (top == 4) {
		strokec[0] = r / 255;
		strokec[1] = g / 255;
		strokec[2] = b / 255;
		strokec[3] = a / 255;
	} else if (lua_istable(L, -1)) {
		lua_pushstring(L, "r");
		lua_gettable(L, -2);
		strokec[0] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
		lua_pushstring(L, "g");
		lua_gettable(L, -2);
		strokec[1] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
		lua_pushstring(L, "b");
		lua_gettable(L, -2);
		strokec[2] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
		lua_pushstring(L, "a");
		lua_gettable(L, -2);
		strokec[3] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
	}
	return 0;
}
float fillc[4] = {1, 1, 1, 1};
int fill(lua_State *L) {
	int top = lua_gettop(L);
	if (top == 4) {
		fillc[0] = lua_tonumber(L, -4) / 255;
		fillc[1] = lua_tonumber(L, -3) / 255;
		fillc[2] = lua_tonumber(L, -2) / 255;
		fillc[3] = lua_tonumber(L, -1) / 255;
	} else if (lua_istable(L, -1)) {
		lua_pushstring(L, "r");
		lua_gettable(L, -2);
		fillc[0] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
		lua_pushstring(L, "g");
		lua_gettable(L, -2);
		fillc[1] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
		lua_pushstring(L, "b");
		lua_gettable(L, -2);
		fillc[2] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
		lua_pushstring(L, "a");
		lua_gettable(L, -2);
		fillc[3] = lua_tonumber(L, -1) / 255;
		lua_pop(L, 1);
	}
	return 0;
}
int width, height;
struct engine *current = NULL;
struct TextureMap {
	GLuint tex, w, h, offset;
};
char *textureData = NULL;
size_t textureSize = 0;
struct TextureMap *maps = NULL;
size_t textures = 0;
static int MAKE_TEXTURE = -1;
int setContext(lua_State *L) {
	int nw;
	int nh;
	if (lua_gettop(L)) {
		if (current)
			eglMakeCurrent(current->display, current->_surface, current->_surface, current->context);
		glClearColor(0.f, 0.f, 0.f, .0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.125f, 0.25f, 0.75f, 1);
		if (lua_istable(L, 1)) {
			if (current) {
				lua_pushlightuserdata(L, current);
				lua_pushvalue(L, 1);
				lua_rawset(L, LUA_REGISTRYINDEX);
			}
			lua_getfield(L, 1, "width");
			int w = lua_tointeger(L, -1);
			lua_getfield(L, 1, "height");
			int h = lua_tointeger(L, -1);
			lua_pop(L, 2);
			nw = nh = 1;
			while (nw < w)
				nw = nw << 1;
			while (nh < h)
				nh = nh << 1;
			while (nw > 512)
				nw = nw >> 1;
			while (nh > 512)
				nh = nh >> 1;
			MAKE_TEXTURE = -1;
			for (int i = 0; i < textures; ++i) {
				if (maps[i].tex == -1) {
					MAKE_TEXTURE = i;
					break;
				}
			}
			if (-1 == MAKE_TEXTURE) {
				MAKE_TEXTURE = textures++;
				maps = realloc(maps, sizeof(struct TextureMap) * (textures));
				maps[MAKE_TEXTURE].tex = -1;
			}
			maps[MAKE_TEXTURE].offset = 0;
			maps[MAKE_TEXTURE].w = nw;
			maps[MAKE_TEXTURE].h = nh;
			// nw=256;nh=256;
			// glDrawBuffer();
			glViewport(0, 0, nw, nh);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glScalef(2.0f / w, -2.0f / h, 1);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			// glScalef(1/300.0f,1/300.0f,1);
			// glTranslatef(-512,512,0);
			glTranslatef(-0.5f * w, -0.5f * h, 0);
		} else {
			if (-1 == MAKE_TEXTURE) {
				lua_pushstring(L, lua_typename(L, lua_type(L, 1)));
				lua_setglobal(L, "RUNINFO");
			}
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		// glEnableClientState(GL_COLOR_ARRAY_POINTER);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY_POINTER);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, quadVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, quadTexCoords);
	} else {
		glPopMatrix();
		glFinish();
		// unsigned char *px = (unsigned char *)malloc(w * h * 4);
		// unsigned char *nd = (unsigned char *)malloc(nw * nh * 4);
		if (MAKE_TEXTURE != -1) {
			nw = maps[MAKE_TEXTURE].w;
			nh = maps[MAKE_TEXTURE].h;
			GLuint tex = -1;
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			textureData = realloc(textureData, textureSize + nw * nh * 4);
			glReadPixels(0, 0, nw, nh, GL_RGBA, GL_UNSIGNED_BYTE, textureData + textureSize);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nw, nh, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData + textureSize);
			maps[MAKE_TEXTURE].tex = tex;
			maps[MAKE_TEXTURE].offset = textureSize;
			textureSize += nw * nh * 4;
			if (current) {
				lua_pushlightuserdata(L, current);
				lua_rawget(L, LUA_REGISTRYINDEX);
				lua_pushinteger(L, MAKE_TEXTURE);
				lua_setfield(L, -2, "tex");
				lua_pop(L, 1);
				lua_pushlightuserdata(L, current);
				lua_pushnil(L);
				lua_rawset(L, LUA_REGISTRYINDEX);
			}
			MAKE_TEXTURE = -1;
		}
		if (current)
			eglMakeCurrent(current->display, current->surface, current->surface, current->context);
		ResizeGL(width, height);
	}
	return 0;
}
// int density = 360;
int glRect(lua_State *L) {
	int top = lua_gettop(L);
	// printf("%i\n",top);
	float t = lua_tonumber(L, -5) * width / 2 / FILLSIZE;
	float x = lua_tonumber(L, -4);
	float y = lua_tonumber(L, -3);
	float w = lua_tonumber(L, -2) * 0.5f;
	float h = lua_tonumber(L, -1) * 0.5f;
	// printf("%f,%f,%f,%f\n",x,y,w,h);
	glBindTexture(GL_TEXTURE_2D, 0);
#if 1
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(fillc[0], fillc[1], fillc[2], fillc[3]);
	// glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0, 1, 1, 0);
	glVertex2f(0, 1, x + w, y + h);
	// glColor3f(1.0f, 0.0f, 1.0f);
	glTexCoord2f(2, 3, 1, 1);
	glVertex2f(2, 3, x + w, y - h);
	// glColor3f(1.0f, 1.0f, 0.0f);
	glTexCoord2f(4, 5, 0, 1);
	glVertex2f(4, 5, x - w, y - h);
	// glColor3f(0.0f, 1.0f, 1.0f);
	glTexCoord2f(6, 7, 0, 0);
	glVertex2f(6, 7, x - w, y + h);
	glEnd(4);
	if (t > 0) {
		glLineWidth(t);
		t = t * 0.2f;
		glBegin(GL_LINE_LOOP);
		glColor4f(strokec[0], strokec[1], strokec[2], strokec[3]);
		glVertex2f(0, 1, x + w - t, y + h - t);
		glVertex2f(2, 3, x + w - t, y - h + t);
		glVertex2f(4, 5, x - w + t, y - h + t);
		glVertex2f(6, 7, x - w + t, y + h - t);
		glEnd(4);
	}
#endif
	return 0;
}
void testText(float timing) {
	// float x = 0, y = 0, w = size, h = size;
	glColor4f(1, 1, 1, 1);
	glBegin(GL_TRIANGLE_FAN);
	// glColor3f(1.0f, 1.0f, 1.0f);
	// glTexCoord2f(0, 1, 1, 0);
	/*glVertex2f(0, 1, x + w, y + h);
	// glColor3f(1.0f, 0.0f, 1.0f);
	// glTexCoord2f(2, 3, 1, 1);
	glVertex2f(2, 3, x + w, y - h);
	// glColor3f(1.0f, 1.0f, 0.0f);
	// glTexCoord2f(4, 5, 0, 1);
	glVertex2f(4, 5, x - w, y - h);
	// glColor3f(0.0f, 1.0f, 1.0f);
	// glTexCoord2f(6, 7, 0, 0);
	glVertex2f(6, 7, x - w, y + h);*/
	drawText(0, quadTexCoords, quadVertices, 0);
	static float times = 0;
	static float lastTime = 0;
	static int index = -1;
	times += timing;
	if (times > lastTime) {
		lastTime = times + 1;
		for (int i = 0; i < textures; ++i) {
			struct TextureMap *m = maps + ((i + index + 1) % textures);
			if (m->w && m->h) {
				char info[256];
				index = ((i + index + 1) % textures);
				sprintf(info, "%d (%d x %d)", index, maps[index].w, maps[index].h);
				lua_pushstring(L, info);
				lua_setglobal(L, "NUMTEXTURES");
				break;
			}
		}
	}
	if (index != -1)
		glBindTexture(GL_TEXTURE_2D, maps[index].tex);
	glEnd(4);
	glBindTexture(GL_TEXTURE_2D, 0);
}
int glRectText(lua_State *L) {
	int top = lua_gettop(L);
	// printf("%i\n",top);
	unsigned char c = lua_tointeger(L, -6);
	float t = lua_tonumber(L, -5) * width / 2 / FILLSIZE;
	float x = lua_tonumber(L, -4);
	float y = lua_tonumber(L, -3);
	float w = lua_tonumber(L, -2) * 0.5f;
	float h = lua_tonumber(L, -1) * 0.5f;
	// printf("%f,%f,%f,%f\n",x,y,w,h);
#if 1
	x += w;
	y -= h;
	// glBindTexture(GL_TEXTURE_2D, 0);
	if (t > 0) {
		glLineWidth(t);
		float tt = t * 0.0f;
		glBegin(GL_LINE_LOOP);
		glColor4f(strokec[0], strokec[1], strokec[2], strokec[3]);
		glVertex2f(0, 1, x + w - tt, y + h - tt);
		glVertex2f(2, 3, x + w - tt, y - h + tt);
		glVertex2f(4, 5, x - w + tt, y - h + tt);
		glVertex2f(6, 7, x - w + tt, y + h - tt);
		glEnd(4);
	}
	glColor4f(fillc[0], fillc[1], fillc[2], fillc[3]);
	glBegin(GL_TRIANGLE_FAN);
	// glColor3f(1.0f, 1.0f, 1.0f);
	// glTexCoord2f(0, 1, 1, 0);
	glVertex2f(0, 1, x + w, y + h);
	// glColor3f(1.0f, 0.0f, 1.0f);
	// glTexCoord2f(2, 3, 1, 1);
	glVertex2f(2, 3, x + w, y - h);
	// glColor3f(1.0f, 1.0f, 0.0f);
	// glTexCoord2f(4, 5, 0, 1);
	glVertex2f(4, 5, x - w, y - h);
	// glColor3f(0.0f, 1.0f, 1.0f);
	// glTexCoord2f(6, 7, 0, 0);
	glVertex2f(6, 7, x - w, y + h);
	drawText(c, quadTexCoords, quadVertices, h + h);
	glEnd(4);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (t > 0) {
		glColor4f(1, 0.25f, 0, 1.0);
		glBegin(GL_LINE_LOOP);
		glEnd(4);
	}
#endif
	return 0;
}
float offset[3];
int textureOffset(lua_State *L) {
	offset[0] = lua_tonumber(L, 1);
	offset[1] = -lua_tonumber(L, 2);
	offset[2] = 1;
	return 0;
}
int gldraw(lua_State *L) {
	int top = lua_gettop(L);
	++runCount;
	// printf("%i\n",top);
	unsigned int t = lua_tointeger(L, -5);
	float x = lua_tonumber(L, -4);
	float y = lua_tonumber(L, -3);
	float w = lua_tonumber(L, -2) * 0.5f;
	float h = lua_tonumber(L, -1) * 0.5f;
	// printf("%f,%f,%f,%f\n",x,y,w,h);
	if (maps[t].tex == 0) {
		int size = maps[t].w * maps[t].h * 4;
		if (size) {
			glGenTextures(1, &maps[t].tex);
			glBindTexture(GL_TEXTURE_2D, maps[t].tex);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// glReadBuffer(GL_BACK);
			// glReadPixels(0, 0, nw, nh, GL_RGBA, GL_UNSIGNED_BYTE, nd);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maps[t].w, maps[t].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData + maps[t].offset);
		} else {
#ifdef RESOURCE_PATH
			maps[t].tex = d_makeTextureFrom(textureData + maps[t].offset, NULL, 0, NULL, NULL);
#else
			if (current) {
				AAsset *asset = AAssetManager_open(current->app->activity->assetManager, textureData + maps[t].offset + 7, AASSET_MODE_BUFFER);
				const unsigned char *buf = (const unsigned char *)AAsset_getBuffer(asset);
				size_t len = AAsset_getLength(asset);
				maps[t].tex = d_makeTextureFrom(NULL, buf, len, NULL, NULL);
				AAsset_close(asset);
			} else {
				unsigned int ret = 0;
				char tmp[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
				glGenTextures(1, &ret);
				glBindTexture(GL_TEXTURE_2D, ret);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 4, 4, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tmp);
				glBindTexture(GL_TEXTURE_2D, 0);
				maps[t].tex = ret;
			}
#endif
		}
	}
	glBindTexture(GL_TEXTURE_2D, maps[t].tex);
	// if(!t)glColor4f(0.0f, 0.0f, 0.0f,0.45f);
	glBegin(GL_TRIANGLE_FAN);
	if (offset[2]) {
		offset[2] = 0;
		glTexCoord2f(0, 1, 1 + offset[0], offset[1]);
		glTexCoord2f(2, 3, 1 + offset[0], 1 + offset[1]);
		glTexCoord2f(4, 5, offset[0], 1 + offset[1]);
		glTexCoord2f(6, 7, offset[0], offset[1]);
	} else {
		glTexCoord2f(0, 1, 1, 0);
		glTexCoord2f(2, 3, 1, 1);
		glTexCoord2f(4, 5, 0, 1);
		glTexCoord2f(6, 7, 0, 0);
	}
	glVertex2f(0, 1, x + w, y + h);
	glVertex2f(2, 3, x + w, y - h);
	glVertex2f(4, 5, x - w, y - h);
	glVertex2f(6, 7, x - w, y + h);
	glEnd(4);
	return 0;
}
extern float charScale(int c);
int charSize(lua_State *L) {
	int c = lua_tointeger(L, -1);
	lua_pushnumber(L, charScale(c));
	return 1;
}
int tint(lua_State *L) {
	glColor4f(lua_tonumber(L, -4) / 255, lua_tonumber(L, -3) / 255, lua_tonumber(L, -2) / 255, lua_tonumber(L, -1) / 255);
	return 0;
}
int drawLine(float w, float x, float y, float x1, float y1) {
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(w);
	glBegin(GL_LINES);
	glVertex2f(0, 1, x, y);
	glVertex2f(2, 3, x1, y1);
	glEnd(2);
	// printf("%f,%f,%f,%f\n",x,y,x1,y1);
	return 0;
}
int line(lua_State *L) {
	float x = lua_tonumber(L, -4);
	float y = lua_tonumber(L, -3);
	float x1 = lua_tonumber(L, -2);
	float y1 = lua_tonumber(L, -1);
	glBegin(GL_LINES);
	glVertex2f(0, 1, x, y);
	glVertex2f(2, 3, x1, y1);
	glEnd(2);
	// printf("%f,%f,%f,%f\n",x,y,x1,y1);
	return 0;
}
int ellipse(lua_State *L) {
	float x = lua_tonumber(L, -3), x1;
	float y = lua_tonumber(L, -2), y1;
	float r = lua_tonumber(L, -1) * 0.5f;
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINE_LOOP);
	int i;
	for (i = 0; i < 18; ++i) {
		x1 = sinf(i * 3.14159f / 9) * r + x;
		y1 = cosf(i * 3.14159f / 9) * r + y;
		glVertex2f((i << 1), (i << 1) + 1, x1, y1);
	}
	glEnd(18);
	// /printf("%f,%f,%f\n",x,y,r);
	return 0;
}
int loadTexture(lua_State *L) {
	unsigned int ret = 0, iw, ih;
	if (lua_isstring(L, -1)) {
		const char *name = lua_tostring(L, -1);
		// printf("%s\n",name);
		if (strstr(name, "Cargo Bot:")) {
			char tmp[260];
			sprintf(tmp, "assets/%s.png", name + 10);
			int i = textures;
			while (--i >= 0) {
				if (maps[i].w == 0 && maps[i].h == 0 && 0 == strcmp(textureData + maps[i].offset, tmp))
					break;
			}
			if (i < 0) {
#ifdef RESOURCE_PATH
				ret = d_makeTextureFrom(tmp, NULL, 0, &iw, &ih);
#else
				if (current) {
					AAsset *asset = AAssetManager_open(current->app->activity->assetManager, tmp + 7, AASSET_MODE_BUFFER);
					const unsigned char *buf = (const unsigned char *)AAsset_getBuffer(asset);
					size_t len = AAsset_getLength(asset);
					ret = d_makeTextureFrom(NULL, buf, len, &iw, &ih);
					AAsset_close(asset);
				} else {
					char tmp[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
					glGenTextures(1, &ret);
					ih = iw = 4;
					glBindTexture(GL_TEXTURE_2D, ret);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, iw, ih, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tmp);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
#endif
				maps = realloc(maps, sizeof(struct TextureMap) * (1 + textures));
				maps[textures].tex = ret;
				maps[textures].w = maps[textures].h = 0;
				maps[textures].offset = textureSize;
				textureData = realloc(textureData, textureSize + 1 + strlen(tmp));
				strcpy(textureData + textureSize, tmp);
				textureSize += 1 + strlen(tmp);
				ret = textures;
				++textures;
			} else
				ret = i;
		}
		//
	} // else ret=tex;
	lua_pushnumber(L, iw);
	lua_pushnumber(L, ih);
	lua_pushnumber(L, ret);
	return 3;
}
// #define BOX_DEBUG
#include "b64/cdecode.h"
#include "sound.h"
long readTableIntoSFXRInstance(struct lua_State *L, int index);
int sound(lua_State *L) {
	int top = lua_gettop(L);
	if (lua_isnumber(L, -top)) {
		switch (lua_tointeger(L, -top)) {
		case 0: // ENCODE
			/*lua_pushinteger(L, encodes);
			lua_setglobal(L, "SAVE_CODE");*/
			lua_pushvalue(L, 1 - top);
			return 1;
		case 1: // DATA
		{
			lua_Number vol = 1.0;
			if (top > 2 && lua_isnumber(L, 2 - top)) {
				vol = lua_tonumber(L, 2 - top);
			}
			if (top > 1) {
				if (lua_istable(L, 2)) {
					if (vol > 0.1) {
						long hash = readTableIntoSFXRInstance(L, 2);
						int len = Sound_play(NULL, hash, vol);
#ifdef BOX_DEBUG
						lua_pushinteger(L, len);
						lua_setglobal(L, "SAVE_CODE");
#endif
					}
				} else {
					const char *data = lua_tostring(L, 1 - top);
					int len = Sound_play(data, strlen(data), vol);
#ifdef BOX_DEBUG
					lua_pushinteger(L, len);
					lua_setglobal(L, "SAVE_CODE");
#endif
					/*
					lua_pushvalue(L, 1-top);
					lua_setglobal(L, "SAVE_SIZE");*/
				}
			}
		} break;
			/*case 2: // SOUND_SHOOT
				break;
			case 3: // SOUND_RANDOM
				break;
			case 4: // SOUND_POWERUP
				break;
			case 5: // SOUND_PICKUP
				break;
			case 6: // SOUND_JUMP
				break;
			case 7: // SOUND_HIT
				break;
			case 8: // SOUND_EXPLODE
				break;
			case 9: // SOUND_BLIT
				break;*/
		}
	}
	return 0;
}
int supportedOrientations(lua_State *L) { return 0; }
void require_code(lua_State *L, const char *module, struct engine *engine, int preload) {
	char path[256];
#ifndef RESOURCE_PATH
	if (strcmp(module, "def") && strcmp(module, "test"))
		sprintf(path, "CargoDroid.codea/%s.lua", module);
	else
		sprintf(path, "preload/%s.lua", module);
	AAsset *asset = AAssetManager_open(engine->app->activity->assetManager, path, AASSET_MODE_BUFFER);
	const char *source = AAsset_getBuffer(asset);
	off_t len = AAsset_getLength(asset);
	luaL_loadbufferx(L, source, len, module, "t");
	lua_pcall(L, 0, LUA_MULTRET, 0);
	AAsset_close(asset);
#else
	if (strcmp(module, "def") && strcmp(module, "test"))
		sprintf(path, "assets/CargoDroid.codea/%s.lua", module);
	else
		sprintf(path, "assets/preload/%s.lua", module);
	luaL_loadfile(L, path);
	lua_pcall(L, 0, LUA_MULTRET, 0);
#endif
	if (preload) {
		sprintf(path, "package.preload['%s']=print", module);
		luaL_dostring(L, path);
	}
}
int hasError = 0;
#define registerFunc(func)      \
	lua_getglobal(L, #func);      \
	if (lua_isnil(L, -1)) {       \
		lua_pushcfunction(L, func); \
		lua_setglobal(L, #func);    \
	}                             \
	lua_pop(L, 1)
void InitLua(lua_State *L, struct engine *engine) {
	initPhysicsLib(L);
	registerFunc(setContext);
	registerFunc(glRect);
	registerFunc(glRectText);
	registerFunc(charSize);
	registerFunc(stroke);
	registerFunc(fill);
	registerFunc(gldraw);
	registerFunc(textureOffset);
	registerFunc(pushMatrix);
	registerFunc(popMatrix);
	registerFunc(translate);
	registerFunc(rotate);
	registerFunc(tint);
	registerFunc(line);
	registerFunc(ellipse);
	registerFunc(loadTexture);
	registerFunc(sound);
	registerFunc(supportedOrientations);
#undef registerFunc
	luaL_openlibs(L);
	lua_settop(L, 0);
#ifndef RESOURCE_PATH
#define require_code(n) n,
	const char *requires[] = {"def", require_code("IO") require_code("Tweener") require_code("Events") require_code("Table") require_code("PositionObj") require_code("RectObj") require_code("SpriteObj") require_code("ShadowObj") require_code("Button") require_code("Command") require_code("Sounds") require_code("ABCMusic") require_code("ABCMusicData") require_code("Music") require_code("Panel") require_code("Smoke") require_code("Crate") require_code("BaseStage") require_code("Stage") require_code("StagePhysics") require_code("Goal") require_code("Toolbox") require_code("Program") require_code("Register") require_code("StageWall") require_code("Claw") require_code("Pile") require_code("Screen") require_code("WinScreen") require_code("CreditsScreen") require_code("ScrollingTexture") require_code("ShakeDetector") require_code("HowScreen") require_code("BaseSelect") require_code("LevelSelect") require_code("PackSelect") require_code("StartScreen") require_code("TransitionScreen") require_code("SplashScreen") require_code("Level") require_code("Tutorial") require_code("Levels") require_code("Main") require_code("Stack") require_code("Popover") NULL};
#undef require_code
	for (int i = 0; requires[i]; ++i) {
		require_code(L, requires[i], engine, 1);
	}
#endif
	require_code(L, "test", engine, 0);
	if (lua_gettop(L)) {
		lua_setglobal(L, "RUNINFO");
		hasError = 1;
	} else {
		// /printf("error->%i\n",lua_error(L));
		luaL_dostring(L, "setup()");
		if (lua_gettop(L)) {
			lua_setglobal(L, "RUNINFO");
			hasError = 1;
		}
	}
}
// #include "/sdcard/_ws/mainws.h"
void init() {
	hasError = 0;
	LOGI("ENTER LOOP");
	Sound_init();
	// init_dll();
}
void step() {
	// step_dll();
}
void quit() {
	if (L) {
		freePhysics(L);
		lua_close(L);
		L = NULL;
	}
	Sound_exit();
	if (textureData) {
		free(textureData);
		textureData = NULL;
		textureSize = 0;
	}
	if (maps) {
		free(maps);
		maps = NULL;
		textures = 0;
	}
	// exit_dll();
	LOGI("END LOOP");
}
GLint w, h;
/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine *engine) {
	// initialize OpenGL ES and EGL
	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
	EGLint dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	/* Here, the application chooses the configuration it desires. In this
		 sample, we have a very simplified selection process, where we pick the
		 first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed
		 to be accepted by ANativeWindow_setBuffersGeometry(). As soon as we picked
		 a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match,
		 using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	{
		attribs[1] = EGL_PBUFFER_BIT;
		/*attribs[10] = EGL_WIDTH;
		attribs[11] = 512;
		attribs[12] = EGL_HEIGHT;
		attribs[13] = 512;*/
		// eglChooseConfig(display, attribs, &config, 1, &numConfigs);
		// eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
		EGLint _attribs[] = {EGL_WIDTH, 512, EGL_HEIGHT, 512, EGL_NONE};
		engine->_surface = eglCreatePbufferSurface(display, config, _attribs);
		if (engine->_surface) {
			eglQuerySurface(display, engine->_surface, EGL_WIDTH, &w);
			eglQuerySurface(display, engine->_surface, EGL_HEIGHT, &h);
		} else {
			EGLint err = eglGetError();
			const char *s = eglQueryString(display, err);
			LOGW("Unable to eglCreatePbufferSurface: %s", s);
			return -1;
		}
	}
	// engine->_surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);
	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	width = w;
	height = h;
	// engine->state.angle = 0;
	engine->animating = 1;
	// Initialize GL state.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	// glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef RESOURCE_PATH
	RenderGLInit("assets/Roboto-Black.ttf", NULL, 0);
#else
	AAsset *asset = AAssetManager_open(engine->app->activity->assetManager, "Roboto-Black.ttf", AASSET_MODE_BUFFER);
	RenderGLInit(NULL, AAsset_getBuffer(asset), AAsset_getLength(asset));
	AAsset_close(asset);
#endif
	if (NULL == L) {
		L = luaL_newstate();
		if (engine->app->activity->internalDataPath) {
			lua_pushstring(L, engine->app->activity->internalDataPath);
			lua_setglobal(L, "INTERNALDATAPATH");
		}
		if (engine->app->activity->externalDataPath) {
			lua_pushstring(L, engine->app->activity->externalDataPath);
			lua_setglobal(L, "EXTERNALDATAPATH");
		}
		InitLua(L, engine);
	}
	ResizeGL(w, h);
	return 0;
}
/**
 * Just the current frame in the display.
 */
float *fline = 0;
int nline = 0;
extern const char *testCode;
static void engine_draw_frame(struct engine *engine) {
	if (engine->display == NULL) {
		// No display.
		return;
	}
	// Just fill the screen with a color.
	current = engine;
	/*if (hasError) {
		glClearColor(0.75f, 0.25f, 0.125f, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(engine->display, engine->surface);
		return;
	}*/
	// glClearColor(0.375f, 0.25f, 0.5f, 1);
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		// glMatrixMode(GL_PROJECTION);
		// glLoadIdentity();
		// glScalef(1, 9.0f / 16, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// glScalef(50,50,1);
		// glRotatef(engine->state.angle, 0, 0, 1);
		glEnableClientState(GL_VERTEX_ARRAY);
		// glEnableClientState(GL_COLOR_ARRAY_POINTER);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY_POINTER);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, quadVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, quadTexCoords);
		// glBindTexture(GL_TEXTURE_2D, tex);
#if 1
		glPushMatrix();
#if 0

 // glRotatef(theta, 0.0f, 0.0f, 1.0f);
 // glLoadIdentity();
 glScalef(300, 300, 1);
 glBindTexture(GL_TEXTURE_2D, tex);
 glBegin(GL_TRIANGLE_FAN);
 glTexCoord2f(0, 1, 1.0f, 0.0f);
 glVertex2f(0, 1, 1.0f, 1.0f);
 glTexCoord2f(2, 3, 1.0f, 1.0f);
 glVertex2f(2, 3, 1.0f, -1.0f);
 glTexCoord2f(4, 5, 0.0f, 1.0f);
 glVertex2f(4, 5, -1.0f, -1.0f);
 glTexCoord2f(6, 7, 0.0f, 0.0f);
 glVertex2f(6, 7, -1.0f, 1.0f);
 glEnd(4);
#else
		glTranslatef(-384, -512, 0);
		if (hasError) {
			luaL_dostring(L, "noTint() noFill() textMode(CORNER) fontSize(32) textWrapWidth(768)");
			luaL_dostring(L, "if NUMTEXTURES then text(NUMTEXTURES,10,64) end");
			luaL_dostring(L, "if RUNINFO then text(RUNINFO,10,96) end");
		} else {
			lua_pushnumber(L, (engine->time < 50 ? engine->time : 50) * 0.001);
			lua_setglobal(L, "DeltaTime");
			luaL_dostring(L, "ElapsedTime=ElapsedTime+DeltaTime\nphysics:step()");
#if 1
			luaL_dostring(L, "ok, errors = xpcall(draw,debug.traceback)");
			lua_getglobal(L, "errors");
			if (lua_isnil(L, -1)) {
			} else {
				if (lua_isstring(L, -1)) {
					/*FILE *f = fopen("/sdcard/log.txt", "wb");
					fprintf(f, "%s", lua_tostring(L, -1));
					fclose(f);*/
					lua_setglobal(L, "NUMTEXTURES");
				}
				hasError = 1;
			}
			lua_pop(L, 1);
#else
			lua_getglobal(L, "draw");
			if (lua_isfunction(L, -1)) {
				lua_pcall(L, 0, 0, 0);
				if (lua_gettop(L)) {
					/*FILE *f = fopen("/sdcard/log.txt", "wb");
					fprintf(f, "[%s]", lua_tostring(L, -1));
					fclose(f);*/
					hasError = 1;
				}
			} else
				lua_pop(L, 1);
#endif
#ifndef EXPORT_TO_APK
			if (engine->state.z) {
				// testText((engine->time < 50 ? engine->time : 50) * 0.001);
				luaL_dostring(L, "noTint() noFill() textMode(CORNER) fontSize(32) textWrapWidth(1000)");
				luaL_dostring(L, "if NUMTEXTURES then text(NUMTEXTURES,10,64) end");
				luaL_dostring(L, "if RUNINFO then text(RUNINFO,10,96) end");
			}
#endif
		}
#endif
#ifdef BOX_DEBUG
		glBindTexture(GL_TEXTURE_2D, 0);
		drawPhysics(L);
		luaL_dostring(L, "noTint() noFill() textMode(CORNER) fontSize(32) textWrapWidth(768)");
		// luaL_dostring(L, "if currentMusic then text('currentMusic',10,32) else text('no currentMusic',10,32) end");
		luaL_dostring(L, "if EXTERNALDATAPATH then text(EXTERNALDATAPATH,10,64) end");
		luaL_dostring(L, "if INTERNALDATAPATH then text(INTERNALDATAPATH,10,96) end");
// luaL_dostring(L, "text(SAVE_DATA or 'nil',10,64)");
#endif
		glPopMatrix();
#endif
	}
	// glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	// glEnableClientState(GL_COLOR_ARRAY);
	// glColorPointer(4, GL_UNSIGNED_BYTE, 0, cVertices);
	/*glVertexPointer(2, GL_FLOAT, 0, fline);
	glScalef(300, 300, 1);
	int i;
	for (i = 0; i < nline; ++i) {
		glDrawArrays(GL_LINE_STRIP, i * 32, 30);
	}
	glDisableClientState(GL_VERTEX_ARRAY_POINTER);
	glDisableClientState(GL_COLOR_ARRAY);*/
	eglSwapBuffers(engine->display, engine->surface);
}
/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine *engine) {
	for (size_t i = 0; i < textures; ++i) {
		if (maps[i].tex) {
			glBindTexture(EGL_TEXTURE_2D, maps[i].tex);
			glDeleteTextures(1, &maps[i].tex);
			maps[i].tex = 0;
		}
	}
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->_surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->_surface);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
	engine->_surface = EGL_NO_SURFACE;
}
/**
 * Process the next input event.
 */
void updataline(struct engine *e, float *tline, float x, float y) {
	int i;
	for (i = 0; i < 31; ++i) {
		tline[i * 2 + 0] = tline[i * 2 + 2];
		tline[i * 2 + 1] = tline[i * 2 + 3];
	}
	tline[62] = x * 2 / e->width - 1;
	tline[63] = -y * 2 / e->width + 16.0f / 9;
}
void setline(struct engine *e, float *tline, float x, float y) {
	int i;
	for (i = 0; i < 32; ++i) {
		tline[i * 2 + 0] = x * 2 / e->width - 1;
		tline[i * 2 + 1] = -y * 2 / e->width + 16.0f / 9;
	}
}
void touch(int x, int y, const char *state) {
	// y=(FILLSIZE+512)-((FILLSIZE+FILLSIZE)/h)*y;
	// x=((FILLSIZE+FILLSIZE)/h)*x-(FILLSIZE*w/h-384);
	x = x * (FILLSIZE + FILLSIZE) / w;
	y = ((h >> 1) - y) * (FILLSIZE + FILLSIZE) / w + 512;
	// printf("Top=%i\n",lua_gettop(L));
	lua_getglobal(L, "touch");
	lua_pushstring(L, "state");
	lua_getglobal(L, state);
	lua_settable(L, -3);
	lua_pushstring(L, "x");
	lua_pushinteger(L, x);
	lua_settable(L, -3);
	lua_pushstring(L, "y");
	lua_pushinteger(L, y);
	lua_settable(L, -3);
	lua_pop(L, 1);
	// printf("Top=%i\n",lua_gettop(L));
	luaL_dostring(L, "if touch and touched then touched(touch) end");
	if (lua_gettop(L)) {
		hasError = 1;
		lua_setglobal(L, "RUNINFO");
	}
}
static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
	struct engine *engine = (struct engine *)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		int e = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
		if (engine->animating) {
			engine->state.x = AMotionEvent_getX(event, 0);
			engine->state.y = AMotionEvent_getY(event, 0);
			switch (e) {
			case AMOTION_EVENT_ACTION_DOWN:
				touch(engine->state.x, engine->state.y, "BEGAN");
				break;
			case AMOTION_EVENT_ACTION_UP:
				touch(engine->state.x, engine->state.y, "ENDED");
				break;
			case AMOTION_EVENT_ACTION_MOVE:
				touch(engine->state.x, engine->state.y, "MOVING");
				break;
			}
		} else if (e == AMOTION_EVENT_ACTION_UP) {
			engine->animating = 1;
		}
	} else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
		if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP)
			if (AKeyEvent_getKeyCode(event) == AKEYCODE_MENU) {
				engine->state.z = !engine->state.z;
			}
	}
	return 0;
}
/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
	struct engine *engine = (struct engine *)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state *)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
		}
		// Also stop animating.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}
/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
unsigned int timeGet() {
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
	return start.tv_sec * 1000 + start.tv_nsec / CLOCKS_PER_SEC;
}
#include <unistd.h>
void android_main(struct android_app *state) {
	struct engine engine;
	// Make sure glue isn't stripped.
	app_dummy();
#ifdef RESOURCE_PATH
	chdir(RESOURCE_PATH);
#endif
	// density = AConfiguration_getDensity(state->config);
	// SDL_Init( SDL_INIT_VIDEO );
	memset(&engine, 0, sizeof(engine));
	engine.time = 0.05f;
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;
	// Prepare to monitor accelerometer
	/*engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);*/
	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state *)state->savedState;
	}
	// loop waiting for stuff to do.
	init();
	unsigned int startTime = timeGet();
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source *source;
		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void **)&source)) >= 0) {
			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}
			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
						/* LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x,
							 event.acceleration.y, event.acceleration.z); */
					}
				}
			}
			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				if (fline)
					free(fline);
				fline = 0;
				nline = 0;
				quit();
				return;
			}
		}
		if (engine.animating) {
			// Done with events; draw next animation frame.
			unsigned int endTime = timeGet();
			engine.time = (endTime - startTime);
			step();
			startTime = endTime;
			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			engine_draw_frame(&engine);
		}
	}
	quit();
}
