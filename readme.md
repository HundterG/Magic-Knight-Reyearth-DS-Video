Repository to store my attempt at getting full speed video playing on a Nintendo DS. It is broken up into 5 sections. 4 backups of the DS program at various times and a working directory that was used to convert all of the video frames into their compressed DS formats.

V1
This was the first attempt using a slightly customized QOI encoding for the video and streaming raw 16bit pcm data from the cart. Other than slowdown, only the top screen works. This is due to the DS only allowing raw pixel data to be presented on the screen for the main renderer. Some attempts were made to use a background layer for the bottom screen but it was apparent that the drawing board needed to be revisited.

NOTE: Video.qoia has been compressed to fit in Githubs upload requirements.

V2
This version switched to using a modified DXT compression for the video. Both screens are visible here using 24 tiled sprites, 12 for each screen in a 4x3 layout.

Sprites on the DS work differently than one would expect. Every 8 pixels horizontally, the data will wrap to the next line. After 8 lines, it will start drawing where the 9th pixel would have been on a normal renderer.
(sprite_pixel_layout_16x16.png)

Despite the decoding algorithm being about 20% faster than the QOI, the same frame slowness existed. It was also possible to have the video go into an endless loop at the end instead of bringing up the stats.

NOTE: Video.dxta has been compressed to fit in Githubs upload requirements.

V3
Looked to the audio to try to track down the slowness. The audio was re-sampled to 32000Hz and the slowness improved a little. Removing all audio made the video run at 20fps, almost full speed. There is a lot of color popping here due to the colors and the frame data updating independently and potentially over a screen sync boundary.

V4 - Final version
DXT compression was modified to store color data for 2 tiles in one pallet instead of one, effectively cutting the final size of the video by 25% and bringing the video playback to full speed. Pallets were also changed to cover a number of frames instead of 1. This reduces the color popping but introduces some color inaccuracies. Attempts were made to reintroduce the audio streaming but any attempt to bring it back resulted in the slowness returning. in the end, a mono 8bit audio stream running at 22050Hz is just small enough to fit fully in main memory. So it was shifted to loading it all at the beginning. This brings the playback to a good enough ending.

Future???
If I were to revisit this, it would be to use a smarter palette function to palette the frames with better colors.

Working directory
This directory contains all the files and conversion code to convert the video to its final version on the DS. Due to its rapid and sporadic development, these files are very messy and I do not have any desire to fix that. The first frame is provided for demonstration purposes.
