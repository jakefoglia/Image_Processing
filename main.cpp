
/*
    https://imagemagick.org/script/magick++.php#install
*/
  
#include "Magick++.h" // version 7
#include <stdio.h>
#include <iostream> 
#include <stdlib.h>    
#include <time.h>
#include <chrono>
#include <random>
#include <math.h>
#include <string.h>

using namespace std; 
using namespace Magick; 

#define scale (255 / QuantumRange)

//int64_t distr_sum; // for testing
//int64_t distr_count;

typedef struct float3
{
    float3() {};
    float3(float x, float y, float z) : x(x), y(y), z(z) {}

    float x, y, z;
    inline float3 operator*(float s) const { return float3(x*s, y*s, z*s); }
    inline float3 operator/(float s) const { return float3(x/s, y/s, z/s); }
    inline float3 operator+(const float3& a) const { return float3(x+a.x, y+a.y, z+a.z); }

} float3;


void prompt(char const* prompt_msg, int& val)
{
    printf(prompt_msg);
    printf("\n");
    std::cin >> val;
}
void prompt(char const* prompt_msg, uint& val)
{
    printf(prompt_msg);
    printf("\n");
    std::cin >> val;
}
void prompt(char const* prompt_msg, float& val)
{
    printf(prompt_msg);
    printf("\n");
    std::cin >> val;
}
void prompt(char const* prompt_msg, string& val)
{
    printf(prompt_msg);
    printf("\n");
    std::cin >> val;
}

uint8_t coin_flip()
{
    uint8_t val = rand()%2;
    //printf("CF was %i\n", val);
    return val ; 
}

void clip(int& r, int& g, int& b, int max)
{
    if(r < 0)
        r = 0;
    else if (r > max)
        r = max;

    if(g < 0)
        g = 0;
    else if (g > max)
        g = max;  
    
    if(b < 0)
        b = 0;
    else if (b > max)
        b = max;
}

void get_rgb(Quantum *pixels, int w, int h, int row, int col, int& r,  int& g,  int& b)
{
    Quantum* rq = &pixels[3*(w*row + col)    ];
    Quantum* gq = &pixels[3*(w*row + col) + 1];
    Quantum* bq = &pixels[3*(w*row + col) + 2];

    r = *rq * scale;
    g = *gq * scale;
    b = *bq * scale;

    if(r < 0 || r > 255)
        printf("get_rgb not working r = %i\n", r);
    if(g < 0 || g > 255)
        printf("get_rgb not working r = %i\n", g);
    if(b < 0 || b > 255)
        printf("get_rgb not working r = %i\n", b);
}
void set_rgb(Quantum *pixels, int w, int h, int row, int col, int r,  int g,  int b)
{
    clip(r, g, b, 255);

    Quantum* rq = &pixels[3*(w*row + col)    ];
    Quantum* gq = &pixels[3*(w*row + col) + 1];
    Quantum* bq = &pixels[3*(w*row + col) + 2];

    *rq = r/scale;
    *gq = g/scale;
    *bq = b/scale;
}

float3 variance(Quantum *pixels, int w, int h)
{
    // the variance of the signal is the : mean of its (squares minus the square of its mean)

    float r_mean, g_mean, b_mean;
    r_mean = g_mean = b_mean = 0.0f;

    Quantum* p = pixels;
    for(int row = 0; row < h; row++)
    {
        for(int col = 0; col < w; col++)
        {
            int r = *p++ * scale;
            int g = *p++ * scale;
            int b = *p++ * scale;

            r_mean += r;
            g_mean += g;
            b_mean += b;
        }
    }
    r_mean /= w*h;
    g_mean /= w*h;
    b_mean /= w*h;


    float r_var, g_var, b_var;
    r_var = g_var = b_var = 0.0f;

    for(int row = 0; row < h; row++)
    {
        for(int col = 0; col < w; col++)
        {
            int r = *pixels++ * scale;
            int g = *pixels++ * scale;
            int b = *pixels++ * scale;

            r_var += (r*r - r_mean);
            g_var += (g*g - g_mean);
            b_var += (b*b - b_mean);
        }
    }
    r_var /= w*h;
    g_var /= w*h;
    b_var /= w*h;

    return float3(r_var, g_var, b_var);

}

float3 power(Quantum *pixels, int w, int h)
{
    //The power of a signal is the sum of the absolute squares of its time-domain samples divided by the signal length, or, equivalently, the square of its RMS level.

    float r_sq_sum, g_sq_sum, b_sq_sum;
    r_sq_sum = g_sq_sum = b_sq_sum = 0.0f;


    Quantum* p = pixels;
    for(int row = 0; row < h; row++)
    {
        for(int col = 0; col < w; col++)
        {
            int r = *p++ * scale;
            int g = *p++ * scale;
            int b = *p++ * scale;

            r_sq_sum += r*r;
            g_sq_sum += g*g;
            b_sq_sum += b*b;
        }
    }
    
    return float3(r_sq_sum/(w*h), g_sq_sum/(w*h), b_sq_sum/(w*h));
}


int16_t generate_AWGN(float SNR, float std_dev) // -255 to 255
{
    /*
    Signal-to-Noise Ratio (SNR)
    http://web.mit.edu/6.02/www/currentsemester/handouts/L08_slides.pdf  slide 7
    */

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    seed = seed * seed;
    default_random_engine generator (seed);
    normal_distribution<float> distribution(0, std_dev);

    int16_t result;
    if(!isinf(std_dev))
    {
        result = distribution(generator);
        if(result > 255)
            result = 255;
        else if(result < -255)
            result = -255;
        
        //distr_sum += result;
        //distr_count++;
    }
    else
    {
        srand(seed);
        result = 255 - (255*2* coin_flip()); // force 255 or -255 if std_dev was inf
    }
    //printf("res: %6.4f\t", result);
    return  result;
}

void AWGN(Image& img, float SNRdB, char const* output_file) //https://pysdr.org/content/noise.html#snr
{
    //distr_count = 0;
    //distr_sum = 0;

    int w = img.columns();
    int h = img.rows();

    img.type(TrueColorType);
    img.modifyImage();
    Pixels view(img);
    Quantum *pixels = view.get(0,0,w,h);

    
    float SNR = powf(10.0f, SNRdB / 10.0f);
    float3 signal_power = power(pixels, w, h);
/*    
    printf("\tSNRdB %8.6f   SNR %8.6f\n\n\tr_pow %8.6f\n\tg_pow %8.6f\n\tb_pow %8.6f\n\n",
        SNRdB, SNR, signal_power.x, signal_power.y, signal_power.z);
*/
    //SNR = Power(signal) / variance     variance = std_dev^2
    float r_std_dev = sqrt( signal_power.x / SNR ); 
    float g_std_dev = sqrt( signal_power.y / SNR ); 
    float b_std_dev = sqrt( signal_power.z / SNR ); 

/*
    printf("\tr_std_dev %8.6f\n\tg_std_dev %8.6f\n\tb_std_dev %8.6f\n\n",
        r_std_dev, g_std_dev, b_std_dev);
*/
    for(int row = 0; row < h; row++)
        for(int col = 0; col < w; col++)
        {
            Quantum* rq = &pixels[3*(w*row + col)    ];
            Quantum* gq = &pixels[3*(w*row + col) + 1];
            Quantum* bq = &pixels[3*(w*row + col) + 2];

            //printf("Signal:\t %8.6f %8.6f %8.6f \n", *r * scale, *g * scale, *b * scale);
            
            int r_noise = generate_AWGN(SNRdB, r_std_dev); //* 255 / scale;
            int g_noise = generate_AWGN(SNRdB, g_std_dev); //* 255 / scale;
            int b_noise = generate_AWGN(SNRdB, b_std_dev); //* 255 / scale;


           int r = (*rq * scale + r_noise); 
           int g = (*gq * scale + g_noise);
           int b = (*bq * scale + b_noise);

            set_rgb(pixels, w, h, row, col, r, g, b);
    
            //printf("NOISE:\t %8.6f %8.6f %8.6f  \n", r_noise * scale, g_noise * scale, b_noise * scale);
            //printf("Result:\t %8.6f %8.6f %8.6f  \n\n", *r * scale, *g * scale, *b * scale);
    }
    img.syncPixels();
    view.sync();

    printf("Writing image to disk...\n");
    img.write(output_file);  

}

void SPN(Image& img, float countToPixelsRatio, char const* output_file) //https://www.programmersought.com/article/24315567946/
{

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    seed = seed * seed;
    srand(seed);

    int w = img.columns();
    int h = img.rows();

    img.type(TrueColorType);
    img.modifyImage();
    Pixels view(img);
    Quantum *pixels = view.get(0,0,w,h);

    
    //printf("stuff");

    if(countToPixelsRatio < 1.f)
    {
        uint32_t count = countToPixelsRatio * w * h;
        bool hit[w*h] = {false};
        for(int i = 0; i < count; i++)
        {
            int x, y;
            do // all must be unique
            {
                x = rand() % w;
                y = rand() % h;
            } while(hit[w*y + x]) ; 

            hit[w*y + x] = true;


            int val = 255*coin_flip();
            //printf( ((val == 255) ? "white\n" : "black\n" ));
            set_rgb(pixels, w, h, y, x, val, val, val);
        }
    }
    else // just iterate through all
    {
        for(int row = 0; row < h; row++)
        {
            for(int col = 0; col < w; col++)
            {
                int val = 255*coin_flip();
                //printf( ((val == 255) ? "white\n" : "black\n" ));
                set_rgb(pixels, w, h, row, col, val, val, val);
            }
        }

    }
    
    img.syncPixels();
    view.sync();

    printf("Writing image to disk...\n");
    img.write(output_file);  
}



double gaussian(float x, float mu, float sigma ) {
    float a = (x - mu) / sigma;
    return exp(-0.5 * a * a);
}

//https://stackoverflow.com/questions/42186498/gaussian-blur-image-processing-c //https://stackoverflow.com/questions/8204645/implementing-gaussian-blur-how-to-calculate-convolution-matrix-kernel
 void GBNR(Image& img, int mask_radius, char const* output_file) 
{
    int min_radius = 1;
    int max_radius = 10;

    if(mask_radius < min_radius)
    {
        printf("mask_radius must be between %i and %i\n", min_radius, max_radius);
        printf("using mask_radius = %i\n", min_radius);
        mask_radius = min_radius;
    }
    else if(mask_radius > max_radius)
    {
        printf("mask_radius must be between %i and %i\n", min_radius, max_radius);
        printf("using mask_radius = %i\n", max_radius);
        mask_radius = max_radius;
    }
    int mask_dim = 1+2*mask_radius;

    // generate mask (not normalized)
    float mask[mask_dim][mask_dim];
    float norm = 0.f;
    double sigma = mask_radius/2.f;
    
    for (int row = 0; row < mask_dim; row++) 
        for (int col = 0; col < mask_dim; col++) {
            float x = gaussian(row, mask_radius, sigma) * gaussian(col, mask_radius, sigma);
            mask[row][col] = x;
            norm += x;
    }

/*
    for (int row = 0; row < mask_dim; row++) 
        for (int col = 0; col < mask_dim; col++) 
            printf("mask(%i, %i) : %6.4f\n", row, col, mask[row][col]/norm);
*/
    
    /* dont normalize the mask
    for (int col = 0; col < mask_dim; col++)
        for (int row = 0; row < mask_dim; row++) // normalize
         {
            mask[row][col] /= norm;
    }
    */


    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    seed = seed * seed;
    srand(seed);

    int w = img.columns();
    int h = img.rows();

    img.type(TrueColorType);
    img.modifyImage();
    Pixels view(img);
    Quantum *pixels = view.get(0,0,w,h);
    int bytes = 3 * w * h * sizeof(Quantum);
    Quantum *result = (Quantum*) malloc(bytes);


    for(int row = 0; row < h; row++)
    {
        for(int col = 0; col < w; col++)
        {
            float true_norm;
            if(row - mask_radius < 0 || row + mask_radius >= h || col - mask_radius < 0 || col + mask_radius >= w) // boundary condition needs custom normalization
            {

                true_norm = 0.f;
                for (int j = -1*mask_radius; j <= mask_radius; j++) 
                    for (int k = -1*mask_radius; k <= mask_radius; k++) {
                        int y = row + j; 
                        int x = col + k;
                        if(y >= 0 && y < h && x >= 0 && x < w) {
                            true_norm += mask[mask_radius+j][mask_radius+k]; // mask_radius is the center of that dimension (if radius is 1, size is 3, index 1 is the middle of mask) 
                        }
                }
            }
            else {
                true_norm = norm;
            }
            
            int r = 0;
            int g = 0;
            int b = 0;
            
            // convolve the mask
            for (int j = -1*mask_radius; j <= mask_radius; j++) 
                for (int k = -1*mask_radius; k <= mask_radius; k++) {
                    int y = row + j; 
                    int x = col + k;


                    if(y >= 0 && y < h && x >= 0 && x < w) {
                        int xy_r, xy_g, xy_b;
                        get_rgb(pixels, w, h, y, x, xy_r, xy_g, xy_b);

                        r += xy_r * mask[mask_radius+j][mask_radius+k]; 
                        g += xy_g * mask[mask_radius+j][mask_radius+k]; 
                        b += xy_b * mask[mask_radius+j][mask_radius+k]; 

                    }


                    

            }
            r /= true_norm;
            g /= true_norm;
            b /= true_norm;

            set_rgb(result, w, h, row, col, r, g, b);
        }
    }
    memcpy((void*) pixels, (const void*) result, bytes); 
    free(result);

    img.syncPixels();
    view.sync();

    printf("Writing image to disk...\n");
    img.write(output_file);  
}



void AWGN_driver()
{
    printf("\nAdditive White Gaussian Noise Tool\n");
    InitializeMagick(nullptr);
    Image image;
    
    string in_str;
    prompt("\nEnter the name of the image to process", in_str);
    in_str = string("images/") + in_str;

    char const* in_c = const_cast<char*>(in_str.c_str());

    printf("Reading image from disk...\n");
    try { 
        image.read(in_c);
    }
    catch( Exception &error_ ) 
    { 
        printf("Could not read image ");
        printf(in_c); 
        printf("\nCaught exception:\n\t");
        printf(error_.what());
        printf("\n"); 
        return;
    }

    string out_str;
    prompt("\nEnter the desired name of the output image", out_str);
    out_str = string("images/") + out_str;

    char const* out_c = const_cast<char*>(out_str.c_str());


    float SNRdB;
    prompt("\nEnter the Signal to Noise Ratio in dB (recomended range: -30 dB to 40 dB)", SNRdB);

    printf("\nGenerating AWGN...\n");
    AWGN(image, SNRdB, out_c);
     
    
}

void SPN_driver()
{
    printf("\nSalt and Pepper Noise Tool\n");
    InitializeMagick(nullptr);
    Image image;
    
    string in_str;
    prompt("\nEnter the name of the image to process", in_str);
    in_str = string("images/") + in_str;

    char const* in_c = const_cast<char*>(in_str.c_str());

    printf("Reading image from disk...\n");
    try { 
        image.read(in_c);
    }
    catch( Exception &error_ ) 
    { 
        printf("Could not read image ");
        printf(in_c); 
        printf("\nCaught exception:\n\t");
        printf(error_.what());
        printf("\n"); 
        return;
    }

    string out_str;
    prompt("\nEnter the desired name of the output image", out_str);
    out_str = string("images/") + out_str;

    char const* out_c = const_cast<char*>(out_str.c_str());

    float ctpr;
    prompt("\nEnter the Count to Pixel Ratio (0.0 to 1.0)", ctpr);

    printf("\nGenerating MSN...\n");
    SPN(image, ctpr, out_c);
}


void GBNR_driver()
{
    printf("\nGaussian Blur Noise Removal Tool\n");
    InitializeMagick(nullptr);
    Image image;
    
    string in_str;
    prompt("\nEnter the name of the image to process", in_str);
    in_str = string("images/") + in_str;

    char const* in_c = const_cast<char*>(in_str.c_str());

    printf("Reading image from disk...\n");
    try { 
        image.read(in_c);
    }
    catch( Exception &error_ ) 
    { 
        printf("Could not read image ");
        printf(in_c); 
        printf("\nCaught exception:\n\t");
        printf(error_.what());
        printf("\n"); 
        return;
    }

    string out_str;
    prompt("\nEnter the desired name of the output image", out_str);
    out_str = string("images/") + out_str;
    char const* out_c = const_cast<char*>(out_str.c_str());


    int mask_radius;
    prompt("\nEnter the mask radius", mask_radius);

    printf("\nRemoving Noise...\n");
    GBNR(image, mask_radius, out_c);
}

void test_driver()
{
    printf("Reading image from disk...\n");
    Image img;
    img.read("images/color_gradient.jpg");
    int w = img.columns();
    int h = img.rows();

    img.type(TrueColorType);
    img.modifyImage();
    Pixels view(img);
    Quantum *pixels = view.get(0,0,w,h);

    for(int row = 0; row < h; row++)
        for(int col = 0; col < w; col++)
        {        
            set_rgb(pixels, w, h, row, col, (row+col) % 255, (row+col) % 255, (row+col) % 255);
        }  

    //void set_rgb(Quantum *pixels, int w, int h, int row, int col, int r,  int g,  int b)
    img.syncPixels();
    view.sync();

    printf("Writing image to disk...\n");
    img.write("images/test.jpg");  
}

int main(int argc, char **argv) 
{ 
    
    bool run = true;
    while(run)
    {
        int tool = 0;
        char const* str = "\nEnter an integer to select a tool: \n\t0 : Exit \n\t1 : Additive White Gaussian Noise \n\t2 : Salt & Pepper Noise\n\t3 : Gaussian Blur Noise Removal\n";  // feel free to add more options for your tools
        prompt(str, tool);
        //printf("input %i\n", tool);
        switch(tool)
        {
            case 0 :
                run = false;
                break;

            case 1 :
                run = true;
                AWGN_driver();
                break;

            case 2 :
                run = true;
                SPN_driver();
                break;
            
            case 3 :
                run = true;
                GBNR_driver();
                break;

            default :
                run = true;
                printf("invalid input\n");
                break;

        }
    }
    
   //test_driver();
   return 0;
}


/*
Jake:

Additive White Gaussian Noise       CHECK
Multiplicative/Speckle Noise        CHECK
noise removal (Gaussian Filtering)  CHECK
*/ 
