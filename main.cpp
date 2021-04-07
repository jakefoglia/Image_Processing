
/*
    https://imagemagick.org/script/magick++.php#install
*/
  
#include "Magick++.h" // version 7
#include <stdio.h>
#include <iostream> 

using namespace std; 
using namespace Magick; 

int main(int argc,char **argv) 
{ 
    printf("Hello world\n");
    InitializeMagick(*argv);

    // Construct the image object. Seperating image construction from the 
    // the read operation ensures that a failure to read the image file 
    // doesn't render the image object useless. 
    Image image;
    try { 
        // Read a file into image object 
        image.read( "images/color_gradient.jpg" );
        int w = image.columns();
        int h = image.rows();

        image.type(TrueColorType);
        image.modifyImage();
        //MagickCore::Quantum *pixels = image.getPixels(0, 0, w, h);

        Pixels view(image);
        Color green("green"); 

        //Quantum *pixels = view.get( (int)(w/2),(int)(h/2),w,h); 
        int start_row = (h/2);
        int start_col = (w/2);
        Quantum *pixels = view.get(start_col,start_row,w-start_col,h-start_row); 

        for(int col = 0; col < w - start_col; col++)
        {
            for(int row = 0; row < h - start_row; row++)
            {
                *pixels++=QuantumRange*green.quantumRed();
                *pixels++=QuantumRange*green.quantumGreen();
                *pixels++=QuantumRange*green.quantumBlue();
            }
        }
        image.syncPixels();
         view.sync();
        // Crop the image to specified size (width, height, xOffset, yOffset)
        //image.crop( Geometry(100,100, 100, 100) );

        
        // Write the image to a file 
        image.write( "images/out.jpg" ); 


    } 
    catch( Exception &error_ ) 
    { 
        cout << "Caught exception: " << error_.what() << endl; 
        return 1; 
    }
 
    return 0; 
}


/*
Jake TODO:

Additive White Gaussian Noise
Multiplicative/Speckle Noise

noise removal (Gaussian Filtering)
*/