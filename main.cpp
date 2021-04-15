
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

void set_rgb(Quantum *pixels, int w, int h, int x, int y, int r,  int g,  int b)
{
    clip(r, g, b, 255);

    Quantum* rq = &pixels[3*(w*y + x)    ];
    Quantum* gq = &pixels[3*(w*y + x) + 1];
    Quantum* bq = &pixels[3*(w*y + x) + 2];

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
    for(int col = 0; col < w; col++)
    {
        for(int row = 0; row < h; row++)
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

    for(int col = 0; col < w; col++)
    {
        for(int row = 0; row < h; row++)
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
    for(int col = 0; col < w; col++)
    {
        for(int row = 0; row < h; row++)
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


int16_t AWGN(float SNR, float std_dev) // -255 to 255
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

void generate_AWGN(Image& img, float SNRdB) //https://pysdr.org/content/noise.html#snr
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
    
    printf("\tSNRdB %8.6f   SNR %8.6f\n\n\tr_pow %8.6f\n\tg_pow %8.6f\n\tb_pow %8.6f\n\n",
        SNRdB, SNR, signal_power.x, signal_power.y, signal_power.z);

    //SNR = Power(signal) / variance     variance = std_dev^2
    float r_std_dev = sqrt( signal_power.x / SNR ); 
    float g_std_dev = sqrt( signal_power.y / SNR ); 
    float b_std_dev = sqrt( signal_power.z / SNR ); 

    printf("\tr_std_dev %8.6f\n\tg_std_dev %8.6f\n\tb_std_dev %8.6f\n\n",
        r_std_dev, g_std_dev, b_std_dev);

    for(int col = 0; col < w; col++)
    {
        for(int row = 0; row < h; row++)
        {
            Quantum* rq = &pixels[3*(w*row + col)    ];
            Quantum* gq = &pixels[3*(w*row + col) + 1];
            Quantum* bq = &pixels[3*(w*row + col) + 2];

            //printf("Signal:\t %8.6f %8.6f %8.6f \n", *r * scale, *g * scale, *b * scale);
            
            int r_noise = AWGN(SNRdB, r_std_dev); //* 255 / scale;
            int g_noise = AWGN(SNRdB, g_std_dev); //* 255 / scale;
            int b_noise = AWGN(SNRdB, b_std_dev); //* 255 / scale;


           int r = (*rq * scale + r_noise); 
           int g = (*gq * scale + g_noise);
           int b = (*bq * scale + b_noise);

            set_rgb(pixels, w, h, col, row, r, g, b);
            
            
            //printf("NOISE:\t %8.6f %8.6f %8.6f  \n", r_noise * scale, g_noise * scale, b_noise * scale);
            //printf("Result:\t %8.6f %8.6f %8.6f  \n\n", *r * scale, *g * scale, *b * scale);
        }
    }
    img.syncPixels();
    view.sync();

    printf("Writing image to disk...\n");
    img.write( "images/out_AWGN.jpg" );  

}

void generate_SPN(Image& img, float countToPixelsRatio) //https://www.programmersought.com/article/24315567946/
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
            set_rgb(pixels, w, h, x, y, val, val, val);
        }
    }
    else // just iterate through all
    {
        for(int col = 0; col < w; col++)
        {
            for(int row = 0; row < h; row++)
            {
                int val = 255*coin_flip();
                //printf( ((val == 255) ? "white\n" : "black\n" ));
                set_rgb(pixels, w, h, col, row, val, val, val);
            }
        }

    }
    
    img.syncPixels();
    view.sync();

    printf("Writing image to disk...\n");
    img.write( "images/out_SPN.jpg" );  
}


void jake_driver_AWGN(int argc,char **argv)
{
    printf("Additive White Gaussian Noise Driver...\n");
    InitializeMagick(*argv);
    Image image;
    try { 
        printf("Reading image from disk...\n");
        image.read( "images/black_n_white.jpg");
        

        printf("Generating AWGN...\n");

        if(argc == 1)
        {
            generate_AWGN(image, 10);
        }
        else
        {
            float SNRdB = atof(argv[1]);
            generate_AWGN(image, SNRdB);
        }

    
    } 
    catch( Exception &error_ ) 
    { 
        cout << "Caught exception: " << error_.what() << endl; 
        return;
    }
}

void jake_driver_SPN(int argc,char **argv)
{
    printf("Salt and Pepper Noise Driver...\n");
    InitializeMagick(*argv);
    Image image;
    try { 
        printf("Reading image from disk...\n");
        //image.read( "images/black_n_white.jpg");
        image.read( "images/color_gradient.jpg");

        printf("Generating MSN...\n");

        if(argc == 1)
        {
            generate_SPN(image, 0.5);
        }
        else
        {
            float ctpr = atof(argv[1]); // countToPixelsRatio
            generate_SPN(image, ctpr);
        }  
    } 
    catch( Exception &error_ ) 
    { 
        cout << "Caught exception: " << error_.what() << endl; 
        return;
    }
}

int main(int argc,char **argv) 
{ 
    //jake_driver_AWGN(argc, argv);
    jake_driver_SPN(argc, argv);
    return 0;
}


/*
Jake TODO:

Additive White Gaussian Noise       CHECK
Multiplicative/Speckle Noise        CHECK
noise removal (Gaussian Filtering)

MAKE A SEPARATE PROGRAM FOR EACH TOOL! Pass image name as parameter as well as other arguments needed for tool. 
*/ 
