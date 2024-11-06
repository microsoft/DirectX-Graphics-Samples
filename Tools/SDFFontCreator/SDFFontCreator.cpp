#include <fstream>
#include <set>
#include <cmath>
#include <ft2build.h>
#include <cstdint>
#include <thread>
#include <vector>
#include <algorithm>
#include <intrin.h>

#include FT_FREETYPE_H

#define kMajorVersion    1
#define kMinorVersion    0

#define kMaxTextureDimension 4096

using namespace std;

template <typename T> inline T align16( T val ) { return (val + 15) & ~15; }

// All measurements are in 12.4 fixed point
struct GlyphInfo
{
    wchar_t c;          // The unicode (UCS2) character code corresponding to the glyph
    uint16_t u, v;      // The upper left texture coordinate of the glyph, not counting border space
    uint16_t width;     // The width of the glyph (not counting horizontal spacing) 
    int16_t bearing;    // The leading space before the glyph (sometimes negative)
    uint16_t advance;   // The total distance to advance the pen after printing
};

__declspec(thread) FT_Library g_FreeTypeLib = 0;    // FreeType2 library wrapper
__declspec(thread) FT_Face g_FreeTypeFace = 0;      // FreeType2 typeface
uint16_t g_numGlyphs = 0;       // Number of glyphs to process
GlyphInfo g_glyphs[0xFFFF];     // An array of glyph information
uint16_t g_borderSize = 0;      // Extra space around each glyph used for effects like glow and drop shadow
uint16_t g_maxDistance = 0;     // Range of search space which controls the "steepness" of the contour map
bool g_fixNumberWidths = false; // Prints all numbers with fixed spacing
uint16_t g_maxGlyphHeight = 0;  // Max height of glyph = ascender - descender
int16_t g_fontOffset = 0;       // Baseline offset to center the text vertically
uint16_t g_fontAdvanceY = 0;    // Distance from baseline to baseline (line height)

float* g_DistanceMap = 0;
uint32_t g_MapWidth = 0;
uint32_t g_MapHeight = 0;
volatile int32_t g_nextGlyphIdx = 0;
volatile bool g_ReadyToPaint = false;

void PrintAssertMessage( const char* file, uint32_t line, const char* cond, const char* msg, ...)
{
    printf("Assertion \"%s\" failed at %s:%u\n", cond, file, line);

    if (msg)
    {
        va_list ap;
        va_start(ap, msg);
        vprintf(msg, ap);
        printf("\n");
    }
}

// Assert which allows setting of file and line number
#define ASSERT_LINE( test, file, line, ... ) \
    if (!(test)) { \
        PrintAssertMessage(file, line, #test, ##__VA_ARGS__); \
        __debugbreak(); \
    }

// Assert which automatically detects call location
#define ASSERT( test, ... ) ASSERT_LINE( test, __FILE__, __LINE__, __VA_ARGS__ )

void InitializeFont( const char* filename = 0, uint32_t size = 0)
{
    static const char* s_filename = 0;
    static uint32_t s_size = 0;

    if (s_filename == 0)
    {
        ASSERT(filename != 0, "We must have a TrueType file to initialize FreeType");
        ASSERT(size > 0, "A size must be specified to initialize FreeType");
    }
    else
    {
        filename = s_filename;
        size = s_size;
    }

    ASSERT(g_FreeTypeLib == 0, "FreeType is already initialized on this thread");

    try
    {
        if (FT_Init_FreeType( &g_FreeTypeLib ))
            throw exception("Failed to create library\n");
        if (FT_New_Face( g_FreeTypeLib, filename, 0, &g_FreeTypeFace ))
            throw exception("Failed to create face\n");
        if (FT_Set_Pixel_Sizes( g_FreeTypeFace, 0, size ))
            throw exception("Failed to set pixel sizes\n");
    }
    catch (exception& e)
    {
        // If this is the first time, that filename is probably bad.  Raise the error.
        if (s_filename == 0)
            throw;

        printf("Error:  Unable to initialize FreeType for thread: %s\n", e.what());
    }

    s_filename = filename;
    s_size = size;
}

void ShutdownFont(void)
{
    if (g_FreeTypeFace)
        FT_Done_Face( g_FreeTypeFace );

    if (g_FreeTypeLib)
        FT_Done_FreeType( g_FreeTypeLib );
}

struct Canvas
{
    uint8_t* bitmap;    // Pointer to rendered glyph memory (from FT_Bitmap)
    uint32_t pitch;        // Width in bytes of a row in the canvas
    uint32_t width;        // Width in pixels of a row in the canvas
    uint32_t rows;        // Number of rows in the canvas
    uint32_t xOff;        // Amount to offset the x coordinate when reading
    uint32_t yOff;        // Amount to offset the y coordinate when reading
};

// Access the high res glyph canvas
inline bool ReadCanvasBit( const Canvas& canvas, uint32_t x, uint32_t y )
{
    // Notice that negative values have been cast to large positive values
    x -= canvas.xOff;
    y -= canvas.yOff;
    if (x >= canvas.width || y >= canvas.rows)
        return false;

    uint32_t p = y * canvas.pitch + x / 8;
    uint32_t k = x & 7;
    return (canvas.bitmap[p] & (0x80 >> k)) ? true : false;
}

// Setup pixel reads from the glyph canvas
inline Canvas LoadCanvas(FT_GlyphSlot glyph)
{
    Canvas ret;
    ret.bitmap = glyph->bitmap.buffer;
    ret.pitch = glyph->bitmap.pitch;
    ret.width = glyph->bitmap.width;
    ret.rows = glyph->bitmap.rows;
    ret.xOff = g_borderSize * 16;
    ret.yOff = g_borderSize * 16 + g_maxGlyphHeight + g_fontOffset - (glyph->metrics.horiBearingY >> 6);
    return ret;
}

float DistanceFromInside(const Canvas& canvas, uint32_t xCoord, uint32_t yCoord)
{
    const uint32_t radius = g_maxDistance * 32;
    const uint32_t x0 =    xCoord * 32 + 15;
    const uint32_t y0 =    yCoord * 32 + 15;

    uint32_t left =        (uint32_t)max(0, (int32_t)(x0 - radius + 1));
    uint32_t right =    x0 + radius - 1;
    uint32_t top =        (uint32_t)max(0, (int32_t)(y0 - radius + 1));
    uint32_t bottom =    y0 + radius - 1;

    uint32_t bestDistSq = radius * radius;

    for (uint32_t y1 = top; y1 <= bottom; y1 += 2)
    {
        int32_t distY = (int32_t)(y1 - y0);
        uint32_t distYSq = (uint32_t)(distY * distY);

        if (distYSq >= bestDistSq)
        {
            if (y1 > y0)
                continue;
            else
                break;
        }

        for (uint32_t x1 = left; x1 <= right; x1 += 2)
        {
            int32_t distX = (int32_t)(x1 - x0);
            uint32_t distXSq = (uint32_t)(distX * distX);

            uint32_t distSq = (uint32_t)(distXSq + distYSq);
            if (distSq < bestDistSq && !ReadCanvasBit(canvas, x1 >> 1, y1 >> 1))
                bestDistSq = distSq;
        }
    }

    return sqrt((float)bestDistSq) / (float)radius;
}

float DistanceFromOutside(const Canvas& canvas, uint32_t xCoord, uint32_t yCoord)
{
    const uint32_t radius = g_maxDistance * 32;
    const uint32_t x0 =    xCoord * 32 + 15;
    const uint32_t y0 =    yCoord * 32 + 15;

    uint32_t left =        (uint32_t)max(0, (int32_t)(x0 - radius + 1));
    uint32_t right =    x0 + radius - 1;
    uint32_t top =        (uint32_t)max(0, (int32_t)(y0 - radius + 1));
    uint32_t bottom =    y0 + radius - 1;

    uint32_t bestDistSq = radius * radius;

    for (uint32_t y1 = top; y1 <= bottom; y1 += 2)
    {
        int32_t distY = (int32_t)(y1 - y0);
        uint32_t distYSq = (uint32_t)(distY * distY);

        if (distYSq >= bestDistSq)
        {
            if (y1 > y0)
                continue;
            else
                break;
        }

        for (uint32_t x1 = left; x1 <= right; x1 += 2)
        {
            int32_t distX = (int32_t)(x1 - x0);
            uint32_t distXSq = (uint32_t)(distX * distX);

            uint32_t distSq = (uint32_t)(distXSq + distYSq);
            if (distSq < bestDistSq && ReadCanvasBit(canvas, x1 >> 1, y1 >> 1))
                bestDistSq = distSq;
        }
    }

    return sqrt((float)bestDistSq) / (float)radius;
}

// Get width and spacing of a given glyph to compute necessary space and layout in final texture.
inline uint16_t GetGlyphMetrics( wchar_t c, GlyphInfo& info )
{
    if (FT_Load_Char( g_FreeTypeFace, c, FT_LOAD_TARGET_MONO ))
        throw exception("Unable to access glyph data");

    FT_Glyph_Metrics& metrics = g_FreeTypeFace->glyph->metrics;
    info.bearing = (int16_t)(metrics.horiBearingX >> 6);
    info.width = (uint16_t)(metrics.width >> 6);
    info.advance =  (uint16_t)(metrics.horiAdvance >> 6);

    return (uint16_t)info.width;
}

// Compute glyph layout in bitmap for a given texture width.  If the height exceeds a certain
// threshold, you should recompute the layout with a larger texture width.
uint32_t UnwrapUVs(uint32_t textureWidth)
{
    uint16_t glyphBorder = g_borderSize * 16;
    uint16_t rowSize = align16(g_maxGlyphHeight);

    // Set the starting point for the drawing 
    uint32_t x = glyphBorder;
    uint32_t y = glyphBorder;

    for (uint16_t i = 0; i < g_numGlyphs; ++i) 
    {
        // We need a pixel border to surround the character because the distance field must enclose
        // the bitmap.
        uint32_t deltaX = align16(g_glyphs[i].width) + glyphBorder;

        // Check for line wrap
        if (x + deltaX > textureWidth * 16) 
        {
            x = glyphBorder;
            y += rowSize + glyphBorder * 2;
        }

        // The actual character UVs don't include the border pixels
        g_glyphs[i].u = (uint16_t)x;
        g_glyphs[i].v = (uint16_t)y;

        x += deltaX + glyphBorder;
    }

    return (y + rowSize + glyphBorder) / 16;
}

void PaintCharacters( float* distanceMap, uint32_t width, uint32_t /*height*/ )
{
    int32_t i = -1;
    while ((i = _InterlockedExchangeAdd((volatile long*)&g_nextGlyphIdx, 1)) < g_numGlyphs)
    {
        // Get the character info
        const GlyphInfo& ch = g_glyphs[i];

        if (FT_Load_Char( g_FreeTypeFace, ch.c, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO ))
            throw exception("Character bitmap rendering failed internally");

        Canvas canvas = LoadCanvas(g_FreeTypeFace->glyph);

        uint32_t charWidth = align16(ch.width) / 16;
        uint32_t charHeight = align16(g_maxGlyphHeight) / 16;
        uint32_t startX = ch.u / 16 - g_borderSize;
        uint32_t startY = ch.v / 16 - g_borderSize;

        // Convert high-res bitmap to low-res distance map
        for (uint32_t x = 0; x < charWidth + g_borderSize * 2; ++x)
        {
            for (uint32_t y = 0; y < charHeight + g_borderSize * 2; ++y)
            {
                uint32_t left = x * 16 + 7;
                uint32_t top = y * 16 + 7;

                bool inside = ReadCanvasBit(canvas, left, top) & ReadCanvasBit(canvas, left + 1, top) &
                    ReadCanvasBit(canvas, left, top + 1) & ReadCanvasBit(canvas, left + 1, top + 1);

                if (inside)
                    distanceMap[startX + x + (startY + y) * width] = +DistanceFromInside(canvas, x, y);
                else
                    distanceMap[startX + x + (startY + y) * width] = -DistanceFromOutside(canvas, x, y);
            }
        }
    }
}

void WorkerFunc( void )
{
    // We can initialize FreeType while we wait to paint the alphabet
    InitializeFont();

    // Sleep for 2 milliseconds per iteration until we're ready to paint
    while (!g_ReadyToPaint)
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

    PaintCharacters(g_DistanceMap, g_MapWidth, g_MapHeight);

    ShutdownFont();
}

struct BMP_Header
{
    // Bitmap file header
    uint32_t filesz;
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;

    // DIB header
    uint32_t header_sz;
    uint32_t width;
    uint32_t height;
    uint16_t nplanes;
    uint16_t bitspp;
    uint32_t compress_type;
    uint32_t bmp_bytesz;
    uint32_t hres;
    uint32_t vres;
    uint32_t ncolors;
    uint32_t nimpcolors;
};

void WritePreviewBMP(const string& fileName, const uint8_t* heightMap, uint32_t width, uint32_t height)
{
    // Append ".bmp" to file name
    char fileWithSuffix[256];
    sprintf_s(fileWithSuffix, 256, "%s.bmp", fileName.c_str());

    // Open file
    ofstream file;
    file.exceptions(ios_base::failbit | ios_base::badbit);
    file.open(fileWithSuffix, ios_base::out | ios_base::binary | ios_base::trunc);

    file.write("BM", 2);

    // Write header
    BMP_Header header;
    memset(&header, 0, sizeof(BMP_Header));
    header.filesz = 54 + 256 * sizeof(uint32_t) + width * height;
    header.bmp_offset = 54 + 256 * sizeof(uint32_t);
    header.header_sz = 40;
    header.width = width;
    header.height = -(int32_t)height;
    header.nplanes = 1;
    header.bitspp = 8;
    header.hres = 0x130B0000;
    header.vres = 0x130B0000;
    file.write((const char*)&header, sizeof(BMP_Header));

    // Write color map table
    uint32_t colorMap[256];
    for (uint32_t i = 0; i < 256; ++i)
        colorMap[i] = i << 16 | i << 8 | i;
    file.write((const char*)colorMap, 256 * sizeof(uint32_t));

    // Write data
    file.write((const char*)heightMap, width * height);

    // Close file
    file.close();
}

void CompileFont(const string& outputName)
{
    // Figure out how many threads to create, and then spawn them.  Leave their parameters blank.  We'll fill them in
    // before they're read.

    size_t numThreads = std::thread::hardware_concurrency();

    std::vector<std::thread> Threads;

    if (numThreads > 1)
    {
        --numThreads;

        for (uint32_t i = 0; i < numThreads; ++i)
        {
            Threads.push_back(std::thread(WorkerFunc));
        }
    }

    // This implicitly embeds space in the font texture, which wastes memory.  What would be better is to just store
    // the font height (i.e. the line spacing) in the final file header, and pack the texture as tightly as possible.
    g_fontAdvanceY = (uint16_t)(g_FreeTypeFace->size->metrics.height >> 6);
    g_maxGlyphHeight = (uint16_t)((g_FreeTypeFace->size->metrics.ascender - g_FreeTypeFace->size->metrics.descender) >> 6);
    g_fontOffset = (int16_t)(g_FreeTypeFace->size->metrics.descender >> 6);

    // Get the dimensions of each glyph
    for (uint16_t i = 0; i < g_numGlyphs; ++i) 
        GetGlyphMetrics(g_glyphs[i].c, g_glyphs[i]);

    // Adjust metrics for numerals if we want to coerce fixed number widths
    if (g_fixNumberWidths)
    {
        // Compute maximum numeric advance
        uint16_t numberWidth = 0;
        for (uint16_t i = 0; i < g_numGlyphs; ++i) 
        {
            wchar_t wc = g_glyphs[i].c;
            if (wc >= L'0' && wc <= L'9')
                numberWidth = max(numberWidth, g_glyphs[i].advance);    
        }

        // Adjust each numeral to advance the same amount and to center the digit by adjusting the bearing
        for (uint16_t i = 0; i < g_numGlyphs; ++i) 
        {
            wchar_t wc = g_glyphs[i].c;
            if (wc >= L'0' && wc <= L'9')
            {
                g_glyphs[i].bearing = (numberWidth - g_glyphs[i].width) / 2;
                g_glyphs[i].advance = numberWidth;
            }
        }
    }

    // Compute the smallest rectangular texture with height < width that can contain the result.  Use
    // widths that are a power of two to accelerate the search.
    for (g_MapWidth = 512; g_MapWidth <= kMaxTextureDimension; g_MapWidth *= 2)
    {
        g_MapHeight = UnwrapUVs(g_MapWidth);

        // Found a good size
        if (g_MapHeight <= g_MapWidth)
            break;
    }

    // We ran through all possibilities and still couldn't fit the font
    if (g_MapHeight > g_MapWidth)
        throw exception("Texture dimensions exceeded maximum allowable");

    // Render the glyphs and generate heightmaps.  Place heightmaps in the
    // locations set aside in the texture.
    g_DistanceMap = new float[g_MapWidth * g_MapHeight];
    for (size_t x = g_MapWidth * g_MapHeight; x > 0; --x)
        g_DistanceMap[x - 1] = -1.0f;

    // Make sure all of the parameters are flushed to memory before we trigger the threads to paint.
    __faststorefence();

    g_ReadyToPaint = true;

    // Also paint on the main thread
    PaintCharacters(g_DistanceMap, g_MapWidth, g_MapHeight);

    // Wait for all of the other threads
    if (numThreads > 0)
    {
        for_each( Threads.begin(), Threads.end(), []( std::thread& T ) { T.join(); } );
    }

    uint8_t* compressedMap8 = new uint8_t[g_MapWidth * g_MapHeight];

    for (uint32_t i = 0; i < g_MapWidth * g_MapHeight; ++i)
        compressedMap8[i] = (uint8_t)(g_DistanceMap[i] * 127.0f + 127.0f);    // (Omit 255)

    WritePreviewBMP(outputName, compressedMap8, g_MapWidth, g_MapHeight);

    for (uint32_t i = 0; i < g_MapWidth * g_MapHeight; ++i)
        compressedMap8[i] = (int8_t)(g_DistanceMap[i] * 127.0f);

    // Append ".fnt" to file name
    char fileWithSuffix[256];
    sprintf_s(fileWithSuffix, 256, "%s.fnt", outputName.c_str());
    ofstream file;
    file.exceptions(ios_base::failbit | ios_base::badbit);
    file.open(fileWithSuffix, ios_base::out | ios_base::binary | ios_base::trunc);
    
    // The first part of the file is an identifier string to verify format and version
    const char* idString = "SDFFONT";
    file.write(idString, 8);

    // The second part is information about the font
    struct FontHeader
    {
        uint8_t  majorVersion;
        uint8_t  minorVersion;
        uint16_t borderSize;
        uint16_t textureWidth;
        uint16_t textureHeight;
        uint16_t fontHeight;
        uint16_t advanceY;
        uint16_t numGlyphs;
        uint16_t searchDist;
    } header;

    header.majorVersion = kMajorVersion;
    header.minorVersion = kMinorVersion;
    header.borderSize = (uint8_t)g_borderSize;
    header.textureWidth = (uint16_t)g_MapWidth;
    header.textureHeight = (uint16_t)g_MapHeight;
    header.fontHeight = g_maxGlyphHeight;
    header.advanceY = g_fontAdvanceY;
    header.numGlyphs = g_numGlyphs;
    header.searchDist = g_maxDistance * 16;
    file.write((const char*)&header, sizeof(FontHeader));

    for (size_t i = 0; i < g_numGlyphs; ++i)
        file.write( (const char*)&g_glyphs[i].c, 2 );

    for (size_t i = 0; i < g_numGlyphs; ++i)
        file.write( (const char*)&g_glyphs[i].c + 2, sizeof(GlyphInfo) - 2 );

    file.write((const char*)compressedMap8, g_MapWidth * g_MapHeight);

    file.close();

    printf("Finished creating %s\n", fileWithSuffix);
}

void main( int argc, const char** argv )
{
    string inputFile = "";
    string outputName = "";
    string characterSet = "ASCII";
    uint32_t size = 32;
    bool extendedASCII = false;
    
    try
    {
        if (argc < 2)
            throw exception("No font file specified");
        inputFile = argv[1];

        for (int arg = 2; arg < argc; ++arg)
        {
            if (argv[arg][0] != '-')
                throw exception("Malformed option");

            if (arg + 1 == argc)
                throw exception("Missing operand");
            else if (strcmp("-size", argv[arg]) == 0)
                size = atoi(argv[++arg]);
            else if (strcmp("-output", argv[arg]) == 0)
                outputName = argv[++arg];
            else if (strcmp("-character_set", argv[arg]) == 0)
                characterSet = argv[++arg];
            else if (strcmp("-border", argv[arg]) == 0)
                g_borderSize = (uint16_t)atoi(argv[++arg]);
            else if (strcmp("-radius", argv[arg]) == 0)
                g_maxDistance = (uint16_t)atoi(argv[++arg]);
            else
                throw exception("Invalid option");
        }
    }
    catch (exception& e)
    {
        printf(
            "Error: %s\n\n"
            "Usage:  %s <TrueType font file> [options]*\n\n"
            "Options:\n\n"
            "-character_set <filename | \"ASCII\" | \"EXTENDED\">\n"
            "\tA file containing the requested character set.\n\tWhen omitted, defaults to printable ASCII.\n"
            "-output <string>\n\tThe root name of resultant files.\n"
            "-size <integer>\n\tThe font pixel resolution.\n"
            "-radius <integer>\n\tThe search radius.\n\tDefaults to font size / 8.\n"
            "-border_size <integer>\n\tExtra spacing around glyphs for various effects.\n\tDefaults to the search radius.\n"
            "\n\nExample:  %s myfont.ttf -character_set Japanese.txt -output japanese\n\n", e.what(), argv[0], argv[0]);
        return;
    }

    if (outputName.length() == 0)
    {
        outputName = inputFile.substr(0, inputFile.rfind('.'));
        char sizeAsString[128];
        sprintf_s(sizeAsString, 128, "%u", size);
        outputName += sizeAsString;
    }

    if (g_maxDistance == 0)
        g_maxDistance = (uint16_t)size / 8;

    if (g_borderSize == 0)
        g_borderSize = g_maxDistance;

    if (_strcmpi(characterSet.c_str(), "extended") == 0)
    {
        characterSet = "ASCII";
        extendedASCII = true;
    }

    printf("\n[ TrueType font compiler v.%d.%d ]\n\n", kMajorVersion, kMinorVersion);
    printf("A tool for generating height maps for enhanced cutout magnification of\nbitmapped text.\n\n");
    printf("Font Name: \"%s\"\n", inputFile.c_str());
    printf("Font Size: %u\n", size);
    printf("Border Size: %u\n", g_borderSize);
    if (extendedASCII)
        printf("Character Set: %s + Extended ASCII\n", characterSet.c_str());
    else
        printf("Character Set: %s\n", characterSet.c_str());
    printf("Output Name: %s\n", outputName.c_str());
    printf("Threads: %u\n\n", std::thread::hardware_concurrency());

    try 
    {
        InitializeFont( inputFile.c_str(), size * 16 );

        if (strcmp(characterSet.c_str(), "ASCII") == 0)
        {
            for (wchar_t c = 32; c < 127; ++c)
            {
                if (FT_Get_Char_Index(g_FreeTypeFace, c))
                    g_glyphs[g_numGlyphs++].c = c;
            }
            if (extendedASCII)
                for (wchar_t c = 128; c < 255; ++c)
                    if (FT_Get_Char_Index(g_FreeTypeFace, c))
                        g_glyphs[g_numGlyphs++].c = c;
        }
        else
        {
            ifstream file;
            file.exceptions(ios_base::badbit);
            file.open(characterSet.c_str(), ios_base::in | ios_base::binary);

            set<wchar_t> charSet;
            wchar_t wtChar;
            file.read((char*)&wtChar, 2);

            // Check for unicode (UCS2) or ASCII
            if (wtChar == 0xFEFF)
                while (file.read((char*)&wtChar, 2))
                    charSet.insert(wtChar);
            else
            {
                // ASCII files don't have extra bytes at the beginning; rewind.
                file.seekg(0, ios_base::beg);

                char tChar = 0;
                while (file.read(&tChar, 1))
                    charSet.insert((wchar_t)tChar);
            }
            
            // Make sure the set includes all extended ascii (except white space)
            if (extendedASCII)
                for (wchar_t c = 32; c < 255; ++c)
                    if (FT_Get_Char_Index(g_FreeTypeFace, c))
                        charSet.insert(c);

            file.close();

            // Sift out the duplicates by iterating across the STL set
            for (set<wchar_t>::iterator it = charSet.begin(); it != charSet.end(); ++it)
                g_glyphs[g_numGlyphs++].c = *it;
        }

        CompileFont(outputName);

        printf("\nComplete!\n");
        //printf("Elapsed Time: %g sec\n", t.getElapsed());
    }
    catch (wofstream::failure& e)
    {
        printf("\nFile I/O Exception\n%s\n", e.what());
    }
    catch (exception& e)
    {
        printf("\nFailed: %s\n", e.what());
    }
    catch (...)
    {
        printf("\nFailed\n");
    }

    ShutdownFont();
}
