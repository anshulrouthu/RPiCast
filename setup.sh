#*********************************************************************
# RPiCast ( Screencasting application using RaspberryPi )
#
# Copyright (C) 2014-2018 Anshul Routhu <anshul.m67@gmail.com>
#
# All rights reserved.
#
# This file setup.sh is part of RPiCast project
#
# RPiCast is a free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*********************************************************************/

#!/bin/sh

mkdir -p staging
mkdir -p extras
cd extras

error_check()
{
    if [ "$?" != "0" ]; then
        echo "$1" 1>&2
        exit 1
    fi
}

#building ffmpeg 
echo ""
echo "##############################"
echo "#      Building FFMPEG       #"
echo "##############################"
echo ""
if [ ! -f "ffmpeg-2.2.1.tar.gz" ]; then
    wget http://www.ffmpeg.org/releases/ffmpeg-2.2.1.tar.gz 
fi
tar xvzf ffmpeg-2.2.1.tar.gz > /dev/null
cd ffmpeg-2.2.1
./configure --prefix=$PWD/../../staging \
	    --enable-gpl       \
        --enable-x11grab   \
        --enable-libx264   \
	    --enable-parsers \
	    --disable-ffprobe \
	    --disable-ffplay \
	    --disable-ffserver \
	    --disable-ffmpeg \
	    --enable-shared \
	    --disable-doc \
	    --disable-postproc \
        --enable-protocols \
	    --enable-nonfree \
	    --enable-openssl \
	    --enable-pthreads \
	    --disable-decoder=vc1_crystalhd \
	    --disable-decoder=wmv3_crystalhd \
	    --disable-decoder=wmv3_vdpau \
	    --disable-decoder=mpeg1_vdpau \
	    --disable-decoder=mpeg2_crystalhd \
	    --disable-decoder=mpeg4_crystalhd \
	    --disable-decoder=mpeg4_vdpau \
	    --disable-decoder=mpeg_vdpau \
	    --disable-decoder=mpeg_xvmc \
	    --disable-decoder=msmpeg4_crystalhd \
	    --disable-decoder=vc1_vdpau \
	    --enable-decoder=mpegvideo \
	    --enable-decoder=mpeg1video \
	    --enable-decoder=mpeg2video \
	    --disable-decoder=mvc1 \
	    --disable-decoder=mvc2 \
	    --disable-decoder=h261 \
	    --disable-decoder=h263 \
	    --disable-decoder=rv10 \
	    --disable-decoder=rv20 \
	    --enable-decoder=mjpeg \
	    --enable-decoder=mjpegb \
	    --disable-decoder=sp5x \
	    --disable-decoder=jpegls \
	    --enable-decoder=mpeg4 \
	    --disable-decoder=msmpeg4v1 \
	    --disable-decoder=msmpeg4v2 \
	    --disable-decoder=msmpeg4v3 \
	    --disable-decoder=wmv1 \
	    --disable-decoder=wmv2 \
	    --disable-decoder=h263p \
	    --disable-decoder=h263i \
	    --disable-decoder=svq1 \
	    --disable-decoder=svq3 \
	    --disable-decoder=dvvideo \
	    --disable-decoder=huffyuv \
	    --disable-decoder=cyuv \
	    --enable-decoder=h264 \
	    --disable-decoder=indeo3 \
	    --disable-decoder=vp3 \
	    --enable-decoder=theora \
	    --disable-decoder=asv1 \
	    --disable-decoder=asv2 \
	    --disable-decoder=ffv1 \
	    --disable-decoder=vcr1 \
	    --disable-decoder=cljr \
	    --disable-decoder=mdec \
	    --disable-decoder=roq \
	    --disable-decoder=xan_wc3 \
	    --disable-decoder=xan_wc4 \
	    --disable-decoder=rpza \
	    --disable-decoder=cinepak \
	    --disable-decoder=msrle \
	    --disable-decoder=msvideo1 \
	    --disable-decoder=idcin \
	    --disable-decoder=smc \
	    --disable-decoder=flic \
	    --disable-decoder=truemotion1 \
	    --disable-decoder=vmdvideo \
	    --disable-decoder=mszh \
	    --disable-decoder=zlib \
	    --disable-decoder=qtrle \
	    --disable-decoder=snow \
	    --disable-decoder=tscc \
	    --disable-decoder=ulti \
	    --disable-decoder=qdraw \
	    --disable-decoder=qpeg \
	    --disable-decoder=png \
	    --disable-decoder=ppm \
	    --disable-decoder=pbm \
	    --disable-decoder=pgm \
	    --disable-decoder=pgmyuv \
	    --disable-decoder=pam \
	    --disable-decoder=ffvhuff \
	    --disable-decoder=rv30 \
        --disable-decoder=rv40 \
	    --enable-decoder=vc1 \
	    --enable-decoder=wmv3 \
	    --disable-decoder=loco \
	    --disable-decoder=wnv1 \
	    --disable-decoder=aasc \
	    --disable-decoder=indeo2 \
	    --disable-decoder=fraps \
	    --disable-decoder=truemotion2 \
	    --disable-decoder=bmp \
	    --disable-decoder=cscd \
	    --disable-decoder=mmvideo \
	    --disable-decoder=zmbv \
	    --disable-decoder=avs \
	    --disable-decoder=nuv \
	    --disable-decoder=kmvc \
	    --disable-decoder=flashsv \
	    --disable-decoder=cavs \
	    --disable-decoder=jpeg2000 \
	    --disable-decoder=vmnc \
	    --disable-decoder=vp5 \
	    --enable-decoder=vp6 \
	    --enable-decoder=vp6f \
	    --disable-decoder=targa \
	    --disable-decoder=dsicinvideo \
	    --disable-decoder=tiertexseqvideo \
	    --disable-decoder=tiff \
	    --disable-decoder=gif \
	    --disable-decoder=dxa \
	    --disable-decoder=dnxhd \
	    --disable-decoder=thp \
	    --disable-decoder=sgi \
	    --disable-decoder=c93 \
	    --disable-decoder=bethsoftvid \
	    --disable-decoder=ptx \
	    --disable-decoder=txd \
	    --disable-decoder=vp6a \
	    --disable-decoder=amv \
	    --disable-decoder=vb \
	    --disable-decoder=pcx \
	    --disable-decoder=sunrast \
	    --disable-decoder=indeo4 \
	    --disable-decoder=indeo5 \
	    --disable-decoder=mimic \
	    --disable-decoder=rl2 \
	    --disable-decoder=escape124 \
	    --disable-decoder=dirac \
	    --disable-decoder=bfi \
	    --disable-decoder=motionpixels \
	    --disable-decoder=aura \
	    --disable-decoder=aura2 \
	    --disable-decoder=v210x \
	    --disable-decoder=tmv \
	    --disable-decoder=v210 \
	    --disable-decoder=dpx \
	    --disable-decoder=frwu \
	    --disable-decoder=flashsv2 \
	    --disable-decoder=cdgraphics \
	    --disable-decoder=r210 \
	    --disable-decoder=anm \
	    --disable-decoder=iff_ilbm \
	    --disable-decoder=iff_byterun1 \
	    --disable-decoder=kgv1 \
 	    --disable-decoder=yop \
	    --enable-decoder=vp8 \
	    --disable-decoder=webp \
	    --disable-decoder=pictor \
	    --disable-decoder=ansi \
	    --disable-decoder=r10k \
	    --disable-decoder=mxpeg \
	    --disable-decoder=lagarith \
	    --disable-decoder=prores \
	    --disable-decoder=jv \
	    --disable-decoder=dfa \
	    --disable-decoder=wmv3image \
	    --disable-decoder=vc1image \
	    --disable-decoder=utvideo \
	    --disable-decoder=bmv_video \
	    --disable-decoder=vble \
	    --disable-decoder=dxtory \
	    --disable-decoder=v410 \
	    --disable-decoder=xwd \
	    --disable-decoder=cdxl \
	    --disable-decoder=xbm \
	    --disable-decoder=zerocodec \
	    --disable-decoder=mss1 \
	    --disable-decoder=msa1 \
	    --disable-decoder=tscc2 \
	    --disable-decoder=mts2 \
	    --disable-decoder=cllc \
	    --disable-decoder=mss2 \
	    --disable-decoder=y41p \
	    --disable-decoder=escape130 \
	    --disable-decoder=exr \
	    --disable-decoder=avrp \
	    --disable-decoder=avui \
	    --disable-decoder=ayuv \
	    --disable-decoder=v308 \
	    --disable-decoder=v408 \
	    --disable-decoder=yuv4 \
	    --disable-decoder=sanm \
	    --disable-decoder=paf_video \
	    --disable-decoder=avrn \
	    --disable-decoder=cpia \
	    --disable-decoder=bintext \
	    --disable-decoder=xbin \
	    --disable-decoder=idf \
        && make -j8 && make install
error_check "ffmgeg build failed! Aborting.\n"  
cd ../

#building the unittest++
echo ""
echo "##############################"
echo "#    Building UnitTest++     #"
echo "##############################"
echo ""
git clone https://github.com/unittest-cpp/unittest-cpp.git UnitTest++
cd UnitTest++
cd builds
cmake -DCMAKE_INSTALL_PREFIX=$PWD/../../../staging/ ../
make all -j8 && make install
error_check "UnitTest++ build failed! Aborting.\n"
cd ../../../

#get cross compiler tool chain for raspberrypi
echo ""
echo "##############################"
echo "#    Get Cross Toolchain     #"
echo "##############################"
echo ""
mkdir -p rpi-toolchain
cd rpi-toolchain
if [ ! -d ".git" ]; then
    git clone https://github.com/raspberrypi/tools.git ./
fi
error_check "Unable to get toolchain for raspberry-pi! Aborting.\n"
cd ../

#create staging dir for raspberrypi
echo ""
echo "##############################"
echo "#     Create RPi Staging     #"
echo "##############################"
echo ""
mkdir -p rpi-staging
cd rpi-staging
#wget tar file 
if [ ! -f "rpi-staging-arch-linux.tar.gz" ]; then
    wget https://www.dropbox.com/s/a85oklmxbbgtdtd/rpi-staging-arch-linux.tar.gz?dl=0 -O rpi-staging-arch-linux.tar.gz
fi
tar xvzf rpi-staging-arch-linux.tar.gz ./
error_check "Unable to get staging files for raspberry-pi! Aborting.\n"
cd ../

echo "Setup script completed successfully"
