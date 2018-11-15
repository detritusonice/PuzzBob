#include "base.h"
#include "globals.h"
#include "texture.h"
#include "basefuncs.h"
#include "antihack.h"

#include <cstdio>
#include <cstdlib>
//#include <iostream>



SDL_Color glbTextColor={0,0,0};


std::vector< SDL_Texture*> BaseTexture::renderTargets(0);
bool BaseTexture::renderToBuffer=false;

void BaseTexture::initTexture()
{
    mTexture = NULL;

    mRcClips = NULL;
    mNumClips =0;
    mClipMode=CLIPMODE_SINGLE;

    setModColor(0xff,0xff,0xff);

    mUseAlpha= true;//false;
    mUseColorMod=false;

    mWidth = mHeight =0;

    isText=false;
}
//-------------------------------------------------------
BaseTexture::BaseTexture()
{
    mRefCount=1;//0;

    initTexture();
}
//-------------------------------------------------------
BaseTexture::~BaseTexture()
{
    if (isText) wipe();

    destroyInstance();
}
//-------------------------------------------------------
//-------------------------------------------------------
void BaseTexture::wipe()
{
    mRefCount=0;
    destroyInstance();
}
//-------------------------------------------------------
bool BaseTexture::destroyInstance()//returns true for non referenced texture
{
    mRefCount--;
#ifdef CONSOLE_LOG
    //sprintf(glbLogMsg,"BaseTexture DestroyInstance() refCount=%d\n",mRefCount);
   // mySDL_Log(glbLogMsg);
#endif
    if (mRefCount>0) return false;

    if (mTexture != NULL)  SDL_DestroyTexture(mTexture);
    if (mRcClips != NULL)  delete[] mRcClips;

    initTexture();
    return true;//meaning destroyed
}
//-------------------------------------------------------
bool BaseTexture::fromFile( std::string filename, unsigned int a, bool use_transparency, int mask_r, int mask_g, int mask_b)
{
    if ( mRefCount > 1 ) return false;
    SDL_Surface* loadedImage = NULL;
    SDL_Texture* texture = NULL;

    #ifdef CONSOLE_LOG
    sprintf(glbLogMsg,"Loading Image: %s\n",filename.c_str());
    mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

    loadedImage= IMG_Load(filename.c_str());

    if (loadedImage==NULL)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Error loading Image %s: %s\n",filename.c_str(),IMG_GetError());
        mySDL_Log(glbLogMsg);
#endif
    }
    else
    {

        unsigned int hsh = imgHash(loadedImage);

        if (use_transparency)
            SDL_SetColorKey( loadedImage,SDL_TRUE, SDL_MapRGB(loadedImage->format,mask_r, mask_g, mask_b));

        if (a!=hsh)//hashes differ
#ifdef USE_WATERMARK
        {
            //SDL_Log("watermap[ %u ]   =%u; //%s\n",strHash(filename.c_str()),hsh,filename.c_str());
            //sprintf(glbLogMsg,"watermap[ %u ] =%u; //%s\n",strHash(filename.c_str()),hsh,filename.c_str());
            //mySDL_Log(glbLogMsg);
            exit(0);
        }
        else
#else
        {
            sprintf(glbLogMsg,"watermap[ %u ]   =%u; //%s\n",strHash(filename.c_str()),hsh,filename.c_str());
            mySDL_Log(glbLogMsg);
        }
#endif
        {//happens when checking and ok or when not checking at all
            texture = SDL_CreateTextureFromSurface(glbRenderer, loadedImage);
        }

        if (texture == NULL )
        {
#ifdef CONSOLE_LOG
                sprintf(glbLogMsg,"Error creating texture from file : %s\n",SDL_GetError());
                mySDL_Log(glbLogMsg);
#endif
        }
        else
        {
            destroyInstance();
            mTexture=texture;
            mWidth = loadedImage->w;
            mHeight = loadedImage->h;
            addInstance();
        }

        SDL_FreeSurface(loadedImage);


    }

    return (texture!=NULL);//throw exception
}
//-------------------------------------------------------

bool BaseTexture::fromText( std::string text, const SDL_Color& color,  TTF_Font* font, int width)
{
    if ( mRefCount > 1 ) return false;

    SDL_Surface* renderedText=NULL;
    SDL_Texture* texture = NULL;

#ifdef CONSOLE_LOG
//    sprintf(glbLogMsg,"Creating Text texture:[%s]\n",text.c_str());
//    mySDL_Log(glbLogMsg);
#endif // CONSOLE_LOG
    if (text.find_first_of('\n') == std::string::npos )
            renderedText = TTF_RenderText_Blended(font,text.c_str(), color);
    else
    {
        if (width == 0)
            width= 4*guiUnit;
        renderedText = TTF_RenderText_Blended_Wrapped(font,text.c_str(),color, width);
    }

    if (renderedText ==NULL)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Error rendering text : %s\n",TTF_GetError() );
        mySDL_Log(glbLogMsg);
#endif
    }
    else
    {
        texture = SDL_CreateTextureFromSurface(glbRenderer,renderedText);

        if (texture == NULL )
        {
#ifdef CONSOLE_LOG
            sprintf(glbLogMsg,"Error creating texture from file : %s\n",SDL_GetError());
            mySDL_Log(glbLogMsg);
#endif
        }
         else
         {
            destroyInstance();
            mWidth = renderedText->w;
            mHeight = renderedText->h;
            mTexture=texture;
            isText=true;

            addInstance();
         }
        SDL_FreeSurface(renderedText);
    }
    return (texture!=NULL);//throw exception
}
//-------------------------------------------------------
void BaseTexture::setEqualClipRects(int numRows, int numColumns)
{
    if ( mRefCount > 1 )
        return;

    if (mTexture==NULL) return;//throw exception

    mNumClips = numRows*numColumns;

    if (mRcClips!=NULL) delete[] mRcClips;

    mRcClips= new SDL_Rect[mNumClips];

    int w,h;

    if ( mWidth % numColumns )
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Image width is not a multiple of #columns\n");
        mySDL_Log(glbLogMsg);
#endif
    }
    if ( mHeight % numRows)
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"Image height is not a multiple of #rows\n");
        mySDL_Log(glbLogMsg);
#endif
    }

    w = mWidth / numColumns;
    h = mHeight /numRows;

    int clip=0;

    for (int r=0; r<numRows; r++)
    for (int c=0; c<numColumns; c++)
    {

        mRcClips[clip].w=w;
        mRcClips[clip].h=h;
        mRcClips[clip].x=c*w;
        mRcClips[clip].y=r*h;
        clip++;
    }

}
//-------------------------------------------------------
void BaseTexture::setVariableClipRects(const SDL_Rect* rcClips, int numClips)
{
    if ( mRefCount > 1 ) return; //when only one istance you may change it

    if (mRcClips !=NULL) {delete[] mRcClips; mRcClips=NULL; mNumClips=0;}

    if ((rcClips!=NULL) && (numClips>0))
    {
        for (int i=0;i<numClips;i++)
        {
            mRcClips[i]=rcClips[i];//struct copy
        }
        mNumClips= numClips;
    }
}
//-------------------------------------------------------
void BaseTexture::render(const int x, const int y, const SDL_Rect* rcTarget, const int clipIndex, const int alpha,const double angle, SDL_Point* center, SDL_RendererFlip flip)
{
    static SDL_Rect rcDest;//={x,y,mWidth,mHeight};

    rcDest.x=x;
    rcDest.y=y;
    if (rcTarget!=NULL)
    {
        rcDest.h = rcTarget->h;
        rcDest.w = rcTarget->w;
    }
    else if (clipIndex==-1)
    {
        rcDest.h = mHeight;
        rcDest.w = mWidth;
    }
    else
    if ( (clipIndex>=0) && (clipIndex<mNumClips) )
    {
        rcDest.h = mRcClips[clipIndex].h;
        rcDest.w = mRcClips[clipIndex].w;
    }


    if (mUseAlpha)
    {
        SDL_SetTextureAlphaMod(mTexture,(renderToBuffer? std::min(2*alpha,(alpha+255)/2):alpha));//HERE IF YOU HAVE ANY ALPHA PROBLEMS IN RENDERING TO BUFFERS
    }

    if ( (clipIndex>=0) && (clipIndex<mNumClips) )
    {
        if ((angle==0.)&&(flip==SDL_FLIP_NONE))
            SDL_RenderCopy(glbRenderer,mTexture,&mRcClips[clipIndex],&rcDest);
        else
            SDL_RenderCopyEx(glbRenderer,mTexture,&mRcClips[clipIndex],&rcDest,angle,center,flip);
    }
    else
        if ((angle==0.)&&(flip==SDL_FLIP_NONE))
            SDL_RenderCopy(glbRenderer,mTexture,NULL,&rcDest);
        else
            SDL_RenderCopyEx(glbRenderer,mTexture,NULL,&rcDest,angle,center,flip);

}
//-------------------------------------------------------

//-------------------------------------------------------
void BaseTexture::useAlphaBlending(bool useAlpha)
{
    mUseAlpha=useAlpha;
    mUseColorMod=false;

    if (useAlpha) SDL_SetTextureBlendMode(mTexture,SDL_BLENDMODE_BLEND);
    else SDL_SetTextureBlendMode(mTexture,SDL_BLENDMODE_NONE);
}
//-------------------------------------------------------
void  BaseTexture::useColorMod(bool useColorModulation)
{
    mUseColorMod=useColorModulation;
    mUseAlpha=false;

    if (useColorModulation) SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);//SDL_BLENDMODE_MOD); blend to display the texture modulated by the modcolor
    else SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_NONE);
}
//-------------------------------------------------------
void BaseTexture::setBlendMode(SDL_BlendMode mode)
{
    SDL_SetTextureBlendMode(mTexture,mode);
}
//-------------------------------------------------------
void BaseTexture::setModColor(Uint8 Red, Uint8 Green, Uint8 Blue)
{
    SDL_SetTextureColorMod(mTexture, Red, Green, Blue);
}
//-------------------------------------------------------
int BaseTexture::getWidth() const
{
    return mWidth;
}
//-------------------------------------------------------
int BaseTexture::getHeight() const
{
    return mHeight;
}
//-------------------------------------------------------
void BaseTexture::addInstance()
{
    mRefCount++;
}
//-------------------------------------------------------
int BaseTexture::getClipWidth(int clipID) const
{
    return mRcClips[clipID].w;
}
//-------------------------------------------------------
int BaseTexture::getClipHeight(int clipID) const
{
    return mRcClips[clipID].h;
}
//-------------------------------------------------------------------------------------------
bool BaseTexture::usesAlphaBlending() const
{
    return mUseAlpha;
}

//-------------------------------------------------------------------------------------------
bool BaseTexture::usesColorModulation() const
{
    return mUseColorMod;
}

//-----------------------------------------------------------------------------------------------
bool BaseTexture::createBlank( int width, int height, SDL_TextureAccess access )
{

    //SDL_PIXELFORMAT_RGBA8888
    //SDL_GetWindowPixelFormat(glbWindow)
    destroyInstance();

    mTexture = SDL_CreateTexture( glbRenderer,SDL_PIXELFORMAT_RGBA8888 , access, width, height );
    if( mTexture == NULL )
    {
#ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"BaseTexture::createBlank: Unable to create blank texture! SDL Error: %s\n", SDL_GetError());
        mySDL_Log(glbLogMsg);
#endif
    }
    else
    {
        mWidth = width;
        mHeight = height;
        addInstance();
    }
    return mTexture != NULL;
}
//--------------------------------------------------------
void BaseTexture::setAsRenderTarget(const SDL_Color& clr)
{
    setAsRenderTarget(clr.r,clr.g,clr.b);
}
//--------------------------------------------------------
void BaseTexture::setAsRenderTarget(int bgRed, int bgGreen, int bgBlue)
{
    if ( (!renderTargets.empty())&&(mTexture==renderTargets.back()))
        return;

    SDL_SetRenderTarget( glbRenderer, mTexture );

    renderTargets.push_back(mTexture);

//    SDL_Log("pushing render target");
//    SDL_Log("targets are:%d",renderTargets.size());

    SDL_SetRenderDrawBlendMode(glbRenderer, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor( glbRenderer, bgRed, bgGreen, bgBlue, 0x00 );

    SDL_RenderClear( glbRenderer );

    SDL_SetRenderDrawBlendMode(glbRenderer, SDL_BLENDMODE_BLEND);

    renderToBuffer=true;

}

//--------------------------------------------------------
void BaseTexture::setAsRenderTargetNoFlush()
{
    if ( (!renderTargets.empty())&&(mTexture==renderTargets.back()))
        return;

//    SDL_Log("pushing render target");
//    SDL_Log("targets are:%d",renderTargets.size());

    SDL_SetRenderTarget( glbRenderer, mTexture );
    renderTargets.push_back(mTexture);
}
//--------------------------------------------------------
void BaseTexture::resetRenderTarget()
{
    renderTargets.pop_back();

    //SDL_Log("popping render target");
    //SDL_Log("targets are:%d",renderTargets.size());

    if (renderTargets.empty())
    {
        SDL_SetRenderTarget( glbRenderer, NULL );
        renderToBuffer=false;
    }
    else
        SDL_SetRenderTarget( glbRenderer, renderTargets.back() );
}

//============================================================================================

TextureManager::TextureManager()
{
    init();
    watermap.clear();
//#ifdef USE_WATERMARK
    buildWaterMap();
//#endif // USE_WATERMARK

    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TextureManager constructor. size= %d, lastItem= %d\n",tv.size(),lastItem);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG
}
//--------------------------------------------------------
TextureManager::~TextureManager()
{
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TextureManager destructor start. size= %d, lastItem= %d\n",tv.size(),lastItem);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

    purge();
    watermap.clear();

    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TextureManager destructor end. size= %d, lastItem= %d\n",tv.size(),lastItem);
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG

}
//--------------------------------------------------------
void TextureManager::purge()
{
    handlemap.clear();

    for (int i=1 ; i < lastItem; i++)//item 0 is invalid
    {
        if (tv[i]!=NULL)
        {
            tv[i]->wipe();//force delete SDL texture and clip rectangles
            delete tv[i];//delete texture wrapper
            tv[i]=NULL;
        }
    }

    tv.clear();
    lastItem=0;
}
//--------------------------------------------------------
void TextureManager::init()
{
    tv.clear();
    handlemap.clear();

    lastItem=0;
    tv.push_back(NULL);
}
//--------------------------------------------------------
void TextureManager::reset()
{
    purge();
    init();
}
//--------------------------------------------------------
int TextureManager::insertTexture(std::string filename, int skinType, bool useAlpha, bool additiveBlend, bool use_transparency, int mask_r, int mask_g, int mask_b)
{
    #ifdef CONSOLE_LOG
        sprintf(glbLogMsg,"TextureManager InsertTexture. size= %d, lastItem= %d\n",tv.size(),lastItem);
        mySDL_Log(glbLogMsg);
        sprintf(glbLogMsg,"TextureManager InsertTexture. name= %s\n",filename.c_str());
        mySDL_Log(glbLogMsg);
    #endif // CONSOLE_LOG


    switch (skinType)
    {
        case SKIN_UNIVERSAL : filename.insert(0,"media/Universal/");break;
        case SKIN_DARK      : filename.insert(0,"media/Skins/Dark/");break;
        case SKIN_LIGHT     : filename.insert(0,"media/Skins/Light/");break;
        case SKIN_PATTERN   : filename.insert(0,"media/Skins/Pattern/");break;
        case SKIN_GEOMETRIC : filename.insert(0,"media/Skins/Geometric/");break;
        case SKIN_DARK2      : filename.insert(0,"media/Skins/Dark2/");break;
        case SKIN_LIGHT2     : filename.insert(0,"media/Skins/Light2/");break;
        case SKIN_PATTERN2   : filename.insert(0,"media/Skins/Pattern2/");break;
        case SKIN_GEOMETRIC2 : filename.insert(0,"media/Skins/Geometric2/");break;

    }

    int handle=handlemap[filename];//try filename for existing entry. if not, a new entry with 0 is created

    if ( (handle==NO_ENTRY) || ( (handle!=NO_ENTRY) && (handle<lastItem) && (tv[handle]==NULL) ) )//0 = just entered, or has been deleted
    {
        unsigned int hash1;


        hash1=strHash(filename.c_str());

        BaseTexture* tex =new BaseTexture();

        if (tex->fromFile(filename,watermap[hash1],use_transparency,mask_r,mask_g,mask_b))
        {
            if (handle==NO_ENTRY)
            {
                tv.push_back(tex);
                handle = ++lastItem;
                handlemap[filename]=handle;
            }
            else
                tv[handle]=tex;
        }
        else
            { delete tex; return -1;}//file could not be loaded
    }
    else tv[handle]->addInstance();

    tv[handle]->useAlphaBlending(useAlpha);

    if (additiveBlend)       tv[handle]->setBlendMode(SDL_BLENDMODE_ADD);

    return handle;
}
//-------------------------------------------------------------
bool TextureManager::createResizedImage( int handle, BaseTexture* tex, SDL_Rect* rc)
{
    if (tex->createBlank( rc->w,rc->h,SDL_TEXTUREACCESS_TARGET) )
    {
        tex->useAlphaBlending(true);
        tex->setAsRenderTarget();

        tv[handle]->render(0,0,rc,-1,255);

        tex->resetRenderTarget();

        return true;
    }
    return false;
}

//-------------------------------------------------------------

void TextureManager::removeTexture(int handle)
{
    if ((handle>0) && (handle <= lastItem ))
        if (tv[handle]!=NULL)
        {
            if (tv[handle]->destroyInstance()==true)///means no references, and texture is invalid
            {
                delete tv[handle];
                tv[handle]=NULL;
            }

        }
}

BaseTexture* TextureManager::getTexturePtr(int handle)
{
#ifdef DEBUGMODE
    if ((handle<0) || (handle > lastItem )) return NULL;
#endif
    return tv[handle];

}

void TextureManager::buildWaterMap()
{



    watermap[ 3522959083 ]  =4180816700; //media/Universal/logo.png
//buttons---------
    watermap[ 3103076357 ]   =3748903042; //media/Universal/buttons/buttons_purple.png
    watermap[ 3535038998 ]   =58168570; //media/Universal/buttons/buttons_green.png
    watermap[ 3616030116 ]   =4084158680; //media/Universal/buttons/buttons_cyan.png
    watermap[ 3215116323 ]   =2822045629; //media/Universal/buttons/buttons_orange.png
    watermap[ 2297020472 ]   =2805345108; //media/Universal/buttons/buttons_yellow.png
    watermap[ 1730701734 ]   =2727736243; //media/Universal/buttons/buttons_blue.png

    watermap[ 1207462317 ]   =2367279867; //media/Universal/buttons/buttons_glow.png
    watermap[ 33847851 ]   =1490381622; //media/Universal/buttons/buttons_glass.png


    watermap[ 613959054 ]   =3150832165; //media/Universal/volumeBarBG.png
    watermap[ 616056206 ]   =882261510; //media/Universal/volumeBarFG.png
    watermap[ 41789260 ]   =3643719177; //media/Universal/volumeBarActive.png
    watermap[ 11080568 ]   =1856173538; //media/Universal/volumeBarHandle.png

//---skin thumbnails
    watermap[ 641937933 ]   =2156248354; //media/Universal/thumbs/skin_dark.png
    watermap[ 2852965270 ]   =4277761249; //media/Universal/thumbs/skin_light.png
    watermap[ 713299066 ]   =2935326827; //media/Universal/thumbs/skin_pattern.png
    watermap[ 1608483244 ]   =1679429778; //media/Universal/thumbs/skin_geometric.png
    watermap[ 3256131458 ]   =2350451928; //media/Universal/thumbs/skin_dark2.png
    watermap[ 265468030 ]   =2715095959; //media/Universal/thumbs/skin_light2.png
    watermap[ 4187159569 ]   =2661353606; //media/Universal/thumbs/skin_pattern2.png
    watermap[ 313020891 ]   =500088860; //media/Universal/thumbs/skin_geometric2.png



    watermap[ 3226720405 ]  =739281496; //media/Universal/message_background.png
    watermap[ 651167106 ]   =1702944156; //media/Universal/warning_block.png
    watermap[ 3784699635 ]  =272293410; //media/Universal/page_background.png
//---balls---------

    watermap[ 2125557661 ]   =3559509092; //media/Universal/balls/ball_bomb.png
    watermap[ 4134593494 ]   =973134555; //media/Universal/balls/ball_painter.png
    watermap[ 4134822662 ]   =4116319008; //media/Universal/balls/ball_shifter.png
    watermap[ 3916596416 ]   =2179218741; //media/Universal/balls/ball_leveler.png
    watermap[ 3883538003 ]   =3622228295; //media/Universal/balls/ball_chrome.png
    watermap[ 1956896047 ]   =2321627581; //media/Universal/balls/timestop.png

    watermap[ 2586898095 ]   =4021104526; //media/Universal/balls/ball_block.png
    watermap[ 1650023312 ]   =2692254362; //media/Universal/balls/ball_reflector.png

//-----machine and border-------
    watermap[ 1871166932 ]  =243623444; //media/Universal/machine.png
    watermap[ 3181996 ]     =1800528143; //media/Universal/cannon.png
    watermap[ 2297659681 ]  =3911999209; //media/Universal/well_bottom.png
    watermap[ 1256690862 ]  =605130258; //media/Universal/well_top.png
    watermap[ 4078101989 ]  =61922850; //media/Universal/well_left.png
    watermap[ 1218327413 ]  =3168798297; //media/Universal/well_right.png
    watermap[ 1285434847 ]  =1426524806; //media/Universal/well_corner_left.png
    watermap[ 1438774545 ]  =859506310; //media/Universal/well_corner_right.png
    watermap[ 1129890655 ]  =3316306366; //media/Universal/well_pistons.png
    watermap[ 1423435219 ]  =1843289358; //media/Universal/well_top_plate.png
    watermap[ 878403118 ]   =759445416; //media/Universal/well_endline.png
    watermap[ 602347510 ]   =2321627581; //media/Universal/timestop.png

    watermap[ 2020948748 ] =743220304; //media/Universal/well_piston_background.png

    watermap[ 3503866126 ] =2602806345; //media/Universal/noLevel.png

//effects-------------

    watermap[ 2637316304 ]  =4201026933; //media/Universal/effects/glint.png

    watermap[ 3687643804 ]  =1861893973; //media/Universal/effects/nova_white.png
    watermap[ 3441268324 ]  =4037125254; //media/Universal/effects/nova_red.png
    watermap[ 2255033589 ]  =1025492168; //media/Universal/effects/nova_blue.png
    watermap[ 3886139477 ]  =3174364102; //media/Universal/effects/nova_purple.png
    watermap[ 779410068 ]   =2003352452; //media/Universal/effects/nova_black.png
    watermap[ 3774230643 ]  =99435210;   //media/Universal/effects/nova_orange.png
    watermap[ 3619502184 ]  =3551462856; //media/Universal/effects/nova_yellow.png
    watermap[ 2068105896 ]  =1418144467; //media/Universal/effects/nova_green.png
    watermap[ 4204803157 ]  =1910318217; //media/Universal/effects/nova_chrome.png
    watermap[ 2127880872 ]  =4116264488; //media/Universal/effects/nova_brown.png
    watermap[ 452254356 ] =739281496 ;   //media/Universal/effects/nova_flash.png

    watermap[ 303226233 ] =2905209307 ;  //media/Universal/effects/spark_yellow.png
    watermap[ 613345571 ] =3100030307 ;  //media/Universal/effects/spark_green.png
    watermap[ 571354436 ] =1925156264 ;  //media/Universal/effects/spark_purple.png
    watermap[ 305005130 ] =4091739469 ;  //media/Universal/effects/spark_blue.png
    watermap[ 1908341023 ] =2343312712 ; //media/Universal/effects/spark_black.png
    watermap[ 1127278924 ] =2357867554 ; //media/Universal/effects/spark_red.png
    watermap[ 626644322 ] =1734235476 ; //media/Universal/effects/spark_orange.png
    watermap[ 555663651 ] =362461394 ; //media/Universal/effects/spark_brown.png
    watermap[ 1058001220 ] =1494482752 ; //media/Universal/effects/spark_chrome.png
    watermap[ 2215041303 ]  =3425395736; //media/Universal/effects/spark_white.png

    watermap[ 2157871622 ]  =3684908649; //media/Universal/effects/score_exploded.png
    watermap[ 1031097392 ]  =2256309027; //media/Universal/effects/score_dropped.png
    watermap[ 645705302 ]   =597870932; //media/Universal/effects/score_matched.png

    watermap[ 4153677918 ] =1692215339; //media/Universal/badges/truerandom.png
    watermap[ 969128880 ] =2043629397; //media/Universal/badges/chaos.png
    watermap[ 1967419372 ] =706579294; //media/Universal/badges/match4.png
    watermap[ 1698983916 ] =2359697240; //media/Universal/badges/match5.png
    watermap[ 1430548460 ] =1620438731; //media/Universal/badges/match6.png
    watermap[ 43063938 ] =1105318446; //media/Universal/badges/width10.png
    watermap[ 579934850 ] =2605974436; //media/Universal/badges/width12.png
    watermap[ 1116805762 ] =1864184479; //media/Universal/badges/width14.png
    watermap[ 1653676674 ] =3844035959; //media/Universal/badges/width16.png
    watermap[ 3911486356 ] =4248362633; //media/Universal/badges/timelimit.png
    watermap[ 3378252564 ] =726085221; //media/Universal/badges/shotlimit.png
    watermap[ 733970834 ] =19247496; //media/Universal/badges/scorelimit.png
    watermap[ 276313042 ] =3258973721; //media/Universal/badges/editor.png

    watermap[ 1936037560 ]   =652112190; //media/Universal/star_cyan.png
    watermap[ 1558584515 ]   =3022692337; //media/Universal/star_green.png
    watermap[ 3794725123 ]   =1466132201; //media/Universal/star_yellow.png

//-----------------skins----------------------------------------------------------------------------

    watermap[ 4091024960 ]   =3810584759; //media/Skins/Dark/ball_green.png
    watermap[ 3218130404 ]   =1254068992; //media/Skins/Dark/ball_blue.png
    watermap[ 2481292859 ]   =4250406251; //media/Skins/Dark/ball_purple.png
    watermap[ 3200234519 ]   =375376208; //media/Skins/Dark/ball_red.png
    watermap[ 2494725661 ]   =1063618390; //media/Skins/Dark/ball_orange.png
    watermap[ 2751034886 ]   =1624852079; //media/Skins/Dark/ball_yellow.png

    watermap[ 1397761652 ]   =485882281; //media/Skins/Dark/ball_white.png
    watermap[ 2800223868 ]   =2228664433; //media/Skins/Dark/ball_black.png

    watermap[ 2795263931 ] =1357189066; //media/Skins/Dark/background.png
    watermap[ 462331782 ] =1500242623; //media/Skins/Dark/well_background.png
    watermap[ 3560051019 ]   =1448995385; //media/Skins/Dark/levelSelector.png

    watermap[ 687965409 ]   =657420229; //media/Skins/Geometric/ball_white.png
    watermap[ 3703153897 ]  =653021930; //media/Skins/Geometric/ball_black.png
    watermap[ 887774545 ]   =851903269; //media/Skins/Geometric/ball_red.png
    watermap[ 2307503317 ]  =3704344127; //media/Skins/Geometric/ball_green.png
    watermap[ 479588112 ]   =1765164000; //media/Skins/Geometric/ball_blue.png
    watermap[ 3646467818 ]  =2110173784; //media/Skins/Geometric/ball_purple.png
    watermap[ 3913572055 ]  =4047129835; //media/Skins/Geometric/ball_yellow.png
    watermap[ 3725547212 ]  =7326793; //media/Skins/Geometric/ball_orange.png

    watermap[ 3704485166 ] =605794043; //media/Skins/Geometric/background.png
    watermap[ 254102795 ] =1270977071; //media/Skins/Geometric/well_background.png
    watermap[ 2651210724 ]   =2148444863; //media/Skins/Geometric/levelSelector.png


    watermap[ 652311929 ]   =1055007869; //media/Skins/Light/ball_white.png
    watermap[ 3545849201 ]  =686231541; //media/Skins/Light/ball_black.png
    watermap[ 3532284626 ]  =2662066228; //media/Skins/Light/ball_red.png
    watermap[ 2255048013 ]  =2775708354; //media/Skins/Light/ball_green.png
    watermap[ 3714506511 ]  =220419212; //media/Skins/Light/ball_blue.png
    watermap[ 358571670 ]   =807211431; //media/Skins/Light/ball_purple.png
    watermap[ 625192619 ]   =323013004; //media/Skins/Light/ball_yellow.png
    watermap[ 302556848 ]   =2184386585; //media/Skins/Light/ball_orange.png

    watermap[ 3542944950 ] =2491322300; //media/Skins/Light/background.png
    watermap[ 3286577676 ] =3137722656; //media/Skins/Light/well_background.png
    watermap[ 1645834469 ]   =2793234962; //media/Skins/Light/levelSelector.png

    watermap[ 2164128728 ]  =1859327454; //media/Skins/Pattern/ball_white.png
    watermap[ 1967512528 ]  =3335563278; //media/Skins/Pattern/ball_black.png
    watermap[ 3356293792 ]  =1848492914; //media/Skins/Pattern/ball_red.png
    watermap[ 544590828 ]   =3922543709; //media/Skins/Pattern/ball_green.png
    watermap[ 3832257603 ]  =3818302800; //media/Skins/Pattern/ball_blue.png
    watermap[ 1166823637 ]  =3697166577; //media/Skins/Pattern/ball_purple.png
    watermap[ 1972412648 ]  =2501803620; //media/Skins/Pattern/ball_yellow.png
    watermap[ 1121691891 ]  =4007237722; //media/Skins/Pattern/ball_orange.png

    watermap[ 1966180887 ] =4015247034; //media/Skins/Pattern/background.png
    watermap[ 4142400232 ] =1143635470; //media/Skins/Pattern/well_background.png
    watermap[ 2710152401 ] =3447088875; //media/Skins/Pattern/levelSelector.png

    watermap[ 191514575 ]   =1422753132; //media/Skins/Dark2/ball_green.png
    watermap[ 3681802773 ]   =469324184; //media/Skins/Dark2/ball_blue.png
    watermap[ 1410981651 ]   =707009137; //media/Skins/Dark2/ball_purple.png
    watermap[ 4169160414 ]   =2348752867; //media/Skins/Dark2/ball_red.png
    watermap[ 1397630773 ]   =214619828; //media/Skins/Dark2/ball_orange.png
    watermap[ 1678061358 ]   =543614103; //media/Skins/Dark2/ball_yellow.png
    watermap[ 2883745787 ]   =3114111036; //media/Skins/Dark2/ball_white.png
    watermap[ 1582979059 ]   =4283729432; //media/Skins/Dark2/ball_black.png

    watermap[ 3262652596 ]   =3759215908; //media/Skins/Dark2/levelSelector.png
	watermap[ 1580074548 ]   =1239713500; //media/Skins/Dark2/background.png
	watermap[ 2549176852 ]   =3395107866; //media/Skins/Dark2/well_background.png

    watermap[ 2379259746 ] =1095040512; //media/Skins/Light2/ball_green.png
    watermap[ 2931982616 ] =496142753; //media/Skins/Light2/ball_blue.png
    watermap[ 47874687 ] =2580835855; //media/Skins/Light2/ball_purple.png
    watermap[ 2587249717 ] =719538468; //media/Skins/Light2/ball_red.png
    watermap[ 93157977 ] =618477202; //media/Skins/Light2/ball_orange.png
    watermap[ 851866178 ] =2916562518; //media/Skins/Light2/ball_yellow.png
    watermap[ 760786774 ] =3702802317; //media/Skins/Light2/ball_white.png
    watermap[ 3638595422 ] =1358729554; //media/Skins/Light2/ball_black.png

    watermap[ 1386049474 ] =2482300562; //media/Skins/Light2/well_background.png
    watermap[ 354628868 ] =3627214340; //media/Skins/Light2/levelSelector.png
    watermap[ 3633634969 ] =1942725797; //media/Skins/Light2/background.png

    watermap[ 3707998497 ] =3938737196; //media/Skins/Pattern2/ball_green.png
    watermap[ 148768697 ] =3560074435; //media/Skins/Pattern2/ball_blue.png
    watermap[ 594734072 ] =2819791312; //media/Skins/Pattern2/ball_purple.png
    watermap[ 2738556793 ] =3848355911; //media/Skins/Pattern2/ball_red.png
    watermap[ 606409694 ] =1064670528; //media/Skins/Pattern2/ball_orange.png
    watermap[ 325983173 ] =2226650338; //media/Skins/Pattern2/ball_yellow.png
    watermap[ 2106302741 ] =4229337762; //media/Skins/Pattern2/ball_white.png
    watermap[ 2285076765 ] =749509784; //media/Skins/Pattern2/ball_black.png

    watermap[ 2286407898 ] =4032994121; //media/Skins/Pattern2/background.png
    watermap[ 545778010 ] =1180890288; //media/Skins/Pattern2/well_background.png
    watermap[ 256048494 ] =3728476919; //media/Skins/Pattern2/levelSelector.png


    watermap[ 1104637726 ] =3817073080; //media/Skins/Geometric2/ball_green.png
    watermap[ 2703390848 ] =686809786; //media/Skins/Geometric2/ball_blue.png
    watermap[ 1018986887 ] =990278235; //media/Skins/Geometric2/ball_purple.png
    watermap[ 1539669034 ] =4062225290; //media/Skins/Geometric2/ball_red.png
    watermap[ 1005287841 ] =3519256157; //media/Skins/Geometric2/ball_orange.png
    watermap[ 211841466 ] =2035281212; //media/Skins/Geometric2/ball_yellow.png
    watermap[ 3781123882 ] =1404253129; //media/Skins/Geometric2/ball_white.png
    watermap[ 350715682 ] =3344541378; //media/Skins/Geometric2/ball_black.png

    watermap[ 3514503165 ] =1171681624; //media/Skins/Geometric2/well_background.png
    watermap[ 2514303645 ] =3447786909; //media/Skins/Geometric2/levelSelector.png
    watermap[ 345190117 ] =433395097; //media/Skins/Geometric2/background.png

}



