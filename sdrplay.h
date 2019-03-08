#define SDR_SAMPLE_RATE 2400000 // sample rate of the SDRplay hardware

void init_SDRplay();
void remove_SDRplay();
void streamCallback(short *xi, short *xq, unsigned int firstSampleNum,
    int grChanged, int rfChanged, int fsChanged, unsigned int numSamples,
    unsigned int reset, unsigned int hwRemoved, void *cbContext);
void gainCallback(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext);

extern int samplesPerPacket;
