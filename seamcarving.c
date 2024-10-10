#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "c_img.h"
#include <math.h>

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    //print_grad(im);
    create_img(grad, im->height, im->width);
    
    int h = im->height;
    int w = im->width;
    
    for(int i = 0; i < im->height; i++){
        
        for(int j = 0; j < im->width; j++){
            
            float r = get_pixel(im, i, j, 0);
            float g = get_pixel(im, i, j, 1);
            float b = get_pixel(im, i, j, 2);
            
            int x_left=j-1;
            int x_right = j+1;
            int y_up= i-1;
            int y_down = i+1;
            
            if(i == 0){
                y_up = h-1;
            }
            if(i==(h-1)){
                y_down = 0;
            }
            if(j == 0){
                x_left = w-1;
            }
            if(j == (w-1)){
                x_right = 0;
            }
            
            int rx = (get_pixel(im, i, x_left, 0)) - (get_pixel(im, i, x_right, 0));
            int ry = (get_pixel(im, y_up, j, 0)) - (get_pixel(im, y_down, j, 0));
            int gy = (get_pixel(im, y_up, j, 1)) - (get_pixel(im, y_down, j, 1));
            int gx = (get_pixel(im, i, x_left, 1)) - (get_pixel(im, i, x_right, 1));
            int bx = (get_pixel(im, i, x_left, 2)) - (get_pixel(im, i, x_right, 2));
            int by = (get_pixel(im, y_up, j, 2)) - (get_pixel(im, y_down, j, 2));
            uint8_t eng = (uint8_t)(sqrt((rx*rx)+((ry*ry))+(gx*gx)+(gy*gy)+(bx*bx)+(by*by)) / 10);
            set_pixel((*grad), i, j, eng, eng, eng);
        
        }
    }
    //print_grad(*grad);
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    *best_arr = (double *)malloc(((grad->height)*(grad->width))*sizeof(double));

    for(int i = 0; i < grad->height; i++){ 

        for(int j = 0; j < grad->width; j++){
            //sum is the final value that is going to be at that point in the matrix    
            double sum = get_pixel(grad, i, j, 0);
            if (i==0){
                (*best_arr)[i*(grad->width)+j] = sum;
            } else {
                if(j == 0){
                    sum = sum +fmin((*best_arr)[(i-1)*grad->width+j], (*best_arr)[(i-1)*grad->width+j+1]); 
                }
                else if (j==(grad->width)-1){
                    sum = sum + fmin((*best_arr)[(i-1)*grad->width+j-1], (*best_arr)[(i-1)*grad->width+j]);
                }else{
                    sum = sum + fmin(fmin((*best_arr)[(i-1)*grad->width+j-1], (*best_arr)[(i-1)*grad->width+j]), (*best_arr)[(i-1)*grad->width+j+1]);
                }
                (*best_arr)[i*(grad->width)+j] = sum;
            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    *path = (int*)malloc((height)*sizeof(int));
    
    int index=height-1;
    //getting the minimum value of the bottom row of the cost array. 
    int initial=0; 
    for (int j = 0; j<width; j++){
        if (((best)[(height-1)*width+j])<((best)[(height-1)*width+initial])){
            //best[height][i]<best[height][initial]
            //(*best_arr)[i*width+j] -> (i,j)
            initial = j;
        }
    }
    
    (*path)[index] = initial;
    index--;


    for (int r = height-1; r>0; r--){
        //the seam is at the left edge
        if (initial==0){
            double value = fmin(best[(r-1)*width+initial], best[(r-1)*width+initial+1]);
            if (value==best[(r-1)*width+initial]){
                initial=initial;
            }else {
                initial=initial+1;
            }
        }
        //the seam is at the right edge
        else if (initial==width-1){
            double value = fmin(best[(r-1)*width+initial-1], best[(r-1)*width+initial]);
            if (value==best[(r-1)*width+initial-1]){
                initial=initial-1;
            }else {
                initial=initial;
            }
        //the seam is somewhere in the middle
        }else{
            double value = fmin(fmin(best[(r-1)*width+initial-1],best[(r-1)*width+initial]), best[(r-1)*width+initial+1]);
            if (value==best[(r-1)*width+initial-1]){
                initial=initial-1;
            }else if(value==best[(r-1)*width+initial]){
                initial=initial;
            }else{
                initial=initial+1;
            }
        }
        (*path)[index] = initial;
        index--;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    create_img(dest, src->height, src->width-1);
    // *dest->height = src->height;
    // *dest->width = src->width-1;

    int src_index = 0;
    for(int i = 0; i < src->height; i++){
        src_index=0;
        for(int j = 0; j < src->width-1; j++){
            if (j==path[i]){
                src_index++;
            }
            float r = get_pixel(src, i, src_index, 0);
            float g = get_pixel(src, i, src_index, 1);
            float b = get_pixel(src, i, src_index, 2);

            set_pixel((*dest), i, j, r, g, b);

            src_index++;
        }
    }
}

// int main(){
//     struct rgb_img *im;
//     struct rgb_img *cur_im;
//     struct rgb_img *grad;
//     double *best;
//     int *path;

//     read_in_img(&im, "6x5.bin");

//     calc_energy(im, &grad);
//     printf("_________________----\n");
//     print_grad(grad);

//     // for(int i = 0; i < 5; i++){
//     //     printf("i = %d\n", i);
//     //     calc_energy(im, &grad);
//     //     dynamic_seam(grad, &best);
//     //     recover_path(best, grad->height, grad->width, &path);
//     //     remove_seam(im, &cur_im, path);

//     //     char filename[200];
//     //     sprintf(filename, "img%d.bin", i);
//     //     write_img(cur_im, filename);


//     //     destroy_image(im);
//     //     destroy_image(grad);
//     //     free(best);
//     //     free(path);
//     //     im = cur_im;
//     // }
//     destroy_image(im);
// }


// int main(void)
// {
//     struct rgb_img *im;
//     struct rgb_img *grad;
//     read_in_img(&im, "6x5.bin");
//     calc_energy(im, &grad);
    
//     for(int i = 0; i < grad->height; i++){
//         for(int j = 0; j < grad->width; j++){
//             printf("%d,", get_pixel(grad, i, j, 0));
//         }
//     }

//     destroy_image(im);
//     destroy_image(grad);

//     return 0;
// }