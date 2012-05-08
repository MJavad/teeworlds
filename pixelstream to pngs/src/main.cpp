#include <zlib/zlib.h>
#include <pnglite/pnglite.h>
#include <base/system.h>
#include <base/math.h>

#define THREADWAITDELAY 1
#define THREADCOUNT 8

#define FPS 30.0f
#define FRAMETIME 1000.f / FPS
#define FRAMETIMEHALF FRAMETIME / 2.f

struct CThreadData
{
    int m_Size;
    int m_W;
    int m_H;

    volatile int m_CounterIn;
    volatile int m_aIndexIn[10];
    unsigned char *m_pDataIn[10];

    volatile int m_CounterOut;
    volatile int m_aIndexOut[10];
    unsigned char *m_pDataOut[10];
};

struct CBlendThreadData
{
    unsigned char *m_pIn1;
    unsigned char *m_pIn2;
    unsigned char *m_pOut;
    volatile int m_Size;
    volatile int m_Start;
    volatile int m_Steps;

    volatile float m_Mix;

    volatile bool m_Finished;
};

static void BlendThread(void *pUser)
{
    CBlendThreadData *pData = (CBlendThreadData *)pUser;
    for (int i = pData->m_Start; i < pData->m_Size; i = i + pData->m_Steps)
    {
        pData->m_pOut[i] = mix(pData->m_pIn1[i], pData->m_pIn2[i], pData->m_Mix);
    }
    pData->m_Finished = true;
}

static void RotThread(void *pUser)
{
    CThreadData *pThreadData = (CThreadData *)pUser;
    unsigned char *pData = new unsigned char[pThreadData->m_Size];
    unsigned char *pTempRow = new unsigned char[pThreadData->m_W*3];

    while(1)
    {
        while(1)
        {
            int MinIndex = 0;
            int MinI= 0;
            for (int i = 0; i < 10; i++)
            {
                if (pThreadData->m_pDataIn[i] != 0 && (MinIndex == 0 || MinIndex > pThreadData->m_aIndexIn[i]))
                {
                    MinIndex = pThreadData->m_aIndexIn[i];
                    MinI = i;
                }
            }
            if (MinIndex != 0)
            {
                mem_copy(pData, pThreadData->m_pDataIn[MinI], pThreadData->m_Size);
                delete []pThreadData->m_pDataIn[MinI];
                pThreadData->m_pDataIn[MinI] = 0;
                break;
            }
            if (THREADWAITDELAY)
                thread_sleep(THREADWAITDELAY);
        }
        for(int y = 0; y < pThreadData->m_H/2; y++)
        {
            mem_copy(pTempRow, pData+y*pThreadData->m_W*3, pThreadData->m_W*3);
            mem_copy(pData+y*pThreadData->m_W*3, pData+(pThreadData->m_H-y-1)*pThreadData->m_W*3, pThreadData->m_W*3);
            mem_copy(pData+(pThreadData->m_H-y-1)*pThreadData->m_W*3, pTempRow,pThreadData->m_W*3);
        }

        int i = 0;
        while(1)
        {
            if (pThreadData->m_pDataOut[i] == 0)
            {
                pThreadData->m_CounterOut++;
                pThreadData->m_aIndexOut[i] = pThreadData->m_CounterOut;
                unsigned char *p = new unsigned char[pThreadData->m_Size];
                mem_copy(p, pData, pThreadData->m_Size);
                pThreadData->m_pDataOut[i] = p;
                break;
            }
            i++;
            if (i == 10)
                i = 0;
            if (THREADWAITDELAY)
                thread_sleep(THREADWAITDELAY);
        }

    }
}

int main(void)
{
    dbg_logger_stdout();
    char aUserdir[1024] = {0};
    char aPixelFile[1024] = {0};
    int PngCounter = 0;

    fs_storage_path("Teeworlds", aUserdir, sizeof(aUserdir));
    str_format(aPixelFile, sizeof(aPixelFile), "%s/tmp/pixelstream/video.stream", aUserdir);
    IOHANDLE PixelStream = io_open(aPixelFile, IOFLAG_READ);

    long long t = 0;
    long long tOld = 0;
    long long tOldAdj = 0;
    unsigned char *pData = 0;
    unsigned char *pDataOld = 0;
    unsigned char *pDataMixed = 0;
    unsigned char *pTempRow = 0;

    CThreadData *pThreadData = new CThreadData();
    mem_zero(pThreadData, sizeof(CThreadData));

    int64 StartTime = time_get();
    int64 SpeedTime = time_get();
    while(PixelStream)
    {
        int Size = 0;
        int W = 0;
        int H = 0;
        int len = io_read(PixelStream, &t, sizeof(t));
        io_read(PixelStream, &Size, sizeof(Size));
        io_read(PixelStream, &W, sizeof(W));
        io_read(PixelStream, &H, sizeof(H));

        if (len == 0)
            break;

        if (!pData)
            pData = new unsigned char[Size];
        if (!pDataOld)
            pDataOld = new unsigned char[Size];
        if (!pDataMixed)
            pDataMixed = new unsigned char[Size];
        if (!pTempRow)
            pTempRow = new unsigned char[W * 3];

        io_read(PixelStream, pData, Size);

        if (pThreadData->m_Size == 0)
        {
            pThreadData->m_Size = Size;
            pThreadData->m_H = H;
            pThreadData->m_W = W;
            thread_detach(thread_create(RotThread, pThreadData));
        }

        int i = 0;
        while(1)
        {
            if (pThreadData->m_pDataIn[i] == 0)
            {
                pThreadData->m_CounterIn++;
                pThreadData->m_aIndexIn[i] = pThreadData->m_CounterIn;
                unsigned char *p = new unsigned char[Size];
                mem_copy(p, pData, Size);
                pThreadData->m_pDataIn[i] = p;
            }
            i++;
            if (i == 10)
                break;
            if (THREADWAITDELAY)
                thread_sleep(THREADWAITDELAY);
        }
        while(1)
        {
            int MinIndex = 0;
            int MinI= 0;
            for (int i = 0; i < 10; i++)
            {
                if (pThreadData->m_pDataOut[i] != 0 && (MinIndex == 0 || MinIndex > pThreadData->m_aIndexOut[i]))
                {
                    MinIndex = pThreadData->m_aIndexOut[i];
                    MinI = i;
                }
            }
            if (MinIndex != 0)
            {
                mem_copy(pData, pThreadData->m_pDataOut[MinI], Size);
                delete []pThreadData->m_pDataOut[MinI];
                pThreadData->m_pDataOut[MinI] = 0;
                break;
            }
            if (THREADWAITDELAY)
                thread_sleep(THREADWAITDELAY);
        }

        float tAdj = 0.0f;
        if (tOld && tOld + tAdj + FRAMETIME < t)
        {
            while(tOld && tOld + tAdj + FRAMETIME < t)
            {
                PngCounter++;
                char aBuf[1024];
                if (PngCounter < 10)
                    str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png0000%i.png", aUserdir, PngCounter);
                if (PngCounter < 100)
                    str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png000%i.png", aUserdir, PngCounter);
                if (PngCounter < 1000)
                    str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png00%i.png", aUserdir, PngCounter);
                if (PngCounter < 10000)
                    str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png0%i.png", aUserdir, PngCounter);
                if (PngCounter < 100000)
                    str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png%i.png", aUserdir, PngCounter);

                #if THREADCOUNT > 0
                CBlendThreadData *apBlendData[THREADCOUNT];
                for (int i = 0; i < THREADCOUNT; i++)
                {
                    apBlendData[i] = new CBlendThreadData();
                    mem_zero(apBlendData[i], sizeof(CBlendThreadData));
                    apBlendData[i]->m_pIn1 = pData;
                    apBlendData[i]->m_pIn2 = pDataOld;
                    apBlendData[i]->m_pOut = pDataMixed;
                    apBlendData[i]->m_Size = Size;
                    apBlendData[i]->m_Steps = THREADCOUNT;
                    apBlendData[i]->m_Start = i;
                    apBlendData[i]->m_Mix = 1.0f - (tAdj + FRAMETIMEHALF) / ((float)(t - tOld));
                    thread_detach(thread_create(BlendThread, apBlendData[i]));
                }
                while (1)
                {
                    bool Done = true;
                    for (int i = 0; i < THREADCOUNT; i++)
                    {
                        if (apBlendData[i]->m_Finished == false)
                        {
                            Done = false;
                            break;
                        }
                    }
                    if (THREADWAITDELAY)
                        thread_sleep(THREADWAITDELAY);
                    if (Done)
                        break;
                }
                #else
                for (int i = 0; i < Size; i++)
                {
                    pDataMixed[i] = mix(pData[i], pDataOld[i], 1.0f - (tAdj + FRAMETIMEHALF) / ((float)(t - tOld)));
                }
                #endif

                png_t Png;
                png_init(0,0);
                png_open_file_write(&Png, aBuf); // ignore_convention
                png_set_data(&Png, W, H, 8, PNG_TRUECOLOR, (unsigned char *)pDataMixed); // ignore_convention
                png_close_file(&Png); // ignore_convention

                if (time_get() - SpeedTime > time_freq())
                {
                    dbg_msg("Frame", "%i (Avg: %.2f s)", PngCounter, ((float)(time_get() - StartTime) / (float)time_freq()) / (float)PngCounter);
                    dbg_msg("FPS", "%.f", (float)PngCounter / ((float)(time_get() - StartTime) / (float)time_freq()));
                    SpeedTime = time_get();
                }

                tAdj = tAdj + FRAMETIME;
            }
        }
        else
        {
            PngCounter++;
            char aBuf[1024];
            if (PngCounter < 10)
                str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png0000%i.png", aUserdir, PngCounter);
            if (PngCounter < 100)
                str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png000%i.png", aUserdir, PngCounter);
            if (PngCounter < 1000)
                str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png00%i.png", aUserdir, PngCounter);
            if (PngCounter < 10000)
                str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png0%i.png", aUserdir, PngCounter);
            if (PngCounter < 100000)
                str_format(aBuf, sizeof(aBuf), "%s/tmp/pixelstream/png%i.png", aUserdir, PngCounter);

            png_t Png;
            png_init(0,0);
            png_open_file_write(&Png, aBuf); // ignore_convention
            png_set_data(&Png, W, H, 8, PNG_TRUECOLOR, (unsigned char *)pData); // ignore_convention
            png_close_file(&Png); // ignore_convention

            if (time_get() - SpeedTime > time_freq())
            {
                dbg_msg("Frame", "%i (Avg: %.2f s)", PngCounter, ((float)(time_get() - StartTime) / (float)time_freq()) / (float)PngCounter);
                dbg_msg("FPS", "%.f", (float)PngCounter / ((float)(time_get() - StartTime) / (float)time_freq()));
                SpeedTime = time_get();
            }

        }

        mem_copy(pDataOld, pData, Size);
        tOld = t;
    }
    delete[] pData;
    delete[] pDataOld;
    delete[] pDataMixed;
    return 0;
}
