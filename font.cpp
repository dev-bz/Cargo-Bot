#include <GLES/gl.h>
#include <stdlib.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
static GLuint g_ftex = 0;
static stbtt_bakedchar g_cdata[96];
const int texSize = 256;
const int fontHeight = 36;
const int fontBottom=30;
static float scale = 1.0f / texSize;
extern float motion;
extern "C" float charScale(int c) {return (c < 32 || c >= 128) ? 0.0f : g_cdata[c - 32].xadvance / fontHeight; }
extern "C" int drawText(unsigned char c, float *cood, float *pos, float size) {
	glBindTexture(GL_TEXTURE_2D, g_ftex);
	if (c > 32 && c < 128) {
		stbtt_bakedchar &t = g_cdata[c - 32];
		cood[0] = scale * t.x1;
		cood[1] = scale * t.y0;
		cood[2] = scale * t.x1;
		cood[3] = scale * t.y1;
		cood[4] = scale * t.x0;
		cood[5] = scale * t.y1;
		cood[6] = scale * t.x0;
		cood[7] = scale * t.y0;
		float sc = size / fontHeight;
		float si = sc * t.xoff; // (pos[0]-pos[4]-sc*(t.x1 - t.x0))*-0.5;
		// pos[4] += si;
		// pos[6] += si; // sc * t.xoff;
		// pos[2] = pos[0] = pos[4] + sc * (t.x1 - t.x0);
		pos[2]-=si;pos[0]=pos[2];
		pos[4] = pos[6] = pos[0] - sc * (t.x1 - t.x0);
		pos[1] += sc * (-fontBottom - t.yoff);
		pos[7] += sc * (-fontBottom - t.yoff);
		pos[3] = pos[5] = pos[1] - sc * (t.y1 - t.y0);
	} else {
		cood[0] = 1;
		cood[1] = 0;
		cood[2] = 1;
		cood[3] = 1;
		cood[4] = 0;
		cood[5] = 1;
		cood[6] = 0;
		cood[7] = 0;
	}
	return g_ftex;
}
extern "C" int RenderGLInit(const char *fontpath) {
	// Load font.
	FILE *fp = fopen(fontpath, "rb");
	if (!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	int size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char *ttfBuffer = (unsigned char *)malloc(size);
	if (!ttfBuffer) {
		fclose(fp);
		return 0;
	}
	fread(ttfBuffer, 1, size, fp);
	fclose(fp);
	fp = 0;
	unsigned char *bmap = (unsigned char *)malloc(512 * 512);
	if (!bmap) {
		free(ttfBuffer);
		return 0;
	}
	int off = stbtt_GetFontOffsetForIndex(ttfBuffer, 2); // stbtt_FindMatchingFont(ttfBuffer, "Arial", STBTT_MACSTYLE_BOLD);
	if (off < 0)
		off = 0;
	stbtt_BakeFontBitmap(ttfBuffer, off, fontHeight, bmap, texSize, texSize, 32, 96, g_cdata);
	// scale=1.0f/256;
	/*for(int i=0;i<512*512;++i){
		bmap[i]=0xff;
	}*/
	// can free ttf_buffer at this point
	glGenTextures(1, &g_ftex);
	glBindTexture(GL_TEXTURE_2D, g_ftex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texSize, texSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmap);
#if 0
	int w = 512, index = 0;
	while (w > 4) {
		int W = w;
		w >>= 1;
		unsigned char *tmap = (unsigned char *)malloc(w * w);
		for (int x = 0; x < w; ++x)
			for (int y = 0; y < w; ++y) {
				int a = bmap[x + W * y + 0];
				int b = bmap[x + W * y + 1];
				int c = bmap[x + W * y + W + 0];
				int d = bmap[x + W * y + W + 1];
				tmap[x + w * y] = (a + b + c + d) / 4;
			}
		free(bmap);bmap=tmap;
		glTexSubImage2D(GL_TEXTURE_2D, ++index, GL_ALPHA, w, w, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmap);
	}
#elif 0
	int w = 512, index = 0;
	while (w > 256) {
		int W = w;
		w >>= 1;
		unsigned char *tmap = (unsigned char *)malloc(w * w);
		for (int x = 0; x < w; ++x)
			for (int y = 0; y < w; ++y) {
				int a = bmap[x + W * y + 0];
				int b = bmap[x + W * y + 1];
				int c = bmap[x + W * y + W + 0];
				int d = bmap[x + W * y + W + 1];
				tmap[x + w * y] = (a + b + c + d) / 4;
			}
		free(bmap);
		bmap = tmap;
	}
	glTexImage2D(GL_TEXTURE_2D, index, GL_ALPHA, w, w, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmap);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// can free ttf_buffer at this point
	free(ttfBuffer);
	free(bmap);
	return 1;
}
const char *testCode =R"lua(
	if currentScreen and currentScreen.bodies then
	  local mbodies = currentScreen.bodies
	  local offset=0
	  noTint()
	  fontSize(32)
	  textMode(CORNER)
	  rectMode(CENTER)
		--strokeWidth(2.0)
		--stroke(1,0.5,0,0.95)
		fill(255,128,64,128)
		for i,b in ipairs(mbodies) do
			local obj = b.body.p
			if b.body.shapeType==POLYGON then
				pushMatrix()
				translate(obj.cx,obj.cy)
				rect(0,0,16,16)
				rotate(obj.angle);
				rect(0,0,b.body.width,b.body.height)
				popMatrix()
			elseif CIRCLE == b.body.shapeType then
				pushMatrix()
				translate(obj.x,obj.y);
				--rect(0,0,16,16)
				popMatrix()
				pushMatrix()
				translate(obj.cx,obj.cy)
				rotate(obj.angle);
				rect(0,0,5,5)
				popMatrix()
				offset=offset+1
				text('radius: '..type(b.body.radius)..b.body.radius..')',10,offset*32)
			end
		end
	elseif currentScreen and currentScreen.stage and currentScreen.stage.bodies then
		textMode(CORNER)
		local offset=0
		--noFill()
		
		noTint()
		fontSize(32)
		pushMatrix()
		--text('bodies: '..#currentScreen.stage.bodies,10,32)
		for body,info in pairs(currentScreen.stage.bodies) do
			offset=offset+1
			--text(offset..': '..type(v),10,offset*32)
			if body.shapeType==POLYGON then
						local obj = body.p
												fill(64,255,64,128)
												pushMatrix()
												translate(obj.x,obj.y);
												rect(0,0,16,16)
												--rotate(obj.angle);
												--rect(0,0,body.width,body.height)
												popMatrix()
												fill(255,128,64,128)
						pushMatrix()
						translate(obj.cx,obj.cy);
						rotate(obj.angle);
						--rect(0,0,16,16)
						rect(0,0,body.width,body.height)
						popMatrix()
			end
		end
		popMatrix()
	end
)lua";
