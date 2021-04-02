# Image_Processing
    CPE462 Image Enhancement Project


# Windows Setup Instructions

**Obtain the cygwin64 installer**
    https://cygwin.com/install.html  >  setup-x86_64.exe

**Run the installer and do the following**
    Set the root directory to something like C:\cygwin64
    Set the Local Package Directory to something like C:\cygwin64-Local-Pkg-Dir
    Leave the rest on default settings, select any mirror when it asks. 
    
    Install the c++ development tools using this tutorial
    https://www.youtube.com/watch?v=DAlS4hF_PbY&t=4
    
    Double check that the following packages are installed:
        lynx, wget, ImageMagick
        
    Click next twice to finish the setup. The download will likely take ~10 minutes or so. 
 
 
**Manually install cyg-apt 
    Right click on the link https://raw.githubusercontent.com/transcode-open/apt-cyg/master/apt-cyg
    Select "Save link as" and save it to "c:\cygwin64\bin" (or wherever your Cygwin bin folder is)
    Rename the downloaded file and remove the .txt extension so that it's just apt-cyg
    Open your cygwin64 Terminal and enter the following commands
        
        $ sudo chmmod +x apt-cyg 
        $ apt-cyg update
        
    Verify apt-cyg is working with the following command

        $ apt-cyg install nano
        
**Navigate to your home directory and create a directory for Git**
    
        $ cd ~
        $ mkdir Git

**Clone the Image_Processing Git Repository**

        $ cd Git
        $ git clone https://github.com/jakefoglia/Image_Processing.git
        $ cd Image_Processing

**Try to build and run the project**
    Enter the following commands
        $ make exe
        $ ./a.exe
        
    If it doesn't build, there is a good chance its an issue with the Makefile
    Try the following commands instead
    
        $ g++ `Magick++-config --cppflags` main.cpp `Magick++-config --ldflags --libs`
        $ ./a.exe

    If the issue persists, you may be missing necessary components, or our code is bugged.
