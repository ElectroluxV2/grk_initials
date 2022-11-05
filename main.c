/*
 * Copyright 2002-2008 Guillaume Cottenceau, 2015 Aleksander Denisiuk
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>


#define OUT_FILE "initials.png"
#define WIDTH 600
#define HEIGHT 600
#define COLOR_TYPE PNG_COLOR_TYPE_RGB
#define BIT_DEPTH 8

void abort_(const char * s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;

void create_png_file()
{
    width = WIDTH;
    height = HEIGHT;
    bit_depth = BIT_DEPTH;
    color_type = COLOR_TYPE;

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y=0; y<height; y++)
        row_pointers[y] = (png_byte*) malloc(width*bit_depth*3);


}


void write_png_file(char* file_name)
{
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp)
        abort_("[write_png_file] File %s could not be opened for writing", file_name);


    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[write_png_file] png_create_write_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[write_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during init_io");

    png_init_io(png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing header");

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing bytes");

    png_write_image(png_ptr, row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during end of write");

    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (y=0; y<height; y++)
        free(row_pointers[y]);
    free(row_pointers);

    fclose(fp);
}

void write_pixel(int px, int py, png_byte cr, png_byte cg, png_byte cb)
{
    png_byte* row = row_pointers[py];
    png_byte* ptr = &(row[3*px]);
    ptr[0] = cr;
    ptr[1] = cg;
    ptr[2] = cb;
}

void line(int i1, int j1, int i2, int j2, png_byte cr, png_byte cg, png_byte cb)
{

    if (i2 < i1) {
        int i = i1; i1 = i2; i2 = i;
        int j = j1; j1 = j2; j2 = j;
    }


    if (i2 > i1 && j2 >= j1 && j2 - j1 <= i2 - i1) { // Type 1
        printf("Type 1\n");
        int m = 2 * (j2 - j1);
        int b = 0;
        write_pixel(i1, j1, cr, cg, cb);

        int j = j1;
        int P = i2 - i1;
        for (int i = i1 + 1; i <= i2; i++)
        {
            b = b + m;
            if (b > P)
            {
                j = j + 1;
                b = b -(2*P);
            }

            write_pixel(i, j, cr, cg, cb);
        }
    } else if (j2 > j1 && i2 >= i1 && i2 - i1 <= j2 - j1) { // Type 2
        printf("Type 2\n");
        int m = 2 * (i2 - i1);
        int b = 0;
        write_pixel(i1, j1, cr, cg, cb);

        int i = i1;
        int P = j2 - j1;
        for (int j = j1 + 1; j <= j2; j++)
        {
            b = b + m;
            if (b > P)
            {
                i = i + 1;
                b = b -(2*P);
            }

            write_pixel(i, j, cr, cg, cb);
        }
    } else if (i2 > i1 && j2 <= j1 && -j2 + j1 <= i2 - i1) { // Type 3
        printf("Type 3\n");
        int m = 2 * (-j2 + j1);
        int b = 0;
        write_pixel(i1, j1, cr, cg, cb);

        int j = j1;
        int P = i2 - i1;
        for (int i = i1 + 1; i <= i2; i++)
        {
            b = b + m;
            if (b > P)
            {
                j = j - 1;
                b = b -(2*P);
            }

            write_pixel(i, j, cr, cg, cb);
        }
    } else if (j2 < j1 && i2 >= i1 && i2 - i1 <= -j2 + j1) { // Type 4
        printf("Type 4\n");
        int m = 2 * (i2 - i1);
        int b = 0;
        write_pixel(i1, j1, cr, cg, cb);

        int i = i1;
        int P = -j2 + j1;
        for (int j = j1 - 1; j >= j2; j--)
        {
            b = b + m;
            if (b > P)
            {
                i = i + 1;
                b = b -(2*P);
            }

            write_pixel(i, j, cr, cg, cb);
        }
    }


}

void write_pixel_8(int px, int py, int x0, int y0, png_byte cr, png_byte cg, png_byte cb) {
    write_pixel(px + x0, py + y0, cr, cg, cb);
    write_pixel(py + x0, px + y0, cr, cg, cb);
    write_pixel(px + x0, -py + y0, cr, cg, cb);
    write_pixel(py + x0, -px + y0, cr, cg, cb);
    write_pixel(-px + x0, py + y0, cr, cg, cb);
    write_pixel(-py + x0, px + y0, cr, cg, cb);
    write_pixel(-px + x0, -py + y0, cr, cg, cb);
    write_pixel(-py + x0, -px + y0, cr, cg, cb);
}

void circle(int x0, int y0, int R, png_byte cr, png_byte cg, png_byte cb) {

    int i, j, f;
    i = 0;
    j = R;
    f = 5-4*R;
    write_pixel_8(i, j, x0, y0, cr, cg, cb);
    while (i <=j) {
        if (f > 0) {
            f = f + 8*i-8*j+20;
            j -= 1;
        } else {
            f = f + 8 *i + 12;
        }
        i++;
        write_pixel_8(i, j, x0, y0, cr, cg, cb);
    }
}

void process_file(void)
{
    for (y=0; y<height; y++) {
        png_byte* row = row_pointers[y];
        for (x=0; x<width; x++) {
            png_byte* ptr = &(row[x*3]);
            ptr[0] = 0;
            ptr[1] = ptr[2] = 255;
        }
    }

    line(111, 418, 151, 419, 255, 0, 0);
    line(151, 419, 151, 419, 255, 0, 0);
    line(151, 419, 190, 265, 255, 0, 0);
    line(190, 265, 190, 266, 255, 0, 0);
    line(190, 266, 205, 374, 255, 0, 0);
    line(205, 374, 205, 374, 255, 0, 0);
    line(205, 374, 228, 268, 255, 0, 0);
    line(228, 268, 228, 268, 255, 0, 0);
    line(228, 268, 251, 417, 255, 0, 0);
    line(251, 417, 251, 417, 255, 0, 0);
    line(251, 417, 287, 413, 255, 0, 0);
    line(287, 413, 287, 413, 255, 0, 0);
    line(287, 413, 236, 204, 255, 0, 0);
    line(236, 204, 236, 204, 255, 0, 0);
    line(236, 204, 206, 301, 255, 0, 0);
    line(206, 301, 206, 301, 255, 0, 0);
    line(206, 301, 190, 204, 255, 0, 0);
    line(190, 204, 190, 203, 255, 0, 0);
    line(190, 203, 112, 418, 255, 0, 0);
    line(112, 418, 112, 417, 255, 0, 0);


    line(316, 201, 315, 414, 255, 0, 0);
    line(315, 414, 315, 414, 255, 0, 0);
    line(315, 414, 387, 412, 255, 0, 0);
    line(387, 412, 387, 412, 255, 0, 0);
    line(387, 412, 430, 388, 255, 0, 0);
    line(430, 388, 430, 388, 255, 0, 0);
    line(430, 388, 446, 356, 255, 0, 0);
    line(446, 356, 446, 356, 255, 0, 0);
    line(446, 356, 436, 323, 255, 0, 0);
    line(436, 323, 436, 323, 255, 0, 0);
    line(436, 323, 408, 313, 255, 0, 0);
    line(408, 313, 408, 313, 255, 0, 0);
    line(408, 313, 385, 300, 255, 0, 0);
    line(385, 300, 384, 300, 255, 0, 0);
    line(384, 300, 408, 287, 255, 0, 0);
    line(408, 287, 408, 287, 255, 0, 0);
    line(408, 287, 433, 282, 255, 0, 0);
    line(433, 282, 433, 279, 255, 0, 0);
    line(433, 279, 451, 250, 255, 0, 0);
    line(451, 250, 450, 249, 255, 0, 0);
    line(450, 249, 444, 217, 255, 0, 0);
    line(444, 217, 444, 217, 255, 0, 0);
    line(444, 217, 422, 198, 255, 0, 0);
    line(422, 198, 422, 198, 255, 0, 0);
    line(422, 198, 316, 201, 255, 0, 0);


    line(353, 230, 353, 230, 255, 0, 0);
    line(353, 230, 353, 272, 255, 0, 0);
    line(353, 272, 353, 272, 255, 0, 0);
    line(353, 272, 383, 265, 255, 0, 0);
    line(383, 265, 383, 265, 255, 0, 0);
    line(383, 265, 411, 249, 255, 0, 0);
    line(411, 249, 411, 249, 255, 0, 0);
    line(411, 249, 401, 224, 255, 0, 0);
    line(401, 224, 401, 224, 255, 0, 0);
    line(401, 224, 353, 230, 255, 0, 0);


    line(348, 331, 346, 392, 255, 0, 0);
    line(346, 392, 346, 392, 255, 0, 0);
    line(346, 392, 382, 390, 255, 0, 0);
    line(382, 390, 382, 390, 255, 0, 0);
    line(382, 390, 408, 376, 255, 0, 0);
    line(408, 376, 408, 376, 255, 0, 0);
    line(408, 376, 416, 349, 255, 0, 0);
    line(416, 349, 416, 349, 255, 0, 0);
    line(416, 349, 411, 336, 255, 0, 0);
    line(411, 336, 411, 336, 255, 0, 0);
    line(411, 336, 348, 331, 255, 0, 0);

    circle(270, 300, 210, 255, 255, 0);
}


int main(int argc, char **argv)
{
    create_png_file();
    process_file();
    write_png_file(OUT_FILE);

    return 0;
}