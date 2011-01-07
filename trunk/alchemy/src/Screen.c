#include "NES.h"
/**
 *
 * <P>
 * Number of Frames drawn since last FPS Count.
 * </P>
 *
 */
int actualFrameNum = 0;
/**
 *
 * <P>
 * The current Frame Skip value.
 * </P>
 *
 */
int frameSkip = 0;
/**
 *
 * <P>
 * The current scanline number.
 * </P>
 *
 */
int scanLine = 0;
/**
 *
 * <P>
 * The Screen Palette.
 * </P>
 *
 */
static int palette[] = { 0xFF808080, 0xFF0658B6, 0xFF4534CA, 0xFF8616BB,
        0xFFB6068E, 0xFFCA084E, 0xFFBB1C0E, 0xFF8E3C00, 0xFF4E6000, 0xFF0E7E00,
        0xFF008E06, 0xFF008C45, 0xFF007886, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFC0C0C0, 0xFF2F81DF, 0xFF6E5DF3, 0xFFAF3FE4, 0xFFDF2FB7, 0xFFF33177,
        0xFFE44437, 0xFFB76506, 0xFF778900, 0xFF37A702, 0xFF06B72F, 0xFF00B56E,
        0xFF02A1AF, 0xFF3D3D3D, 0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFF77C8FF,
        0xFFB6A5FF, 0xFFF686FF, 0xFFFF76FE, 0xFFFF78BF, 0xFFFF8C7E, 0xFFFEAC4E,
        0xFFBFD03A, 0xFF7EEE49, 0xFF4EFE77, 0xFF3AFCB6, 0xFF49E9F6, 0xFF787878,
        0xFF000000, 0xFF000000, 0xFFFFFFFF, 0xFFA2F4FF, 0xFFE1D0FF, 0xFFFFB2FF,
        0xFFFFA2FF, 0xFFFFA4EA, 0xFFFFB8AA, 0xFFFFD879, 0xFFEAFC66, 0xFFAAFF75,
        0xFF79FFA2, 0xFF66FFE1, 0xFF75FFFF, 0xFFC5C5C5, 0xFF000000, 0xFF000000,
        0xFF000000 };
/**
 *
 * <P>
 * Buffer of Colour Entries for Screen.
 * </P>
 *
 */
extern int videoBuffer[256 * 240 * 4];
/**
 *
 * <P>
 * True if the current frame is being skipped.
 * </P>
 *
 */
boolean skipThisFrame = false
;
/**
 *
 * <P>
 * The number of frames skipped since last frame draw.
 * </P>
 *
 */
int framesSkipped = 0;
/**
 *
 * <P>
 * Set the Scanline Manually
 * </P>
 *
 */
boolean setScanLineNum(int sl) {
    // Set the Scanline
    scanLine = sl;
    // Check if we have reached a new Frame
    if (scanLine == 0) {
        // Assume we are going to Skip and increment Counter
        framesSkipped++;
        // Check if we should actually skip this Frame
        if (framesSkipped <= frameSkip) {
            skipThisFrame = true;
        } else {
            framesSkipped = 0;
            skipThisFrame = false;
        }
    }
    return skipThisFrame;
}

/**
 *
 * <P>
 * Sets the frame skip for the TV Controller.
 * </P>
 *
 * @param value
 *            The number of frames to skip before drawing.
 *
 */
void setFrameSkip(int value) {
    frameSkip = value;
}

/**
 *
 * <P>
 * Sets an Array of Pixels representing the current Scanline.
 * </P>
 *
 */
int odd = 0;
void setPixels(byte* palEntries, int lineNum) {
    //	 Check Not First Line
    if (scanLine == 0)
        return;
    if (lineNum < 256)
        return;
    // Set Pixels
    int x;
    int cx;
    int colorIndex;

    for (x = 0; x < 256; x++) {
        colorIndex = palEntries[x];
//        cx = x<<1;
        videoBuffer[((scanLine << 8)) | x] = palette[colorIndex];
//        videoBuffer[(((scanLine << 10)) | cx) + 1] = palette[colorIndex];
    }

//    if(odd){
//        odd=0;
//        for (x = 0; x < 256; x++) {
//            colorIndex = palEntries[x];
//            cx = x<<1;
//            videoBuffer[((scanLine << 10)) | cx] = palette[colorIndex];
//            videoBuffer[(((scanLine << 10)) | cx) + 1] = palette[colorIndex];
//        }
//    }
//    else
//    {
//        odd=1;
//        for (x = 0; x < 256; x++) {
//            colorIndex = palEntries[x];
//            cx = x<<1;
//            videoBuffer[((scanLine << 10)+512) | cx] = palette[colorIndex];
//            videoBuffer[(((scanLine << 10)+512) | cx) + 1] = palette[colorIndex];
//        }
//    }
}
