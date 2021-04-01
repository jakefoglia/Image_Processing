export MAGICK_HOME="/usr/include/ImageMagick-6"
export PATH="$MAGICK_HOME/bin:$PATH"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}$MAGICK_HOME/lib"
export PKG_CONFIG_PATH="$MAGICK_HOME/lib/pkgconfig"

#sudo apt install graphicsmagick-libmagick-dev-compat
#mkdir objs
