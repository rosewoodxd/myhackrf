
#include "HackRFDevice.h"

#include <fstream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <vector>

#define PI 3.141592653589793

HackRFDevice hackrf;

uint64_t fc_hz      = 2490e6; // center freq
double   fs_hz      = 8e6;   // sample rate
uint32_t lna_gain   = 0;
uint8_t  amp_enable = 0;
uint32_t txvga_gain = 30;


void signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    hackrf.cleanup();
    exit(0);
}


//
// Callback function for tx samples
//
//262144
double t = 0.0;
double dt = 1.0/fs_hz;
double df = 1.5e6; // 1MHz baseband CW
double pri = 1e-3;
double pw = 1e-3;
double amp = 100;
double chirp_width = df + 200e3;
double slopeFactor = (chirp_width - df)/(2.0*pw);


#define SAVE_FILE
#ifdef SAVE_FILE
int write_file = 1;
std::ofstream ofile("tx.csv", std::ofstream::out);
#endif

int sample_block_cb_fn(hackrf_transfer* transfer)
{
    for (int ii = 0; ii < transfer->valid_length; ii+=2)
    {
        double i = 0.0;
        double q = 0.0;
        if ( t < pw ) // 40 ns (1 samples)
        {
            
            
            i = amp * cos( 2.0 * PI * ((df*t)+(slopeFactor*pow(t,2))) );  // I
            q = amp * sin( 2.0 * PI * ((df*t)+(slopeFactor*pow(t,2))) );  // Q
            /*
            if ( t > pw / 2.0 )
                dftemp = df * 2.0;
            else
                dftemp = df;
            
            i = amp * cos( 2.0 * PI * (0.0) * t );  // I
            q = amp * sin( 2.0 * PI * (0.0) * t );  // Q
            */
            uint8_t i8 = uint8_t(i);
            uint8_t q8 = uint8_t(q);
            
            transfer->buffer[ii+0] = i8;
            transfer->buffer[ii+1] = q8;
            
            //transfer->buffer[ii+0] = amp;
            //transfer->buffer[ii+1] = amp;
        }
        else
        {
            transfer->buffer[ii+0] = 0;
            transfer->buffer[ii+1] = 0;
        }
        
#ifdef SAVE_FILE
        if ( write_file == 1 )
        {
            ofile << t << ","
//                  << unsigned(transfer->buffer[ii+1]) << ","
//                  << unsigned(transfer->buffer[ii+1]) << std::endl;
                  << i << ","
                  << q << std::endl;
        }
#endif
        
        t = t+dt;
        if ( t >= pri )
        {
            t = 0.0;
        }
    }
    
    

#ifdef SAVE_FILE
    if ( write_file == 1 )
    {
        write_file = 0;
        ofile.close();
    }
#endif
    
    return 0;
}


//
// MAIN
//
int main()
{
    // Setup the interrupt signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    
    if ( ! hackrf.initialize( "23ec7" ) )
    {
        std::cout << "Error initializing hackrf device!" << std::endl;
        return 1;
    }
    
    hackrf.tune( fc_hz );
    hackrf.set_sample_rate( fs_hz );
    hackrf.set_lna_gain( lna_gain );
    hackrf.set_amp_enable( amp_enable );
    hackrf.set_txvga_gain( txvga_gain );
    hackrf.start_Tx( sample_block_cb_fn, (void*)(NULL) );

    //  Wait a while
    sleep(3000000);
    
    return 0;
}





