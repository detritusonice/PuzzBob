#include "base.h"
#include "basefuncs.h"
#include "antihack.h"


bool isLegitimateCopy=
#ifndef DEMO_GAME

    #ifdef PLATFORM_PC
                            true;
    #else
                            false;
    #endif

#else//DEMO_GAME
                            false;
#endif

bool isOriginal=false;
bool isLicensed=false;

bool integrityCheckComplete=false;
bool licenceCheckComplete=false;


void CRLS32(unsigned int& n,int pos)
{
    pos = pos&31;//no more than 31 right shifts;

    n= n>>(32-pos)| n<<pos;
}

void CRLS64(unsigned long long &n,int pos)
{
    pos = pos&63;//no more than 31 right shifts;

    n= n>>(64-pos)| n<<pos;

    //return n;
}

unsigned int strHash(const char* s)
{
    unsigned int hs=0;
    int i=0;
    while (s[i]!=0)
    {
        CRLS32(hs,23);
        hs = hs ^ 13757;
        hs^=(unsigned int)s[i];
        i++;

/*        cout <<" HS=";
        for (int j=31;j>=0;j--)
            cout << ((hs & (unsigned int)(1<<j))>>j);
        cout <<endl;
*/
    }

    return hs;
}

unsigned int imgHash(SDL_Surface* s)
{
    unsigned int hs=0;

    int offset=0;

    int inc=31;

    int sz= (s->h) * (s->w);
    int bpp=s->format->BytesPerPixel;

    Uint8* pel;

    while (offset<sz)
    {
        pel = (Uint8*)( (Uint8*)s->pixels + offset*sizeof(Uint8)*bpp);


        CRLS32(hs,23);
        hs = hs ^ 13757;
        hs^=  (unsigned int) *((Uint32*)pel);

        offset+=inc;
    }
    return hs;
}

char offsets[200]={
                    50,	55,	27,	37,	73,	89,	56,	85,	65,	52,
                    28,	56,	26,	20,	67,	44,	53,	92,	42,	28,
                    57,	37,	75,	85,	16,	26,	21,	73,	68,	79,
                    67,	49,	35,	27,	19,	65,	41,	17,	12,	82,
                    35,	77,	95,	88,	39,	56,	96,	67,	16,	85,
                    46,	48,	92,	56,	19,	18,	16,	65,	65,	58,
                    73,	86,	34,	63,	19,	26,	23,	63,	28,	67,
                    85,	68,	42,	42,	93,	12,	33,	17,	76,	56,
                    35,	14,	50,	30,	34,	72,	92,	93,	94,	24,
                    40,	26,	72,	56,	21,	11,	26,	34,	89,	41,
                    55,	11,	21,	79,	62,	21,	85,	18,	76,	73,
                    88,	90,	17,	54,	89,	51,	94,	92,	53,	82,
                    67,	10,	35,	26,	79,	49,	43,	73,	48,	100,
                    65,	29,	31,	39,	64,	100,57,	60,	29,	42,
                    86,	85,	10,	68,	27,	83,	31,	10,	64,	88,
                    69,	95,	69,	26,	33,	40,	61,	10,	77,	82,
                    26,	75,	74,	99,	36,	44,	17,	76,	89,	37,
                    14,	63,	81,	64,	36,	24,	82,	32,	57,	33,
                    46,	45,	71,	40,	77,	85,	41,	37,	85,	42,
                    85,	70,	19,	34,	41,	40,	89,	36,	64,	41
};

char* scramble(char* str)
{
    int i=0;
    int spread= 127-32;
    while(str[i]!=0)
    {
        str[i]=32+(str[i]-32+offsets[i])%spread;
        i++;
    }
    str[i]=0;
    return str;
}

char* unscramble(char* str)
{
    int i=0;
    int spread= 127-32;
    while(str[i]!=0)
    {
        str[i]= ((str[i]-32)+ (spread-offsets[i]) )%spread +32 ;
        i++;
    }
    str[i]=0;

    return str;
}



#ifdef HACK_PROTECTION
#include <cstdio>


//====================================================================
#ifdef PLATFORM_PC
    #include <fstream>
#else
    #include <iostream>//for std namespace
#endif
unsigned long long ahGetDexSum( unsigned char* buffer, int length)
{
    unsigned long long hs=0;

    for (int i=0;i<length;i++)
    {
        CRLS64(hs,23);
        hs ^= 13757;
        hs^=(unsigned long long)(buffer[i]);
    }
    return hs;
}

unsigned long long ahGetDexSum(const char* filename)
{
    unsigned long long hs=0;

    //SDL_Log("opening %s",filename);

    FILE* fp = NULL;
    fp=fopen(filename,"rb");
    if (fp==NULL)
    {
     //   SDL_Log("failed to open %s",filename);
        return 0;
    }

    unsigned char buffer[200000];
    int sz=sizeof(buffer[0]);

    int nread=fread(buffer,sz,200000,fp);
    fclose (fp);

   // SDL_Log("read %d units of size %d",nread,sz);

    hs=ahGetDexSum(buffer,nread);

//    SDL_Log("Done");

#ifdef PLATFORM_PC
    std::fstream out;
    out.open("DEXCRC.TXT",std::ios_base::out);
    out << hs<<std::endl;
    out.close();
#endif


    return hs;
}

//====================================================================

#ifdef PLATFORM_ANDROID

const unsigned long long SUM_VALUE = 10955049911124013215ull;//this value is generated by puzzbob pc after running ant release (only java code is checked)
#include <jni.h>
#include <miniz.h>

JavaVM* javaVM = NULL;
jobject activityObj;

static jmethodID licenceMID;


void ahGetParams()
{

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject act = (jobject)SDL_AndroidGetActivity();

    env->GetJavaVM(&javaVM);

    activityObj = (jobject) env->NewGlobalRef(act);

    jclass activityClass = env->GetObjectClass(activityObj);

    char funcname[]="4M /YM>j26/:((";//original strings: "audioSetParams","(IZZI)I"
    char sig[]="Z!u 3#\"";

    licenceMID = env->GetStaticMethodID(activityClass, unscramble(funcname), unscramble(sig));

    env->DeleteLocalRef(act);
}

void ahCheckLicence()
{

    if (!licenceCheckComplete)
    {
        JNIEnv* env=(JNIEnv*)SDL_AndroidGetJNIEnv();
        jclass cls= env->GetObjectClass(activityObj);


        int response = env->CallStaticIntMethod(cls,licenceMID,44100,JNI_TRUE,JNI_TRUE,2048);


        bool performing= response & (1<<13);
        bool checkComplete= response & (1<<12);
        bool licensed= response & (1<<11);
        bool errorHappened= response & (1<<10);
        int errorCode = response &((1<<10)-1);
//        SDL_Log("Response %d",response);

  //      SDL_Log("checking=%d checkDone=%d licensed=%d foundError=%d errorCode= %d",performing,checkComplete, licensed, errorHappened, errorCode);

        if (checkComplete)
        {
            isLicensed = licensed;
            licenceCheckComplete = true;

            if (integrityCheckComplete)
                isLegitimateCopy = isOriginal && isLicensed;
        }
    }
}

void* unzipDex(const char* apkname,unsigned int *length)
{
    *length=0;
    char filename[]="6D|9]_L$F:5"; //"classes.dex"

    void* p= mz_zip_extract_archive_file_to_heap( apkname, unscramble(filename), length,
                                                 MZ_ZIP_FLAG_CASE_SENSITIVE| MZ_ZIP_FLAG_IGNORE_PATH );//use scrambled string
#ifdef CONSOLE_LOG
    //SDL_Log("Step=%d",*length);
#endif

    return p;
}

bool ahCheckIntegrity()
{
    char funcname[]=":=0uK]DWI:_H~y4.Je";      //"getPackageCodePath"

    char sig[]="Z`g0Kp:%N6+@IgX??k2W";         // "()Ljava/lang/String;"

    JNIEnv* env=(JNIEnv*)SDL_AndroidGetJNIEnv();//0;

    jclass activityClass = env->GetObjectClass(activityObj);



    jmethodID methodID = env->GetMethodID(activityClass, unscramble(funcname),unscramble(sig));//use scrambled strings

    jstring path = (jstring)env->CallObjectMethod(activityObj, methodID);

    env->DeleteLocalRef(activityClass);

    jboolean isCopy;

    std::string apkname(env->GetStringUTFChars(path, &isCopy));


#ifdef CONSOLE_LOG
    //SDL_Log("Looked up package code path: %s", apkname.c_str());
#endif

    unsigned int length=0;
    void* p=unzipDex(apkname.c_str(), &length);

    if (p!=NULL)
    {
        isOriginal=(ahGetDexSum((unsigned char*)p,length) == SUM_VALUE);
    }
    else
    {
        isOriginal=false;
#ifdef CONSOLE_LOG
//        SDL_Log("EXTRACTION FAILED");
#endif
    }

#ifdef CONSOLE_LOG
//    if (isOriginal) SDL_Log("Original");
//    else SDL_Log("Tinkered");
#endif

    integrityCheckComplete=true;

    if (!isOriginal)
    {
        isLegitimateCopy=false;
        licenceCheckComplete=true;
        isLicensed=false;
    }
    else
        if (licenceCheckComplete)
            isLegitimateCopy = isLicensed;

    return isOriginal;
}

#endif // PLATFORM_ANDROID






#endif //HACK_PROTECTION
//-----------------------------------------------------------------------------------