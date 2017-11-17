/********************************************************************\

  Name:         drs_exam_multi.cpp
  Created by:   Stefan Ritt

  Contents:     Simple example application to read out a several
                DRS4 evaluation board in daisy-chain mode

  $Id: drs_exam_multi.cpp 21509 2014-10-15 10:11:36Z ritt $

\********************************************************************/

#include <math.h>

#ifdef _MSC_VER

#include <windows.h>

#elif defined(OS_LINUX)

#define O_BINARY 0

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DIR_SEPARATOR '/'

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "strlcpy.h"
#include "DRS.h"

using namespace std;

//root includes
#include "TFile.h"
#include "TTree.h"

/*------------------------------------------------------------------*/
//main function for acquiring waveforms using the drs4 eval boards.
//the code is identical to the code supplied in drs_exam_multi.cpp
//in regard to board control. Data structures to store data on the 
//machine being used for acquisition have change and support has been 
//added for writing out wave forms in ROOT TTree data format
//where the resulting event format on disk is vectors of floats in 
//seperate branches. The The branch names are waveforms_i and 
//timeBase_i where i is the channel number. The channel numbers are
//0 indexed begining with the first board. Work was done to generalize 
//the code so that it works with n boards. 
int main(int argc, char const *argv[])
{
   int i, j, k;
   DRS *drs;
   DRSBoard *b, *mb;
//get the data off the stack by removing this
   // float time_array[8][1024];
   // float wave_array[8][1024];
//puts code back to original state if -DROOTEXISTS is not included as a compile flag
#ifndef ROOTEXISTS
   FILE  *f;
#endif
   /* do initial scan, sort boards accordning to their serial numbers */
   drs = new DRS();
   drs->SortBoards();

   /* show any found board(s) */
   for (i=0 ; i<drs->GetNumberOfBoards() ; i++) {
      b = drs->GetBoard(i);
      printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n", 
         b->GetBoardSerialNumber(), b->GetFirmwareVersion());
      if (b->GetBoardType() < 8) {
         printf("Found pre-V4 board, aborting\n");
         return 0;
      }
   }
   int numBoards = drs->GetNumberOfBoards();
   cout << "found " << numBoards << " boards" << endl;
   cout << "allocating waveform memory " << endl;
   //getting memory on the heap to store waveforms and associated times
   vector<vector<float> > waveFormVec(4*2*numBoards);
   for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
   {
      iter->resize(1024);
   }
   //create data structure that mimicks the original but instead points
   //to base of vectors for storing data
   float* time_array[4*numBoards];
   float* wave_array[4*numBoards];
   for(int i = 0; i < 4*numBoards; i++)
   {
      time_array[i] = &waveFormVec[i][0];
      wave_array[i] = &waveFormVec[i+4*numBoards][0];
   }
   /* exit if no board found */
   if (drs->GetNumberOfBoards() == 0) {
      printf("No DRS4 evaluation board found\n");
      return 0;
   }
   //this was removed since the code should now function on n boards
   //including just 1
   /* exit if only one board found */
   // if (drs->GetNumberOfBoards() == 1) {
   //    printf("Only one DRS4 evaluation board found, please use drs_exam program\n");
   //    return 0;
   // }
   
   /* use first board with highest serial number as the master board */
   mb = drs->GetBoard(0);
   
   /* common configuration for all boards */
   for (i=0 ; i<drs->GetNumberOfBoards() ; i++) {
      b = drs->GetBoard(i);
      
      /* initialize board */
      b->Init();
      
      /* select external reference clock for slave modules */
      /* NOTE: this only works if the clock chain is connected */
      if (i > 0) {
         if (b->GetFirmwareVersion() >= 21260) { // this only works with recent firmware versions
            if (b->GetScaler(5) > 300000)        // check if external clock is connected
               b->SetRefclk(true);               // switch to external reference clock
         }
      }
      
      /* set sampling frequency */
      b->SetFrequency(5, true);
      
      /* set input range to -0.5V ... +0.5V */
      b->SetInputRange(0);

      /* enable hardware trigger */
      b->EnableTrigger(1, 0);
      
      if (i == 0) {
         /* master board: enable hardware trigger on CH1 at 50 mV positive edge */
         b->SetTranspMode(1);
         b->SetTriggerSource(1<<3);        // set CH3 as source
         b->SetTriggerLevel(-0.25);         // -2.5 mV
         b->SetTriggerPolarity(true);     // negative edge
         b->SetTriggerDelayNs(100);          // zero ns trigger delay
      } else {
         /* slave boards: enable hardware trigger on Trigger IN */
         b->SetTriggerSource(1<<4);        // set Trigger IN as source
         b->SetTriggerPolarity(false);     // positive edge
         b->SetTriggerDelayNs(100);          // zero ns trigger delay

      }
   }
//puts code back to original state if -DROOTEXISTS is not included as a compile flag
#ifdef ROOTEXISTS
   if(argc<2)
   {
      //command line takes filename for root storage
      cout << "File name not specified for root operation " << endl;
      cout << "usage drs_exam_multi filename.root" << endl;
      return 0;
   }
   //create root file to store the TTree in
   TFile* rootFile = new TFile(argv[1],"recreate");
   //TTree creation, tree is in the file
   TTree* rootTree = new TTree("eventTree","eventTree");
   int index = 0;
   //loop over outer vector container and create branches out of the 
   //inner containers that are 1024 long
   for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
   {
      //unique names for the branches
      stringstream name;
      if(index < 4*numBoards)
      {
         name << "timeBase_" << index;
      }
      else
      {
         name << "waveForms_" << (index-4*numBoards);
      }
      //creates the branch for the vector the iterator is pointing to.
      rootTree->Branch(name.str().c_str(),&(*iter));
      index++;
   }
//oroginal code
#else
   /* open file to save waveforms */
   f = fopen("data.txt", "w");
   if (f == NULL) {
      perror("ERROR: Cannot open file \"data.txt\"");
      return 1;
   }
#endif
   //changed to prompt user for number of events
   int events = 0; 
   cout << "How many events would you like to record?" << endl;
   cin >> events;
   /* repeat ten times */
   for (i=0 ; i<events ; i++) {

      /* start boards (activate domino wave), master is last */
      for (j=drs->GetNumberOfBoards()-1 ; j>=0 ; j--)
      {
        drs->GetBoard(j)->StartDomino();
      }

      /* wait for trigger on master board */
      printf("Waiting for trigger...");
      fflush(stdout);
      while (mb->IsBusy())
         { 
            // sleep(0.001);
         };
//puts code back to original state if -DROOTEXISTS is not included as a compile flag
#ifndef ROOTEXISTS
      fprintf(f, "Event #%d =====================================================\n", j);
#endif
      for (j=0 ; j<drs->GetNumberOfBoards() ; j++) {
         b = drs->GetBoard(j);
         if (b->IsBusy()) 
           {
             cout << "busy skipping event" << endl;
             i--; /* skip that event, must be some fake trigger */
             break;
           }
         
         /* read all waveforms from all boards */
         b->TransferWaves(0, 8);
         
         for (k=0 ; k<4 ; k++) {
            //this now puts the data in the new data stuctures that are 
            //associated with the new branches
            /* read time (X) array in ns */
            b->GetTime(0, k*2, b->GetTriggerCell(0), time_array[k+j*4]);

            /* decode waveform (Y) arrays in mV */
            b->GetWave(0, k*2, wave_array[k+j*4]);
         }
//puts code back to original state if -DROOTEXISTS is not included as a compile flag
#ifdef ROOTEXISTS

#else
         /* Save waveform: X=time_array[i], Channel_n=wave_array[n][i] */
         fprintf(f, "Board #%d ---------------------------------------------------\n t1[ns]  u1[mV]  t2[ns]  u2[mV]  t3[ns]  u3[mV]  t4[ns]  u4[mV]\n", b->GetBoardSerialNumber());
         for (k=0 ; k<1024 ; k++)
            fprintf(f, "%7.3f %7.1f %7.3f %7.1f %7.3f %7.1f %7.3f %7.1f\n",
                    time_array[0][k+j*4], wave_array[0][k+j*4],
                    time_array[1][k+j*4], wave_array[1][k+j*4],
                    time_array[2][k+j*4], wave_array[2][k+j*4],
                    time_array[3][k+j*4], wave_array[3][k+j*4]);
#endif
      }
      //at this point the data structures are full of the event data
      //calling fill stores the data in the tree and moves the tree
      //to the next event. When the buffer size is full, the data are
      //moved to disk which is why the program appears to pause 
      rootTree->Fill();

      /* print some progress indication */
      printf("\rEvent #%d read successfully\n", i);
   }

//puts code back to original state if -DROOTEXISTS is not included as a compile flag
#ifdef ROOTEXISTS
//data collection if finished, so the current state of the tree is written to 
//to disk and the file is closed.
rootFile->Write();
rootFile->Close();
#else
   fclose(f);
#endif
   printf("Program finished.\n");

   /* delete DRS object -> close USB connection */
   delete drs;
}
