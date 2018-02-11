#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <time.h>
//for SSE2
#include <intrin.h>
//for AVX
#include <immintrin.h>
//for MMX
#include <mmintrin.h>

using namespace std;

#define WIDTH 1920
#define HEIGHT 1080

class YUV{
public:
	int width, height;
	int size;
	unsigned char *y, *u, *v;

	YUV(int width_, int height_){
		width=width_;
		height=height_;
		size=width*height;
		y=new unsigned char[size+1];
		u=new unsigned char[(size>>2)+1];
		v=new unsigned char[(size>>2)+1];
		memset(y,0,size+1);
		memset(u,0,(size >> 2)+1);
		memset(v,0,(size >> 2)+1);
	}

	~YUV(){
		delete y;
		delete u;
		delete v;
	}

	void load_yuv(FILE *fp){
		fseek(fp,0,SEEK_SET);
		fread(y,sizeof(unsigned char),size,fp);
		fseek(fp,width*height,SEEK_SET);
		fread(u,sizeof(unsigned char),size>>2,fp);
		fseek(fp,width*height*5/4,SEEK_SET);
		fread(v,sizeof(unsigned char),size>>2,fp);
	}

	void store_yuv(FILE *fp){
		fwrite(y,sizeof(unsigned char),size,fp);
		fwrite(u,sizeof(unsigned char),size>>2,fp);
		fwrite(v,sizeof(unsigned char),size>>2,fp);
	}
};

class YUV_Video{
public:
	int num, idx;
	int width, height;
	YUV **video;

	YUV_Video(int num_, int width_, int height_){
		num=num_;
		idx=0;
		width=width_;
		height=height_;
		video=new YUV*[num];
		for(int i=0;i<num;++i){
			*(video+i)=new YUV(width,height); 
		}
	}

	~YUV_Video(){
		delete video;
	}

	void add_yuv(YUV& yuv){
		memcpy((**(video+idx)).y,yuv.y,height*width);
		memcpy((**(video+idx)).u,yuv.u,width*height/4);
		memcpy((**(video+idx)).v,yuv.v,width*height/4);
		idx++;
	}

	void store_yuv_video(FILE *fp){
		for(int i=0;i<num;++i){
			fwrite((*(video+i))->y,sizeof(unsigned char),width*height,fp);
			fwrite((*(video+i))->u,sizeof(unsigned char),width*height/4,fp);
			fwrite((*(video+i))->v,sizeof(unsigned char),width*height/4,fp);
		}
	}
};



void fade();
void merge();

/* Simple */
unsigned char clip(int t);
void get_yuv(YUV& src, int i, int j, unsigned char& y, unsigned char& u, unsigned char&v);
void yuv2rgb(unsigned char y, unsigned char u, unsigned char v, unsigned char& r, unsigned char& g, unsigned char& b);
void alpha_mix(unsigned char& r, unsigned char& g, unsigned char& b, int A);
void double_mix(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char& r, unsigned char& g, unsigned char& b, int A);
void rgb2yuv(unsigned char r, unsigned char g, unsigned char b, unsigned char& y, unsigned char& u, unsigned char& v);
void save_yuv(YUV& dst, int i, int j, unsigned char y, unsigned char u, unsigned char v);
void yuv_fade(YUV& src, YUV &dst, int A);
void yuv_merge(YUV& src1, YUV& src2, YUV& dst, int A);

/* MMX */
void get_yuv_MMX(YUV& src, int i, int j, __m64& y, __m64& u, __m64& v);
void yuv2rgb_MMX(__m64& y, __m64& u, __m64& v, __m64& r, __m64& g, __m64& b);
void alpha_mix_MMX(__m64& r, __m64& g, __m64& b, int A);
void double_mix_MMX(__m64& r1, __m64& g1, __m64& b1, __m64& r2, __m64& g2, __m64& b2, int A);
void rgb2yuv_MMX(__m64& r, __m64& g, __m64& b, __m64& y, __m64& u, __m64& v);
void save_yuv_MMX(YUV& dst, int i, int j, __m64& y, __m64& u, __m64& v);
void yuv_fade_MMX(YUV& src, YUV &dst, int A);
void yuv_merge_MMX(YUV& src1, YUV& src2, YUV &dst, int A);

/* SSE2 */
void get_yuv_SSE(YUV& src, int i, int j, __m128i& y, __m128i& u, __m128i& v);
void yuv2rgb_SSE(__m128i& y, __m128i& u, __m128i& v, __m128i& r, __m128i& g, __m128i& b);
void alpha_mix_SSE(__m128i& r, __m128i& g, __m128i& b, int A);
void double_mix_SSE(__m128i& r1, __m128i& g1, __m128i& b1, __m128i& r2, __m128i& g2, __m128i& b2, int A);
void rgb2yuv_SSE(__m128i& r, __m128i& g, __m128i& b, __m128i& y, __m128i& u, __m128i& v);
void save_yuv_SSE(YUV& dst, int i, int j, __m128i& y, __m128i& u, __m128i& v);
void yuv_fade_SSE(YUV& src, YUV &dst, int A);
void yuv_merge_SSE(YUV& src1, YUV& src2, YUV &dst, int A);

/* AVX */
void get_yuv_AVX(YUV& src, int i, int j, __m256i& y, __m256i& u, __m256i& v);
void yuv2rgb_AVX(__m256i& y, __m256i& u, __m256i& v, __m256i& r, __m256i& g, __m256i& b);
void alpha_mix_AVX(__m256i& r, __m256i& g, __m256i& b, int A);
void double_mix_AVX(__m256i& r1, __m256i& g1, __m256i& b1, __m256i& r2, __m256i& g2, __m256i& b2, int A);
void rgb2yuv_AVX(__m256i& r, __m256i& g, __m256i& b, __m256i& y, __m256i& u, __m256i& v);
void save_yuv_AVX(YUV& dst, int i, int j, __m256i& y, __m256i& u, __m256i& v);
void yuv_fade_AVX(YUV& src, YUV &dst, int A);
void yuv_merge_AVX(YUV& src1, YUV& src2, YUV &dst, int A);

int main(){
	
	
	fade();
	merge();
	
	system("pause");
	return 0;
}

void fade(){
	FILE *input_file=fopen("dem1.yuv","r");
	FILE *output_file=fopen("fade.yuv","w");
	YUV src(WIDTH,HEIGHT);
	YUV dst(WIDTH,HEIGHT);
	YUV_Video video(85,WIDTH,HEIGHT);
	src.load_yuv(input_file);

	clock_t start, end;
	start = clock();
	for(int i=1;i<256;i+=3){
		yuv_fade_SSE(src,dst,i);
		video.add_yuv(dst);
		//cout<<"fade:"<<i<<endl;
	}
	end = clock();
	//cout << end - start << " " << CLOCKS_PER_SEC << " " << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << endl;

	video.store_yuv_video(output_file);
}

void merge(){
	FILE *input_file1=fopen("dem1.yuv","r");
	FILE *input_file2=fopen("dem2.yuv","r");
	FILE *output_file=fopen("merge.yuv","w");
	YUV src1(WIDTH,HEIGHT), src2(WIDTH,HEIGHT);
	YUV dst(WIDTH,HEIGHT);
	YUV_Video video(85,WIDTH,HEIGHT);
	src1.load_yuv(input_file1);
	src2.load_yuv(input_file2);

	clock_t start, end;
	start = clock();
	for(int i=1;i<256;i+=3){
		yuv_merge_SSE(src1,src2,dst,i);
		video.add_yuv(dst);
		//cout<<"merge:"<<i<<endl;
	}
	end = clock();
	//cout << end - start << " " << CLOCKS_PER_SEC << " " << (double)(end - start) / CLOCKS_PER_SEC << endl;
	cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << endl;

	video.store_yuv_video(output_file);
}

unsigned char clip(int t) {
	if (t < 0)
		return 0;
	else if (t > 255)
		return 255;
	else
		return t;
}

void get_yuv(YUV& src, int i, int j, unsigned char& y, unsigned char& u, unsigned char& v) {
	int index = (i / 2)*(WIDTH / 2) + (j / 2);
	y = *(src.y + i*WIDTH + j);
	u = *(src.u + index);
	v = *(src.v + index);
}

void yuv2rgb(unsigned char y, unsigned char u, unsigned char v, unsigned char& r, unsigned char& g, unsigned char& b) {
	r = clip((296 * y + 411 * v - 57344) >> 8);
	g = clip((299 * y - 101 * u - 211 * v + 34739) >> 8);
	b = clip((299 * y + 519 * u - 71117) >> 8);
}

void alpha_mix(unsigned char& r, unsigned char& g, unsigned char& b, int A) {
	r = A*r / 256;
	g = A*g / 256;
	b = A*b / 256;
}

void double_mix(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char& r, unsigned char& g, unsigned char& b, int A) {
	r = (r1*A + r2*(256 - A)) / 256;
	g = (g1*A + g2*(256 - A)) / 256;
	b = (b1*A + b2*(256 - A)) / 256;
}

void rgb2yuv(unsigned char r, unsigned char g, unsigned char b, unsigned char& y, unsigned char& u, unsigned char& v) {
	y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
	u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
	v = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
}

void save_yuv(YUV& dst, int i, int j, unsigned char y, unsigned char u, unsigned char v) {
	int index = (i / 2)*(WIDTH / 2) + (j / 2);
	*(dst.y + i*WIDTH + j) = y;
	*(dst.u + index) = u;
	*(dst.v + index) = v;
}

void yuv_fade(YUV& src, YUV& dst, int A) {
	unsigned char y, u, v, r, g, b;
	for (int i = 0;i<HEIGHT;++i) {
		for (int j = 0;j<WIDTH;++j) {
			get_yuv(src, i, j, y, u, v);
			yuv2rgb(y, u, v, r, g, b);
			alpha_mix(r, g, b, A);
			rgb2yuv(r, g, b, y, u, v);
			save_yuv(dst, i, j, y, u, v);
		}
	}
}

void yuv_merge(YUV& src1, YUV& src2, YUV& dst, int A) {
	unsigned char y, u, v, r, g, b;
	unsigned char y1, u1, v1, r1, g1, b1;
	unsigned char y2, u2, v2, r2, g2, b2;
	for (int i = 0;i<HEIGHT;++i) {
		for (int j = 0;j<WIDTH;++j) {
			get_yuv(src1, i, j, y1, u1, v1);
			yuv2rgb(y1, u1, v1, r1, g1, b1);
			get_yuv(src2, i, j, y2, u2, v2);
			yuv2rgb(y2, u2, v2, r2, g2, b2);
			double_mix(r1, g1, b1, r2, g2, b2, r, g, b, A);
			rgb2yuv(r, g, b, y, u, v);
			save_yuv(dst, i, j, y, u, v);
		}
	}
}


void get_yuv_MMX(YUV& src, int i, int j, __m64& y, __m64& u, __m64& v) {
	unsigned char y1, y2, y3, y4, u1, v1;
	int index;

	index = (i / 2)*(WIDTH / 2) + (j / 2);

	y1 = *(src.y + i*WIDTH + j);
	y2 = *(src.y + i*WIDTH + (j + 1));
	y3 = *(src.y + (i + 1)*WIDTH + j);
	y4 = *(src.y + (i + 1)*WIDTH + (j + 1));

	u1 = *(src.u + index);
	v1 = *(src.v + index);

	y = _mm_set_pi16(y1, y2, y3, y4);
	u = _mm_set1_pi16(u1);
	v = _mm_set1_pi16(v1);
}

void yuv2rgb_MMX(__m64& y, __m64& u, __m64& v, __m64& r, __m64& g, __m64& b) {
	__m64 tmp, tmp2;

	r = _mm_set1_pi16(296);
	r = _mm_mullo_pi16(r, y);
	tmp = _mm_set1_pi16(411);
	tmp = _mm_mullo_pi16(tmp, v);
	r = _mm_add_pi16(r, tmp);
	tmp = _mm_set1_pi16(57344);
	r = _mm_sub_pi16(r, tmp);
	r = _mm_srli_pi16(r, 8);

	g = _mm_set1_pi16(299);
	g = _mm_mullo_pi16(g, y);
	tmp = _mm_set1_pi16(101);
	tmp = _mm_mullo_pi16(tmp, u);
	g = _mm_sub_pi16(g, tmp);
	tmp = _mm_set1_pi16(211);
	tmp = _mm_mullo_pi16(tmp, v);
	g = _mm_sub_pi16(g, tmp);
	tmp = _mm_set1_pi16(34739);
	g = _mm_add_pi16(g, tmp);
	g = _mm_srli_pi16(g, 8);

	b = _mm_set1_pi16(299);
	b = _mm_mullo_pi16(b, y);
	tmp = _mm_set1_pi16(519);
	tmp = _mm_mullo_pi16(tmp, v);
	b = _mm_add_pi16(b, tmp);
	tmp = _mm_set1_pi16(71117);
	b = _mm_sub_pi16(b, tmp);
	b = _mm_srli_pi16(b, 8);

	//ensure under 256
	tmp = _mm_set1_pi16(255);
	tmp2 = _mm_sub_pi16(r, tmp);
	tmp = _mm_cmpgt_pi16(r, tmp);
	tmp = _mm_srli_pi16(tmp, 31);
	tmp = _mm_mullo_pi16(tmp2, tmp);
	r = _mm_sub_pi16(r, tmp);

	tmp = _mm_set1_pi16(255);
	tmp2 = _mm_sub_pi16(g, tmp);
	tmp = _mm_cmpgt_pi16(g, tmp);
	tmp = _mm_srli_pi16(tmp, 31);
	tmp = _mm_mullo_pi16(tmp2, tmp);
	g = _mm_sub_pi16(g, tmp);

	tmp = _mm_set1_pi16(255);
	tmp2 = _mm_sub_pi16(b, tmp);
	tmp = _mm_cmpgt_pi16(b, tmp);
	tmp = _mm_srli_pi16(tmp, 31);
	tmp = _mm_mullo_pi16(tmp2, tmp);
	b = _mm_sub_pi16(b, tmp);

	//ensure above 0
	tmp = _mm_set1_pi16(0);
	tmp2 = _mm_sub_pi16(tmp, r);
	tmp = _mm_cmpgt_pi16(tmp, r);
	tmp = _mm_srli_pi16(tmp, 31);
	tmp = _mm_mullo_pi16(tmp2, tmp);
	r = _mm_add_pi16(r, tmp);

	tmp = _mm_set1_pi16(0);
	tmp2 = _mm_sub_pi16(tmp, g);
	tmp = _mm_cmpgt_pi16(tmp, g);
	tmp = _mm_srli_pi16(tmp, 31);
	tmp = _mm_mullo_pi16(tmp2, tmp);
	g = _mm_add_pi16(g, tmp);

	tmp = _mm_set1_pi16(0);
	tmp2 = _mm_sub_pi16(tmp, b);
	tmp = _mm_cmpgt_pi16(tmp, b);
	tmp = _mm_srli_pi16(tmp, 31);
	tmp = _mm_mullo_pi16(tmp2, tmp);
	b = _mm_add_pi16(b, tmp);
}

void alpha_mix_MMX(__m64& r, __m64& g, __m64& b, int A) {
	__m64 tmp;

	tmp = _mm_set1_pi16(A);
	r = _mm_mullo_pi16(tmp, r);
	g = _mm_mullo_pi16(tmp, g);
	b = _mm_mullo_pi16(tmp, b);

	r = _mm_srli_pi16(r, 8);
	g = _mm_srli_pi16(g, 8);
	b = _mm_srli_pi16(b, 8);

}

void double_mix_MMX(__m64& r1, __m64& g1, __m64& b1, __m64& r2, __m64& g2, __m64& b2, int A) {
	__m64 tmp;

	tmp = _mm_set1_pi16(A);
	r1 = _mm_mullo_pi16(r1, tmp);
	tmp = _mm_set1_pi16(256 - A);
	r2 = _mm_mullo_pi16(r2, tmp);
	r1 = _mm_add_pi16(r1, r2);
	r1 = _mm_srli_pi16(r1, 8);

	tmp = _mm_set1_pi16(A);
	g1 = _mm_mullo_pi16(g1, tmp);
	tmp = _mm_set1_pi16(256 - A);
	g2 = _mm_mullo_pi16(g2, tmp);
	g1 = _mm_add_pi16(g1, g2);
	g1 = _mm_srli_pi16(g1, 8);

	tmp = _mm_set1_pi16(A);
	b1 = _mm_mullo_pi16(b1, tmp);
	tmp = _mm_set1_pi16(256 - A);
	b2 = _mm_mullo_pi16(b2, tmp);
	b1 = _mm_add_pi16(b1, b2);
	b1 = _mm_srli_pi16(b1, 8);
}

void rgb2yuv_MMX(__m64& r, __m64& g, __m64& b, __m64& y, __m64& u, __m64& v) {
	__m64 tmp, tmp2;

	y = _mm_set1_pi16(66);
	y = _mm_mullo_pi16(y, r);
	tmp = _mm_set1_pi16(129);
	tmp = _mm_mullo_pi16(tmp, g);
	y = _mm_add_pi16(y, tmp);
	tmp = _mm_set1_pi16(25);
	tmp = _mm_mullo_pi16(tmp, b);
	y = _mm_add_pi16(y, tmp);
	tmp = _mm_set1_pi16(128);
	y = _mm_add_pi16(y, tmp);
	y = _mm_srli_pi16(y, 8);
	tmp = _mm_set1_pi16(16);
	y = _mm_add_pi16(y, tmp);

	u = _mm_set1_pi16(-38);
	u = _mm_mullo_pi16(u, r);
	tmp = _mm_set1_pi16(-74);
	tmp = _mm_mullo_pi16(tmp, g);
	u = _mm_add_pi16(u, tmp);
	tmp = _mm_set1_pi16(112);
	tmp = _mm_mullo_pi16(tmp, b);
	u = _mm_add_pi16(u, tmp);
	tmp = _mm_set1_pi16(128);
	u = _mm_add_pi16(u, tmp);
	u = _mm_srli_pi16(y, 8);
	tmp = _mm_set1_pi16(128);
	u = _mm_add_pi16(u, tmp);

	v = _mm_set1_pi16(112);
	v = _mm_mullo_pi16(v, r);
	tmp = _mm_set1_pi16(-94);
	tmp = _mm_mullo_pi16(tmp, g);
	v = _mm_add_pi16(v, tmp);
	tmp = _mm_set1_pi16(-18);
	tmp = _mm_mullo_pi16(tmp, b);
	v = _mm_add_pi16(v, tmp);
	tmp = _mm_set1_pi16(128);
	v = _mm_add_pi16(v, tmp);
	v = _mm_srli_pi16(v, 8);
	tmp = _mm_set1_pi16(128);
	v = _mm_add_pi16(v, tmp);
}

void save_yuv_MMX(YUV& dst, int i, int j, __m64& y, __m64& u, __m64& v) {
	unsigned char y1, y2, y3, y4, u1, v1;
	int index;

	index = (i / 2)*(WIDTH / 2) + (j / 2);

	y4 = *(y.m64_u16);
	y3 = *(y.m64_u16 + 1);
	y2 = *(y.m64_u16 + 2);
	y1 = *(y.m64_u16 + 3);
	u1 = *(u.m64_u16);
	v1 = *(v.m64_u16);

	*(dst.y + i*WIDTH + j) = y1;
	*(dst.y + i*WIDTH + j + 1) = y2;
	*(dst.y + (i + 1)*WIDTH + j) = y3;
	*(dst.y + (i + 1)*WIDTH + j + 1) = y4;

	*(dst.u + index) = u1;
	*(dst.v + index) = v1;
}

void yuv_fade_MMX(YUV& src, YUV &dst, int A) {
	__m64 y, u, v, r, g, b;
	y = _mm_setzero_si64();
	u = _mm_setzero_si64();
	v = _mm_setzero_si64();

	for (int i = 0; i<HEIGHT; i = i + 2) {
		for (int j = 0; j<WIDTH; j = j + 2) {
			get_yuv_MMX(src, i, j, y, u, v);
			yuv2rgb_MMX(y, u, v, r, g, b);
			alpha_mix_MMX(r, g, b, A);
			rgb2yuv_MMX(r, g, b, y, u, v);
			save_yuv_MMX(dst, i, j, y, u, v);
		}
	}
}

void yuv_merge_MMX(YUV& src1, YUV& src2, YUV &dst, int A) {
	__m64 y1, u1, v1, r1, g1, b1;
	__m64 y2, u2, v2, r2, g2, b2;
	y1 = _mm_setzero_si64();
	u1 = _mm_setzero_si64();
	v1 = _mm_setzero_si64();
	y2 = _mm_setzero_si64();
	u2 = _mm_setzero_si64();
	v2 = _mm_setzero_si64();

	for (int i = 0;i<HEIGHT;i += 2) {
		for (int j = 0;j<WIDTH;j += 2) {
			get_yuv_MMX(src1, i, j, y1, u1, v1);
			yuv2rgb_MMX(y1, u1, v1, r1, g1, b1);
			get_yuv_MMX(src2, i, j, y2, u2, v2);
			yuv2rgb_MMX(y2, u2, v2, r2, g2, b2);
			double_mix_MMX(r1, g1, b1, r2, g2, b2, A);
			rgb2yuv_MMX(r1, g1, b1, y1, u1, v1);
			save_yuv_MMX(dst, i, j, y1, u1, v1);
		}
	}
}


void get_yuv_SSE(YUV& src, int i, int j, __m128i& y, __m128i& u, __m128i& v) {
	unsigned char y1, y2, y3, y4, u1, v1;
	int index;

	index = (i / 2)*(WIDTH / 2) + (j / 2);

	y1 = *(src.y + i*WIDTH + j);
	y2 = *(src.y + i*WIDTH + (j + 1));
	y3 = *(src.y + (i + 1)*WIDTH + j);
	y4 = *(src.y + (i + 1)*WIDTH + (j + 1));

	u1 = *(src.u + index);
	v1 = *(src.v + index);

	y = _mm_set_epi32(y1, y2, y3, y4);
	u = _mm_set1_epi32(u1);
	v = _mm_set1_epi32(v1);
}

void yuv2rgb_SSE(__m128i& y, __m128i& u, __m128i& v, __m128i& r, __m128i& g, __m128i& b) {
	__m128i tmp, tmp2;

	r = _mm_set1_epi32(296);
	r = _mm_mullo_epi32(r, y);
	tmp = _mm_set1_epi32(411);
	tmp = _mm_mullo_epi32(tmp, v);
	r = _mm_add_epi32(r, tmp);
	tmp = _mm_set1_epi32(57344);
	r = _mm_sub_epi32(r, tmp);
	r = _mm_srli_epi32(r, 8);

	g = _mm_set1_epi32(299);
	g = _mm_mullo_epi32(g, y);
	tmp = _mm_set1_epi32(101);
	tmp = _mm_mullo_epi32(tmp, u);
	g = _mm_sub_epi32(g, tmp);
	tmp = _mm_set1_epi32(211);
	tmp = _mm_mullo_epi32(tmp, v);
	g = _mm_sub_epi32(g, tmp);
	tmp = _mm_set1_epi32(34739);
	g = _mm_add_epi32(g, tmp);
	g = _mm_srli_epi32(g, 8);

	/*r = clip((296 * y + 411 * v - 57344) >> 8);
	g = clip((299 * y - 101 * u - 211 * v + 34739) >> 8);
	b = clip((299 * y + 519 * u - 71117) >> 8);*/

	b = _mm_set1_epi32(299);
	b = _mm_mullo_epi32(b, y);
	tmp = _mm_set1_epi32(519);
	tmp = _mm_mullo_epi32(tmp, v);
	b = _mm_add_epi32(b, tmp);
	tmp = _mm_set1_epi32(71117);
	b = _mm_sub_epi32(b, tmp);
	b = _mm_srli_epi32(b, 8);

	//ensure under 256
	tmp = _mm_set1_epi32(255);
	tmp2 = _mm_sub_epi32(r, tmp);
	tmp = _mm_cmplt_epi32(tmp, r);
	tmp = _mm_srli_epi32(tmp, 31);
	tmp = _mm_mullo_epi32(tmp2, tmp);
	r = _mm_sub_epi32(r, tmp);

	tmp = _mm_set1_epi32(255);
	tmp2 = _mm_sub_epi32(g, tmp);
	tmp = _mm_cmplt_epi32(tmp, g);
	tmp = _mm_srli_epi32(tmp, 31);
	tmp = _mm_mullo_epi32(tmp2, tmp);
	g = _mm_sub_epi32(g, tmp);

	tmp = _mm_set1_epi32(255);
	tmp2 = _mm_sub_epi32(b, tmp);
	tmp = _mm_cmplt_epi32(tmp, b);
	tmp = _mm_srli_epi32(tmp, 31);
	tmp = _mm_mullo_epi32(tmp2, tmp);
	b = _mm_sub_epi32(b, tmp);

	//ensure above 0
	tmp = _mm_set1_epi32(0);
	tmp2 = _mm_sub_epi32(tmp, r);
	tmp = _mm_cmplt_epi32(r, tmp);
	tmp = _mm_srli_epi32(tmp, 31);
	tmp = _mm_mullo_epi32(tmp2, tmp);
	r = _mm_add_epi32(r, tmp);

	tmp = _mm_set1_epi32(0);
	tmp2 = _mm_sub_epi32(tmp, g);
	tmp = _mm_cmplt_epi32(g, tmp);
	tmp = _mm_srli_epi32(tmp, 31);
	tmp = _mm_mullo_epi32(tmp2, tmp);
	g = _mm_add_epi32(g, tmp);

	tmp = _mm_set1_epi32(0);
	tmp2 = _mm_sub_epi32(tmp, b);
	tmp = _mm_cmplt_epi32(b, tmp);
	tmp = _mm_srli_epi32(tmp, 31);
	tmp = _mm_mullo_epi32(tmp2, tmp);
	b = _mm_add_epi32(b, tmp);
}

void alpha_mix_SSE(__m128i& r, __m128i& g, __m128i& b, int A) {
	__m128i tmp;

	tmp = _mm_set1_epi32(A);
	r = _mm_mullo_epi32(tmp, r);
	g = _mm_mullo_epi32(tmp, g);
	b = _mm_mullo_epi32(tmp, b);

	r = _mm_srli_epi32(r, 8);
	g = _mm_srli_epi32(g, 8);
	b = _mm_srli_epi32(b, 8);
}

void double_mix_SSE(__m128i& r1, __m128i& g1, __m128i& b1, __m128i& r2, __m128i& g2, __m128i& b2, int A) {
	__m128i tmp;

	tmp = _mm_set1_epi32(A);
	r1 = _mm_mullo_epi32(r1, tmp);
	tmp = _mm_set1_epi32(256-A);
	r2 = _mm_mullo_epi32(r2, tmp);
	r1 = _mm_add_epi32(r1, r2);
	r1 = _mm_srli_epi32(r1, 8);

	tmp = _mm_set1_epi32(A);
	g1 = _mm_mullo_epi32(g1, tmp);
	tmp = _mm_set1_epi32(256 - A);
	g2 = _mm_mullo_epi32(g2, tmp);
	g1 = _mm_add_epi32(g1, g2);
	g1 = _mm_srli_epi32(g1, 8);

	tmp = _mm_set1_epi32(A);
	b1 = _mm_mullo_epi32(b1, tmp);
	tmp = _mm_set1_epi32(256 - A);
	b2 = _mm_mullo_epi32(b2, tmp);
	b1 = _mm_add_epi32(b1, b2);
	b1 = _mm_srli_epi32(b1, 8);
}

void rgb2yuv_SSE(__m128i& r, __m128i& g, __m128i& b, __m128i& y, __m128i& u, __m128i& v) {
	__m128i tmp, tmp2;

	y = _mm_set1_epi32(66);
	y = _mm_mullo_epi32(y, r);
	tmp = _mm_set1_epi32(129);
	tmp = _mm_mullo_epi32(tmp, g);
	y = _mm_add_epi32(y, tmp);
	tmp = _mm_set1_epi32(25);
	tmp = _mm_mullo_epi32(tmp, b);
	y = _mm_add_epi32(y, tmp);
	tmp = _mm_set1_epi32(128);
	y = _mm_add_epi32(y, tmp);
	y = _mm_srli_epi32(y, 8);
	tmp = _mm_set1_epi32(16);
	y = _mm_add_epi32(y, tmp);

	u = _mm_set1_epi32(-38);
	u = _mm_mullo_epi32(u, r);
	tmp = _mm_set1_epi32(-74);
	tmp = _mm_mullo_epi32(tmp, g);
	u = _mm_add_epi32(u, tmp);
	tmp = _mm_set1_epi32(112);
	tmp = _mm_mullo_epi32(tmp, b);
	u = _mm_add_epi32(u, tmp);
	tmp = _mm_set1_epi32(128);
	u = _mm_add_epi32(u, tmp);
	u = _mm_srli_epi32(y, 8);
	tmp = _mm_set1_epi32(128);
	u = _mm_add_epi32(u, tmp);

	v = _mm_set1_epi32(112);
	v = _mm_mullo_epi32(v, r);
	tmp = _mm_set1_epi32(-94);
	tmp = _mm_mullo_epi32(tmp, g);
	v = _mm_add_epi32(v, tmp);
	tmp = _mm_set1_epi32(-18);
	tmp = _mm_mullo_epi32(tmp, b);
	v = _mm_add_epi32(v, tmp);
	tmp = _mm_set1_epi32(128);
	v = _mm_add_epi32(v, tmp);
	v = _mm_srli_epi32(v, 8);
	tmp = _mm_set1_epi32(128);
	v = _mm_add_epi32(v, tmp);

}

void save_yuv_SSE(YUV& dst, int i, int j, __m128i& y, __m128i& u, __m128i& v) {
	unsigned char y1, y2, y3, y4, u1, v1;
	int index;

	index = (i / 2)*(WIDTH / 2) + (j / 2);

	y4 = *(y.m128i_i32);
	y3 = *(y.m128i_i32 + 1);
	y2 = *(y.m128i_i32 + 2);
	y1 = *(y.m128i_i32 + 3);
	u1 = *(u.m128i_i32);
	v1 = *(v.m128i_i32);

	*(dst.y + i*WIDTH + j) = y1;
	*(dst.y + i*WIDTH + j + 1) = y2;
	*(dst.y + (i + 1)*WIDTH + j) = y3;
	*(dst.y + (i + 1)*WIDTH + j + 1) = y4;

	*(dst.u + index) = u1;
	*(dst.v + index) = v1;
}

void yuv_fade_SSE(YUV& src, YUV &dst, int A) {
	__m128i y, u, v, r, g, b;
	y = _mm_setzero_si128();
	u = _mm_setzero_si128();
	v = _mm_setzero_si128();

	for (int i = 0; i<HEIGHT; i = i + 2) {
		for (int j = 0; j<WIDTH; j = j + 2) {
			get_yuv_SSE(src, i, j, y, u, v);
			yuv2rgb_SSE(y, u, v, r, g, b);
			alpha_mix_SSE(r, g, b, A);
			rgb2yuv_SSE(r, g, b, y, u, v);
			save_yuv_SSE(dst, i, j, y, u, v);
		}
	}
}

void yuv_merge_SSE(YUV& src1, YUV& src2, YUV& dst, int A) {
	__m128i y1, u1, v1, r1, g1, b1;
	__m128i y2, u2, v2, r2, g2, b2;
	y1 = _mm_setzero_si128();
	u1 = _mm_setzero_si128();
	v1 = _mm_setzero_si128();
	y2 = _mm_setzero_si128();
	u2 = _mm_setzero_si128();
	v2 = _mm_setzero_si128();

	for (int i = 0;i<HEIGHT;i+=2) {
		for (int j = 0;j<WIDTH;j+=2) {
			get_yuv_SSE(src1, i, j, y1, u1, v1);
			yuv2rgb_SSE(y1, u1, v1, r1, g1, b1);
			get_yuv_SSE(src2, i, j, y2, u2, v2);
			yuv2rgb_SSE(y2, u2, v2, r2, g2, b2);
			double_mix_SSE(r1, g1, b1, r2, g2, b2, A);
			rgb2yuv_SSE(r1, g1, b1, y1, u1, v1);
			save_yuv_SSE(dst, i, j, y1, u1, v1);
		}
	}
}


void get_yuv_AVX(YUV& src, int i, int j, __m256i& y, __m256i& u, __m256i& v) {
	unsigned char y1, y2, y3, y4, u1, v1;
	unsigned char y5, y6, y7, y8, u2, v2;
	int index;

	index = (i / 2)*(WIDTH / 2) + (j / 2);

	y1 = *(src.y + i*WIDTH + j);
	y2 = *(src.y + i*WIDTH + (j + 1));
	y3 = *(src.y + (i + 1)*WIDTH + j);
	y4 = *(src.y + (i + 1)*WIDTH + (j + 1));
	u1 = *(src.u + index);
	v1 = *(src.v + index);

	y5 = *(src.y + i*WIDTH + j + 2);
	y6 = *(src.y + i*WIDTH + (j + 3));
	y7 = *(src.y + (i + 1)*WIDTH + j + 2);
	y8 = *(src.y + (i + 1)*WIDTH + (j + 3));
	u2 = *(src.u + index + 1);
	v2 = *(src.v + index + 1);

	y = _mm256_set_epi32(y1, y2, y3, y4, y5, y6, y7, y8);
	u = _mm256_set_epi32(u1, u1, u1, u1, u2, u2, u2, u2);
	v = _mm256_set_epi32(v1, v1, v1, v1, v2, v2, v2, v2);
}

void yuv2rgb_AVX(__m256i& y, __m256i& u, __m256i& v, __m256i& r, __m256i& g, __m256i& b) {
	__m256i tmp, tmp2;

	r = _mm256_set1_epi32(296);
	r = _mm256_mullo_epi32(r, y);
	tmp = _mm256_set1_epi32(411);
	tmp = _mm256_mullo_epi32(tmp, v);
	r = _mm256_add_epi32(r, tmp);
	tmp = _mm256_set1_epi32(57344);
	r = _mm256_sub_epi32(r, tmp);
	r = _mm256_srli_epi32(r, 8);

	g = _mm256_set1_epi32(299);
	g = _mm256_mullo_epi32(g, y);
	tmp = _mm256_set1_epi32(101);
	tmp = _mm256_mullo_epi32(tmp, u);
	g = _mm256_sub_epi32(g, tmp);
	tmp = _mm256_set1_epi32(211);
	tmp = _mm256_mullo_epi32(tmp, v);
	g = _mm256_sub_epi32(g, tmp);
	tmp = _mm256_set1_epi32(34739);
	g = _mm256_add_epi32(g, tmp);
	g = _mm256_srli_epi32(g, 8);

	/*r = clip((296 * y + 411 * v - 57344) >> 8);
	g = clip((299 * y - 101 * u - 211 * v + 34739) >> 8);
	b = clip((299 * y + 519 * u - 71117) >> 8);*/

	b = _mm256_set1_epi32(299);
	b = _mm256_mullo_epi32(b, y);
	tmp = _mm256_set1_epi32(519);
	tmp = _mm256_mullo_epi32(tmp, v);
	b = _mm256_add_epi32(b, tmp);
	tmp = _mm256_set1_epi32(71117);
	b = _mm256_sub_epi32(b, tmp);
	b = _mm256_srli_epi32(b, 8);

	//ensure under 256
	tmp = _mm256_set1_epi32(255);
	tmp2 = _mm256_sub_epi32(r, tmp);
	tmp = _mm256_cmpgt_epi32(r, tmp);
	tmp = _mm256_srli_epi32(tmp, 31);
	tmp = _mm256_mullo_epi32(tmp2, tmp);
	r = _mm256_sub_epi32(r, tmp);

	tmp = _mm256_set1_epi32(255);
	tmp2 = _mm256_sub_epi32(g, tmp);
	tmp = _mm256_cmpgt_epi32(g, tmp);
	tmp = _mm256_srli_epi32(tmp, 31);
	tmp = _mm256_mullo_epi32(tmp2, tmp);
	g = _mm256_sub_epi32(g, tmp);

	tmp = _mm256_set1_epi32(255);
	tmp2 = _mm256_sub_epi32(b, tmp);
	tmp = _mm256_cmpgt_epi32(b, tmp);
	tmp = _mm256_srli_epi32(tmp, 31);
	tmp = _mm256_mullo_epi32(tmp2, tmp);
	b = _mm256_sub_epi32(b, tmp);

	//ensure above 0
	tmp = _mm256_set1_epi32(0);
	tmp2 = _mm256_sub_epi32(tmp, r);
	tmp = _mm256_cmpgt_epi32(tmp, r);
	tmp = _mm256_srli_epi32(tmp, 31);
	tmp = _mm256_mullo_epi32(tmp2, tmp);
	r = _mm256_add_epi32(r, tmp);

	tmp = _mm256_set1_epi32(0);
	tmp2 = _mm256_sub_epi32(tmp, g);
	tmp = _mm256_cmpgt_epi32(tmp, g);
	tmp = _mm256_srli_epi32(tmp, 31);
	tmp = _mm256_mullo_epi32(tmp2, tmp);
	g = _mm256_add_epi32(g, tmp);

	tmp = _mm256_set1_epi32(0);
	tmp2 = _mm256_sub_epi32(tmp, b);
	tmp = _mm256_cmpgt_epi32(tmp, b);
	tmp = _mm256_srli_epi32(tmp, 31);
	tmp = _mm256_mullo_epi32(tmp2, tmp);
	b = _mm256_add_epi32(b, tmp);
}

void alpha_mix_AVX(__m256i& r, __m256i& g, __m256i& b, int A) {
	__m256i tmp;

	tmp = _mm256_set1_epi32(A);
	r = _mm256_mullo_epi32(tmp, r);
	g = _mm256_mullo_epi32(tmp, g);
	b = _mm256_mullo_epi32(tmp, b);

	r = _mm256_srli_epi32(r, 8);
	g = _mm256_srli_epi32(g, 8);
	b = _mm256_srli_epi32(b, 8);
}

void double_mix_AVX(__m256i& r1, __m256i& g1, __m256i& b1, __m256i& r2, __m256i& g2, __m256i& b2, int A) {
	__m256i tmp;

	tmp = _mm256_set1_epi32(A);
	r1 = _mm256_mullo_epi32(r1, tmp);
	tmp = _mm256_set1_epi32(256 - A);
	r2 = _mm256_mullo_epi32(r2, tmp);
	r1 = _mm256_add_epi32(r1, r2);
	r1 = _mm256_srli_epi32(r1, 8);

	tmp = _mm256_set1_epi32(A);
	g1 = _mm256_mullo_epi32(g1, tmp);
	tmp = _mm256_set1_epi32(256 - A);
	g2 = _mm256_mullo_epi32(g2, tmp);
	g1 = _mm256_add_epi32(g1, g2);
	g1 = _mm256_srli_epi32(g1, 8);

	tmp = _mm256_set1_epi32(A);
	b1 = _mm256_mullo_epi32(b1, tmp);
	tmp = _mm256_set1_epi32(256 - A);
	b2 = _mm256_mullo_epi32(b2, tmp);
	b1 = _mm256_add_epi32(b1, b2);
	b1 = _mm256_srli_epi32(b1, 8);
}

void rgb2yuv_AVX(__m256i& r, __m256i& g, __m256i& b, __m256i& y, __m256i& u, __m256i& v) {
	__m256i tmp, tmp2;

	y = _mm256_set1_epi32(66);
	y = _mm256_mullo_epi32(y, r);
	tmp = _mm256_set1_epi32(129);
	tmp = _mm256_mullo_epi32(tmp, g);
	y = _mm256_add_epi32(y, tmp);
	tmp = _mm256_set1_epi32(25);
	tmp = _mm256_mullo_epi32(tmp, b);
	y = _mm256_add_epi32(y, tmp);
	tmp = _mm256_set1_epi32(128);
	y = _mm256_add_epi32(y, tmp);
	y = _mm256_srli_epi32(y, 8);
	tmp = _mm256_set1_epi32(16);
	y = _mm256_add_epi32(y, tmp);

	u = _mm256_set1_epi32(-38);
	u = _mm256_mullo_epi32(u, r);
	tmp = _mm256_set1_epi32(-74);
	tmp = _mm256_mullo_epi32(tmp, g);
	u = _mm256_add_epi32(u, tmp);
	tmp = _mm256_set1_epi32(112);
	tmp = _mm256_mullo_epi32(tmp, b);
	u = _mm256_add_epi32(u, tmp);
	tmp = _mm256_set1_epi32(128);
	u = _mm256_add_epi32(u, tmp);
	u = _mm256_srli_epi32(y, 8);
	tmp = _mm256_set1_epi32(128);
	u = _mm256_add_epi32(u, tmp);

	v = _mm256_set1_epi32(112);
	v = _mm256_mullo_epi32(v, r);
	tmp = _mm256_set1_epi32(-94);
	tmp = _mm256_mullo_epi32(tmp, g);
	v = _mm256_add_epi32(v, tmp);
	tmp = _mm256_set1_epi32(-18);
	tmp = _mm256_mullo_epi32(tmp, b);
	v = _mm256_add_epi32(v, tmp);
	tmp = _mm256_set1_epi32(128);
	v = _mm256_add_epi32(v, tmp);
	v = _mm256_srli_epi32(v, 8);
	tmp = _mm256_set1_epi32(128);
	v = _mm256_add_epi32(v, tmp);

}

void save_yuv_AVX(YUV& dst, int i, int j, __m256i& y, __m256i& u, __m256i& v) {
	unsigned char y1, y2, y3, y4, y5, y6, y7, y8, u1, u2, v1, v2;
	int index;

	index = (i / 2)*(WIDTH / 2) + (j / 2);

	y8 = *(y.m256i_i32);
	y7 = *(y.m256i_i32 + 1);
	y6 = *(y.m256i_i32 + 2);
	y5 = *(y.m256i_i32 + 3);
	y4 = *(y.m256i_i32 + 4);
	y3 = *(y.m256i_i32 + 5);
	y2 = *(y.m256i_i32 + 6);
	y1 = *(y.m256i_i32 + 7);
	u1 = *(u.m256i_i32);
	v1 = *(v.m256i_i32);
	u2 = *(u.m256i_i32 + 4);
	v2 = *(v.m256i_i32 + 4);

	*(dst.y + i*WIDTH + j) = y1;
	*(dst.y + i*WIDTH + j + 1) = y2;
	*(dst.y + (i + 1)*WIDTH + j) = y3;
	*(dst.y + (i + 1)*WIDTH + j + 1) = y4;
	*(dst.y + i*WIDTH + j + 2) = y5;
	*(dst.y + i*WIDTH + j + 3) = y6;
	*(dst.y + (i + 1)*WIDTH + j + 2) = y7;
	*(dst.y + (i + 1)*WIDTH + j + 3) = y8;

	*(dst.u + index) = u1;
	*(dst.v + index) = v1;
	*(dst.u + index + 1) = u2;
	*(dst.v + index + 1) = v2;
}

void yuv_fade_AVX(YUV& src, YUV &dst, int A) {
	__m256i y, u, v, r, g, b;
	y = _mm256_setzero_si256();
	u = _mm256_setzero_si256();
	v = _mm256_setzero_si256();

	for (int i = 0; i<HEIGHT; i = i + 2) {
		for (int j = 0; j<WIDTH; j = j + 2) {
			get_yuv_AVX(src, i, j, y, u, v);
			yuv2rgb_AVX(y, u, v, r, g, b);
			alpha_mix_AVX(r, g, b, A);
			rgb2yuv_AVX(r, g, b, y, u, v);
			save_yuv_AVX(dst, i, j, y, u, v);
		}
	}
}

void yuv_merge_AVX(YUV& src1, YUV& src2, YUV& dst, int A) {
	__m256i y1, u1, v1, r1, g1, b1;
	__m256i y2, u2, v2, r2, g2, b2;
	y1 = _mm256_setzero_si256();
	u1 = _mm256_setzero_si256();
	v1 = _mm256_setzero_si256();
	y2 = _mm256_setzero_si256();
	u2 = _mm256_setzero_si256();
	v2 = _mm256_setzero_si256();

	for (int i = 0;i<HEIGHT;i += 2) {
		for (int j = 0;j<WIDTH;j += 2) {
			get_yuv_AVX(src1, i, j, y1, u1, v1);
			yuv2rgb_AVX(y1, u1, v1, r1, g1, b1);
			get_yuv_AVX(src2, i, j, y2, u2, v2);
			yuv2rgb_AVX(y2, u2, v2, r2, g2, b2);
			double_mix_AVX(r1, g1, b1, r2, g2, b2, A);
			rgb2yuv_AVX(r1, g1, b1, y1, u1, v1);
			save_yuv_AVX(dst, i, j, y1, u1, v1);
		}
	}
}