#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "stb/stb.h"

typedef struct {
    int id;
    int width, height, nrChannels;
    unsigned char* data;
    char* path;
} Image;

int main() {

    int bitmap_width = 128;
    int bitmap_height = 128;
    int padding = 1;
    int num_nodes = bitmap_width - padding;
    int num_images;
    int num_rects;
    int all_rects_packed;

    unsigned char* data;
    unsigned char* bitmap;
    stbrp_context* context;
    stbrp_node* nodes;
    stbrp_rect* rects;
    Image* images;

    context = malloc(sizeof(stbrp_context));
    nodes   = malloc(sizeof(stbrp_node) * num_nodes);
    

    DIR* dir = opendir("images");
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
        images[i].data = stbi_load("assets/textures/objects/object1", 
                                    &images[i].width, 
                                    &images[i].height, 
                                    &images[i].nrChannels, 4);
        ++i;
    }

    for (int i = 0; i < num_rects; i++)
        rects[i] = (stbrp_rect) { .id = i, .w = 16, .h = 16 };

    stbrp_init_target(context, bitmap_width - padding, bitmap_height - padding, nodes, num_nodes);
    stbrp_pack_rects(context, rects, num_rects);

    bitmap = calloc(bitmap_width * bitmap_height, sizeof(unsigned char));

    stbi_write_png("packed.png", bitmap_width, bitmap_height, 1, bitmap, bitmap_width);

    for (int i = 0; i < num_images; ++i)
        stbi_image_free(images[i].data);

    free(bitmap);
    free(context);
    free(nodes);
    free(rects);
    closedir(dir);
}