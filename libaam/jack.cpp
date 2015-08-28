 // energyXT - JACK audio interface
// author: jorgen aase - www.energy-xt.com and ralf beck
// compile: g++ -shared -lasound -ljack jack.cpp -o libaam.so
// set
#include <alsa/asoundlib.h>
#include <jack/jack.h>
#include <stdlib.h>
#include <signal.h>

// define max values for in/outputs

#define MAX_JACK_INS  128
#define MAX_JACK_OUTS 128
#define MAX_MIDI_INS  64
#define MAX_MIDI_OUTS 64

// default values
#define DEFAULT_JACK_INS  2
#define DEFAULT_JACK_OUTS 2
#define DEFAULT_MIDI_INS  2
#define DEFAULT_MIDI_OUTS 2

// actual values used by this instance

int num_midi_ins;
int num_midi_outs;
int num_jack_ins;
int num_jack_outs;

// libaam interface

int libaam (int index, int value1, int value2) asm ("libaam");

const int STREAM_INIT = 0;          
const int STREAM_OPEN = 1;          
const int STREAM_CLOSE = 2;         
//const int STREAM_PLAY = 3;          
//const int STREAM_STOP = 4;   
const int STREAM_EXIT = 5;          
const int STREAM_ABOUT = 6;         
const int STREAM_AUDIO_COUNT = 7;
const int STREAM_AUDIO_NAME = 8;
const int STREAM_FRAMES = 9;        
const int STREAM_RATE = 10;         
const int STREAM_LATENCY = 17;      
const int STREAM_RELOAD = 18;                
const int MIDI_IN_COUNT = 30;
const int MIDI_IN_NAME = 31;
const int MIDI_IN_ENABLE = 32;    
const int MIDI_IN_ENABLED = 33;    
const int AUDIO_IN_COUNT = 40;
const int AUDIO_IN_NAME = 41;
const int AUDIO_IN_ENABLE = 42;
const int AUDIO_IN_ENABLED = 43;
const int AUDIO_OUT_COUNT = 50;
const int AUDIO_OUT_NAME = 51;
const int AUDIO_OUT_ENABLE = 52;
const int AUDIO_OUT_ENABLED = 53;

// create a new alsa sequencer client

snd_seq_t *aseq_client;

snd_seq_t *open_client()
{
        snd_seq_t *handle;
        int err;
        err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
        if (err < 0)
                return NULL;
        snd_seq_set_client_name(handle, "EnergyXT2");
        return handle;
}

// create a new port; return the port id
// port will be read/writable and accept the read/write-subscription.
int new_in_port(snd_seq_t *handle, char *name)
{
        return snd_seq_create_simple_port(handle, name,
                        SND_SEQ_PORT_CAP_WRITE
                        |SND_SEQ_PORT_CAP_SUBS_WRITE,
                        SND_SEQ_PORT_TYPE_MIDI_GENERIC);
}

// create a new port; return the port id
// port will be read/writable and accept the read/write-subscription.
int new_out_port(snd_seq_t *handle, char *name)
{
        return snd_seq_create_simple_port(handle, name,
                        SND_SEQ_PORT_CAP_READ
                        |SND_SEQ_PORT_CAP_SUBS_READ,
                        SND_SEQ_PORT_TYPE_MIDI_GENERIC);
}

typedef int (CProcess) (float** inputs, float** outputs, int numIns, int numOuts,
    int frames, void* midiData, int midiProducerIndex);
CProcess* process = 0;

class CThread
{
  public:
    bool terminated;
    pthread_t handle;
    CThread();
    ~CThread();
    virtual void execute() {};
};

void* threadProc(void * param)
{
  CThread* thread = (CThread*)param;
  thread->execute();
    return 0;
}

CThread :: CThread()
{
  terminated = false;
  pthread_create(&handle, 0, &threadProc, this);  
}

CThread :: ~CThread()
{
  terminated = true;
  pthread_kill(handle, SIGKILL);
  pthread_join(handle, 0);
}

struct CMIDIData
{
    int data;                // midi bytes
    char port;                // midi port index
    void* buffer;            // pointer to buffer (sysex etc)
};

class CMIDIThread : public CThread
{
  public:
    void execute();
};

int midiDevices = 0;
CMIDIThread *midiThread = 0;

unsigned int frames = 1024;
unsigned int rate = 44100;

// jack audio

struct CJackPort
{
    char id[255];
    int enabled, active;
    jack_port_t *port;
};

int aoutcount = 0, aincount = 0, ainused = 0, aoutused = 0;
jack_client_t *client = 0;
CJackPort ain[MAX_JACK_INS], aout[MAX_JACK_OUTS];

// midi device

class CMIDIDev
{
  public:
    snd_rawmidi_t *handle;
    char device[255], id[255];
    int enabled;
    CMIDIThread* thread;
    int index;
    CMIDIDev();
    ~CMIDIDev();
    void enable(int enable);
};

CMIDIDev* mdevs[MAX_MIDI_INS];

// midi data buffer

#define MIDIBUFFERSIZE 512

CMIDIData midiData[MIDIBUFFERSIZE];
volatile int midiProducerIndex = 0;
volatile int midiConsumerIndex = 0;

// jack process

jack_default_audio_sample_t* outputs[MAX_JACK_OUTS], *inputs[MAX_JACK_INS];    

int process_jack (jack_nframes_t samples, void *arg)
{

    int i, tempBufferCount, producer, used;
        
    CMIDIData tempBuffer[128];
    jack_transport_state_t ts = jack_transport_query(client, 0);
    
//    if (ts == JackTransportRolling)
//    {
        
        // intput & output buffers
        
        used = 0;
        for (i = 0; i < num_jack_ins, used < ainused; i++)
        {
            if (ain[i].active)
                inputs[used++] = (jack_default_audio_sample_t*) jack_port_get_buffer (ain[i].port, samples);            
        }
        used = 0;
        for (i = 0; i < num_jack_outs, used < aoutused; i++)
        {
            if (aout[i].active)
                outputs[used++] = (jack_default_audio_sample_t*) jack_port_get_buffer (aout[i].port, samples);            
        }
        
        // midi in
        
                tempBufferCount = 0;
                producer = midiProducerIndex;

                if (midiConsumerIndex > producer) {
           memcpy(&tempBuffer[tempBufferCount], &midiData[midiConsumerIndex], (MIDIBUFFERSIZE - midiConsumerIndex) * sizeof(CMIDIData));
                   tempBufferCount = MIDIBUFFERSIZE - midiConsumerIndex;
                   midiConsumerIndex = 0;
                }
                memcpy(&tempBuffer[tempBufferCount], &midiData[midiConsumerIndex], (producer - midiConsumerIndex) * sizeof(CMIDIData));
                tempBufferCount += producer - midiConsumerIndex;
                midiConsumerIndex = producer;

        // process
        
        process(inputs, outputs, ainused, aoutused, samples, &tempBuffer, tempBufferCount);        
        
        // todo: midi out
        
//    }
        
    return 0;      
    
}

void jack_shutdown (void *arg)
{
    // todo
}


// jack open device

void openJack()
{

    char str[255];

    jack_status_t status;
    jack_options_t options = JackNullOption;
    
    ainused = 0;
    aoutused = 0;
    
    // open client
    
    aincount = 0;
    aoutcount = 0;

    client = jack_client_open ("energyXT2", options, &status, 0);
    if (client)
    {

        // setup process

        jack_set_process_callback(client, process_jack, 0);
        jack_on_shutdown(client, jack_shutdown, 0);

        // get sample rate

        rate = jack_get_sample_rate (client);        
        
        // set/get buffer frames

        frames = jack_get_buffer_size (client);        

        // activate client
        
        if (jack_activate (client))
        {
            jack_client_close (client);                                
            client = 0;                
            printf("jack_activate failed\n");
        }        

        // inputs and outputs    
            
        else {
            
            int i;
            
            // audio inputs
            
                        for (i=0; i<num_jack_ins; i++) {

                            sprintf(str, "in%d", i);
                            strcpy(ain[i].id, str);
                            aincount++;
                            ain[i].active = 0;
                            
                            sprintf(str, "in%d", ainused);
                            ain[i].port = jack_port_register
                                            (client, str, JACK_DEFAULT_AUDIO_TYPE,
                                             JackPortIsInput, 0);
                            if (ain[i].port) {
 
                               ainused++;
                               ain[i].active = 1;

                            } // if ain[i].port

                        } // for

            // audio outputs            

                        for (i=0; i<num_jack_outs; i++) {

                            sprintf(str, "out%d", i);
                            strcpy(aout[i].id, str);
                            aoutcount++;
                            aout[i].active = 0;

                            sprintf(str, "out%d", aoutused);
                            aout[i].port = jack_port_register
                                             (client, str, JACK_DEFAULT_AUDIO_TYPE,
                                              JackPortIsOutput, 0);
                            if (aout[i].port) {

                               aoutused++;
                               aout[i].active = 1;

                            } // if aout[i].port

                        } // for

        } // if jack_activate
        
    } // if client    
    
}

// jack close device

void closeJack()
{
    if (client)
    {
        jack_client_close (client);                                
        client = 0;
    }
}

// init

void init()
{

     char id[16];
     char *env;

     int i;
     int in;

     num_jack_ins = DEFAULT_JACK_INS;

     if ((env = getenv("ENERGYXT2_JACK_INS")) != NULL) {

        num_jack_ins = atoi(env);

        if (num_jack_ins < 0)
        num_jack_ins = DEFAULT_JACK_INS;
     
        if (num_jack_ins > MAX_JACK_INS)
           num_jack_ins = MAX_JACK_INS;

     }

     num_jack_outs = DEFAULT_JACK_OUTS;

     if ((env = getenv("ENERGYXT2_JACK_OUTS")) != NULL) {

        num_jack_outs = atoi(env);

        if (num_jack_outs < 0)
           num_jack_outs = DEFAULT_JACK_OUTS;

        if (num_jack_outs > MAX_JACK_OUTS)
           num_jack_outs = MAX_JACK_OUTS;

     }

     num_midi_ins = DEFAULT_MIDI_INS;

     if ((env = getenv("ENERGYXT2_MIDI_INS")) != NULL) {

        num_midi_ins = atoi(env);

        if (num_midi_ins < 0)
           num_midi_ins = DEFAULT_MIDI_INS;

        if (num_midi_ins > MAX_MIDI_INS)
           num_midi_ins = MAX_MIDI_INS;

     }

     num_midi_outs = DEFAULT_MIDI_OUTS;

     if ((env = getenv("ENERGYXT2_MIDI_OUTS")) != NULL) {

        num_midi_outs = atoi(env);

        if (num_midi_outs < 0)
           num_midi_outs = DEFAULT_MIDI_OUTS;

        if (num_midi_outs > MAX_MIDI_OUTS)
           num_midi_outs = MAX_MIDI_OUTS;

     }

     // disable all channels (by default)
    
     for (i = 0; i < MAX_JACK_INS; i++)
     {
          ain[i].enabled = false;
     }

     for (i = 0; i < MAX_JACK_INS; i++)
     {
          aout[i].enabled = false;
     }

     midiDevices = 0;

     // ALSA midi    
    
     aseq_client = open_client();

     for (i=0; i<num_midi_ins; i++) {

         sprintf(id, "aseq %d", i);
         if ((in = new_in_port(aseq_client, id)) != -1) {

            mdevs[i] = new CMIDIDev();
            mdevs[i]->index = in;
            strcpy(mdevs[i]->device, "alsaseq");
            strcpy(mdevs[i]->id, id);
            midiDevices++;
    
         } // if

     } // for

     if (midiDevices)
        midiThread = new CMIDIThread();

} // init

// deinit

void deinit()
{
}

// dispatcher

int libaam (int index, int value1, int value2)
{
  int i, result = -1;

  switch (index)
  {
        
    // init library
    
    case STREAM_INIT:     
        init();                
        process = (CProcess*)value1;        
        break;
        
      // exit library
                
      case STREAM_EXIT:
          deinit();
           break;
        
      // device count
      
      case STREAM_AUDIO_COUNT:
        result = 1; // jack only
        break;
                
      // device name
      
      case STREAM_AUDIO_NAME:     
        strcpy((char*)value2, "JACK Audio");    
        break;

      // open device
      
      case STREAM_OPEN:
          openJack();
          result = 1;
        break;

      // close device
      
      case STREAM_CLOSE:
          closeJack();
        result = 1;
        break;
        
      // get/set frames
      
      case STREAM_FRAMES:
        if (value1 > 0)
          frames = value1;
        result = frames;
        break;
        
      // get/set rate
      
      case STREAM_RATE:
          if (value1 == 0)
                *(float*)value2 = rate;
        else if (value1 > 0)
          rate = value1;
        break;
        
      // audio in count
      
      case AUDIO_IN_COUNT:
          return aincount;
        break;
                    
      // audio out enable
      
      case AUDIO_IN_ENABLE:
            if (value1 >= 0 && value1 < num_jack_ins)
                ain[value1].enabled = value2;
        break;

      // audio in enabled
      
      case AUDIO_IN_ENABLED:
            if (value1 >= 0 && value1 < num_jack_ins)
                result = ain[value1].enabled;
        break;
        
      // audio in name
      
      case AUDIO_IN_NAME:
            if (value1 >= 0 && value1 < aincount)
              strcpy((char*)value2, ain[value1].id);
        break;
            
      // audio out count
      
      case AUDIO_OUT_COUNT:
          return aoutcount;
        break;
            
      // audio out enable
      
      case AUDIO_OUT_ENABLE:
            if (value1 >= 0 && value1 < num_jack_outs)
                aout[value1].enabled = value2;
        break;

      // audio out enabled
      
      case AUDIO_OUT_ENABLED:
            if (value1 >= 0 && value1 < num_jack_outs)
                result = aout[value1].enabled;
        break;
                    
      // audio out name
      
      case AUDIO_OUT_NAME:
            if (value1 >= 0 && value1 < aoutcount)
              strcpy((char*)value2, aout[value1].id);
        break;
        
        // midi in count
        
      case MIDI_IN_COUNT:
        return midiDevices;
        break;

        // midi in name

      case MIDI_IN_NAME:
          if (value1 >= 0 && value1 < midiDevices)
            strcpy((char*)value2, mdevs[value1]->id);
        break;

        // enable midi in

      case MIDI_IN_ENABLE:
          if (value1 >= 0 && value1 < midiDevices)
            mdevs[value1]->enable(value2);
        break;

        // is midi in enabled?
 
      case MIDI_IN_ENABLED:
          if (value1 >= 0 && value1 < midiDevices)
            return mdevs[value1]->enabled;
      break;

  }
  
  return result;
  
}

CMIDIDev :: CMIDIDev()
{
        enabled = 0;  
}

CMIDIDev::~CMIDIDev()
{
    enable(0);
}

// midi dev enable

void CMIDIDev :: enable(int enable)
{

  enabled = enable;
    
}

//CMIDIThread :: CMIDIThread()
//{    
//};

// handle midi

void handleMIDI(snd_seq_event_t *ev)
{

    int i = 0;
    int data = 0;

    switch (ev->type) {

    case SND_SEQ_EVENT_NOTEON:
       data = (0x90 | ev->data.note.channel) |
              (ev->data.note.note << 8) |
              (ev->data.note.velocity << 16);
       break;
    case SND_SEQ_EVENT_NOTEOFF:
       data = (0x80 | ev->data.note.channel) |
              (ev->data.note.note << 8) |
              (ev->data.note.off_velocity << 16);
       break;
    case SND_SEQ_EVENT_KEYPRESS:
       data = (0xA0 | ev->data.note.channel) |
              (ev->data.note.note << 8) |
              (ev->data.note.velocity << 16);
       break;
    case SND_SEQ_EVENT_CONTROLLER:
       data = (0xB0 | ev->data.control.channel) |
              (ev->data.control.param << 8) |
              (ev->data.control.value << 16);
       break;
    case SND_SEQ_EVENT_PGMCHANGE:
       data = (0xC0 | ev->data.control.channel) |
              (ev->data.control.value << 8);
       break;
    case SND_SEQ_EVENT_CHANPRESS:
       data = (0xD0 | ev->data.control.channel) |
              (ev->data.control.value << 8);
       break;
    case SND_SEQ_EVENT_PITCHBEND:
       ev->data.control.value += 8192;
       data = (0xE0 | ev->data.control.channel) |
              ((ev->data.control.value & 0x7f) << 8);
              (((ev->data.control.value >> 7) & 0x7f) << 16);
       break;
    case SND_SEQ_EVENT_CLOCK:
       data = 0xF8;
       break;
    case SND_SEQ_EVENT_SONGPOS:
       data = 0xF2 |
              ((ev->data.control.value & 0x7f) << 8);
              (((ev->data.control.value >> 7) & 0x7f) << 16);
       break;
    case SND_SEQ_EVENT_START:
       data = 0xFA;
       break;
    case SND_SEQ_EVENT_STOP:
       data = 0xFC;
       break;
    case SND_SEQ_EVENT_CONTINUE:
       data = 0xFB;
       break;

    default:
       break;

    } // switch (ev)->type
 
    if (data)
    {        
        midiData[midiProducerIndex].data = data;
        midiData[midiProducerIndex].port = ev->dest.port;
        midiProducerIndex = (midiProducerIndex + 1) % MIDIBUFFERSIZE;            
    }
    
} // handleMIDI

// midi thread

void CMIDIThread :: execute()
{
  int count, ms = 1;
  snd_seq_event_t *ev = 0;

  while (!terminated)
  {

        snd_seq_event_input(aseq_client, &ev);
        if (mdevs[ev->dest.port]->enabled)
           handleMIDI(ev);

  }

} // CMIDIThread :: execute()
