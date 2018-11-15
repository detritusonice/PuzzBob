#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include <map>

extern SDL_Color glbTextColor;

enum TEXTURE_SKINS
{
    SKIN_DARK=0,
    SKIN_LIGHT,
    SKIN_PATTERN,
    SKIN_GEOMETRIC,
    SKIN_DARK2,
    SKIN_LIGHT2,
    SKIN_PATTERN2,
    SKIN_GEOMETRIC2,
    NUM_SKIN_TYPES,
    SKIN_UNIVERSAL
};

class BaseTexture
{

enum ClipMode
    {
        CLIPMODE_SINGLE=0,
        CLIPMODE_EQUAL_SIZE,
        CLIPMODE_VARIABLE_SIZE
    };

    SDL_Texture* mTexture;
    SDL_Rect* mRcClips;
    SDL_Color mModColor;

    ClipMode mClipMode;

    int mRefCount;

    bool mUseAlpha;
    bool mUseColorMod;

    bool isText;

    int mNumClips;

    int mWidth, mHeight;

    void initTexture();


public:

    BaseTexture( const BaseTexture& b) = delete;
    BaseTexture& operator =(const BaseTexture& b) =delete;

    BaseTexture( const BaseTexture&& b) = delete;
    BaseTexture& operator =(const BaseTexture&& b) = delete; // suppress copy and move operations

    static std::vector< SDL_Texture* > renderTargets;
    static bool renderToBuffer;

    BaseTexture();
    ~BaseTexture();
    void wipe();

    bool fromFile(std::string filename, unsigned int a, bool use_transparency=false, int mask_r=255, int mask_g=0, int mask_b=255);

    bool fromText(std::string text,const SDL_Color& color=glbTextColor,TTF_Font* font=glbMessageFont, int width=0);

    bool createBlank( int width, int height, SDL_TextureAccess access );

    void setEqualClipRects(int numRows, int numColumns);

    void setVariableClipRects(const SDL_Rect* rcClips, int numClips);

    void render(const int x, const int y, const SDL_Rect* rcTarget=NULL, const int clipIndex = -1, const int alpha=255,const double angle=0., SDL_Point* center=NULL, SDL_RendererFlip flip=SDL_FLIP_NONE);
    void addInstance();
    bool destroyInstance();

    void useAlphaBlending(bool useAlpha);
    void useColorMod(bool useColorModulation);
    void setModColor(Uint8 Red, Uint8 Green, Uint8 Blue);
    void setBlendMode( SDL_BlendMode newMode);

    int getWidth() const;
    int getHeight() const;

    int getClipWidth(int clipID) const;
    int getClipHeight(int clipID) const;

    bool usesAlphaBlending() const;
    bool usesColorModulation() const;

    void setAsRenderTarget(const SDL_Color& clr);
    void setAsRenderTarget(int bgRed=0, int bgGreen=0, int bgBlue=0);
    void setAsRenderTargetNoFlush();

    void resetRenderTarget();

};

class TextureManager
{
    static const int NO_ENTRY=0;
    int lastItem;
    std::vector<BaseTexture*> tv;
    std::map<std::string,int> handlemap;
    std::map<unsigned int, unsigned int> watermap;


    bool createResizedImage( int handle, BaseTexture* tex, SDL_Rect* rc);

    void purge();
    void init();
    void buildWaterMap();

public:
    TextureManager();
    ~TextureManager();

    int insertTexture(std::string filename, int skinType, bool useAlpha=false, bool additiveBlend=false, bool use_transparency=false, int mask_r=255, int mask_g=0, int mask_b=255 );
    void removeTexture(int handle);
    BaseTexture* getTexturePtr(int handle);
    void reset();

};
#endif // TEXTURE_H
