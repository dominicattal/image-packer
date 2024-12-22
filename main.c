#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "stb/stb.h"

typedef struct {
    int id;
    int width, height, num_channels;
    unsigned char* data;
    char* path;
} Image;

int main() {

    int num_channels = 4;
    int bitmap_width = 32;
    int bitmap_height = 32;
    int padding = 1;
    int num_nodes = bitmap_width;
    int num_images;
    int num_rects;
    int all_rects_packed;

    unsigned char* bitmap;
    stbrp_context* context;
    stbrp_node* nodes;
    stbrp_rect* rects;
    Image* images;

    /* --------- setup rect information --------- */

    const char* image_dir = "images";
    DIR* dir = opendir(image_dir);
    struct dirent* entry;
    
    num_images = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        ++num_images;
    }
    rewinddir(dir);

    num_rects = num_images;

    rects   = malloc(sizeof(stbrp_rect) * num_rects);
    images  = malloc(sizeof(Image) * num_images);

    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        images[i].id = i;
        images[i].path = malloc((strlen(image_dir) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
        strcpy(images[i].path, image_dir);
        strcat(images[i].path, "/");
        strcat(images[i].path, entry->d_name);
        images[i].data = stbi_load( images[i].path, 
                                   &images[i].width, 
                                   &images[i].height, 
                                   &images[i].num_channels, num_channels);
        rects[i].id = images[i].id;
        rects[i].w  = padding + images[i].width;
        rects[i].h  = padding + images[i].height;
        ++i;
    }
    closedir(dir);

    /* --- setup packing context and pack rects --- */

    context = malloc(sizeof(stbrp_context));
    nodes   = malloc(sizeof(stbrp_node) * num_nodes);

    stbrp_init_target(context, bitmap_width, bitmap_height, nodes, num_nodes);
    all_rects_packed = stbrp_pack_rects(context, rects, num_rects);
    puts((all_rects_packed) ? "All rects packed" : "Not all rects packed");

    /* ------- convert packed rects into bitmap -------- */

    bitmap = calloc(bitmap_height * bitmap_width * num_channels, sizeof(unsigned char));

    for (int i = 0; i < num_rects; i++) {
        if (!rects[i].was_packed) {
            printf("Failed to pack image %d: %s\n", images[i].id, images[i].path);
            continue;
        }
        // height, width, channels, index in data, index in bitmap
        int y, x, c, data_idx, bitmap_idx;
        for (y = 0; y < images[i].height; ++y) {
            for (x = 0; x < images[i].width; ++x) {
                for (c = 0; c < num_channels; ++c) {

                    data_idx =   y * num_channels * images[i].width  
                               + x * num_channels 
                               + c;

                    bitmap_idx =   (y + rects[i].y) * num_channels * bitmap_width
                                 + (x + rects[i].x) * num_channels
                                 +  c;

                    bitmap[bitmap_idx] = images[i].data[data_idx];
                }
            }
        }
    }

    stbi_write_png("packed.png", bitmap_width, bitmap_height, num_channels, bitmap, 0);

    /* ----------- cleanup ------------- */

    for (int i = 0; i < num_images; ++i) {
        free(images[i].path);
        stbi_image_free(images[i].data);
    }

    free(images);
    free(bitmap);
    free(context);
    free(nodes);
    free(rects);
}