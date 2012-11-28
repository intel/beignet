#include "utest_helper.hpp"

struct seg {
	unsigned int end, color, offset;
	seg(int e, int c):end(e), color(c) {}
};
typedef struct seg seg;

typedef struct {
	std::vector<seg> segs;
} rle_data;

struct rle_image {
	int width, height;
	std::vector<rle_data> data;
	rle_image(int w, int h):width(w), height(h) {}
};
typedef struct rle_image rle_image;

static  void read_data(const char *filename, rle_image &image)
{
	FILE *fp;
	char line[4096];
	int i;
	fp = fopen(filename, "r");
	for (i = 0; i < image.height; i++) {
		char *nptr = line, *endptr;
		rle_data d;
		int start = 0;
		if (fgets(line, sizeof(line), fp) == NULL)
			break;
		for (;;) {
			int len = strtol(nptr, &endptr, 10);
			nptr = endptr;
			int color = strtol(nptr, &endptr, 10);
			nptr = endptr;
			seg s(start + len, color);
			d.segs.push_back(s);
			if (*endptr == '\n' || *endptr == 0)
				break;
			start += len;
		}
		image.data.push_back(d);
	}
	fclose(fp);
}

static void prepare_rle_buffer(rle_image &image, std::vector<int> &rle_buffer, int *offsets)
{
	int offset = 0;
	for (int i = 0; i < image.height; i++) {
		unsigned int j;
		rle_data d = image.data[i];
		for (j = 0; j < d.segs.size(); j++) {
			rle_buffer.push_back(d.segs[j].end);
			rle_buffer.push_back(d.segs[j].color);
		}
		offsets[i] = offset;
		offset += j;
	}

}

static void expand_rle(rle_image &image)
{
	std::vector<int> rle_buffer;
	int offsets[image.height];
	int w = image.width/16;
	prepare_rle_buffer(image, rle_buffer, offsets);
	OCL_CREATE_KERNEL("my_test");
	OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, 2*sizeof(int)*rle_buffer.size(), &rle_buffer[0]);
	OCL_CREATE_BUFFER(buf[1], CL_MEM_COPY_HOST_PTR, sizeof(int)*image.height, offsets);
	OCL_CREATE_BUFFER(buf[2], 0, image.width*image.height, NULL);
	OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
	OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
	OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
	OCL_SET_ARG(3, sizeof(w), &w);

	globals[0] = image.height;
	locals[0] = 16;
	OCL_NDRANGE(1);
#if 1
	OCL_MAP_BUFFER(2);
	for (int i = 0; i < image.height; i++) {
		for (int j = 0; j < image.width; j++)
			printf("%d ", ((unsigned char*)buf_data[2])[i*image.width+j]);
		printf("\n****\n");
	}
	OCL_UNMAP_BUFFER(2);
#endif
}

static void my_test(void)
{
	rle_image image(256, 256);
	read_data("new_data.txt", image);
	expand_rle(image);
}
MAKE_UTEST_FROM_FUNCTION(my_test);
