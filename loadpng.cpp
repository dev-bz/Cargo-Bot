#include <png.h>
struct pnginfo {
	int w, h, s;
	unsigned char *d;
};
struct pngbuffer {
	size_t s, o;
	const unsigned char *d;
};
void freepng(pnginfo png) {
	if (png.d)
		delete[] png.d;
}
void read_data_fn(png_structp sp, png_bytep bp, png_size_t s) {
	pngbuffer *buf = (pngbuffer *)png_get_io_ptr(sp);
	memcpy(bp, buf->d + buf->o, s);
	buf->o += s;
}
pnginfo readpng__001(const char *name,const unsigned char *data, size_t len) {
	// 前边几句是扯淡，初始化各种结构
	pnginfo ret;
	FILE *file = nullptr;
	if (name) {
		file = fopen(name, "rb");
		if (file == 0) {
			ret.d = 0;
			return ret;
		}
	}
	printf("start.load %s\n", name);
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));
	// 这句很重要
	pngbuffer tmp;
	if (file)
		png_init_io(png_ptr, file);
	else {
		tmp.d = data;
		tmp.s = len;
		tmp.o = 0;
		png_set_read_fn(png_ptr, &tmp, read_data_fn);
	}
	// 读文件了
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
	// 得到文件的宽高色深
	int m_width = png_get_image_width(png_ptr, info_ptr);
	int m_height = png_get_image_height(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);
	// 申请个内存玩玩，这里用的是c++语法，甭想再c上编过
	int c4 = 0;
	int st = 0;
	printf("w=%i,h=%i\n", m_width, m_height);
	if (PNG_COLOR_TYPE_RGBA == color_type) {
		printf("RGBA\n");
		c4 = 4;
		st = 4;
	} else if (PNG_COLOR_TYPE_RGB == color_type) {
		printf("RGB\n");
		c4 = 3;
		st = 3;
	} else if (PNG_COLOR_TYPE_GRAY_ALPHA == color_type) {
		printf("ALPHA\n");
		c4 = 1;
		st = 2;
	} else {
		printf("ColorType = %i\n", color_type);
		c4 = 3;
	}
	if (c4) {
		int size = m_height * m_width * c4;
		if (c4 == 2)
			size *= 2;
		unsigned char *bgra = // (unsigned char *)malloc(size);
				new unsigned char[size];
		int pos = 0;
		ret.w = m_width;
		ret.h = m_height;
		ret.s = c4;
		ret.d = bgra;
		if (c4 == 2)
			ret.s = 4;
		// row_pointers里边就是传说中的rgba数据了
		png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
		// 拷贝！！注意，如果你读取的png没有A通道，就要3位3位的读。还有就是注意字节对其的问题，最简单的就是别用不能被4整除的宽度就行了。读过你实在想用，就要在这里加上相关的对齐处理。
		for (int i = 0; i < m_height; i++) {
			for (int j = 0; j < (st * m_width); j += st) {
				if (st == 2)
					bgra[pos++] = row_pointers[i][j + 1]; // blue
				else
					bgra[pos++] = row_pointers[i][j + 0]; // blue
				if (ret.s > 1)
					bgra[pos++] = row_pointers[i][j + 1]; // green
				if (ret.s > 2)
					bgra[pos++] = row_pointers[i][j + 2]; // red
				if (ret.s == 4)
					bgra[pos++] = row_pointers[i][j + 3]; // alpha
			}
		}
	}
	// 好了，你可以用这个数据作任何的事情了。。。把它显示出来或者打印出来都行。
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	if (file)
		fclose(file);
	return ret;
}
/*int main() {
 readpng__001("/mnt/sdcard/ajava/lpng1610/pngnow.png");
}*/
