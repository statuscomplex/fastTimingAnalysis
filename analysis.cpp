#include <algorithm>
//basic event loop
void doAnalysis(string a_fileName,int a_numBoards)
{
  //allocating space
  vector<vector<float> > waveFormVec(4*2*a_numBoards);
  for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
  {
    iter->resize(1024);
  }
  //open the root file
  TFile* rootFile = new TFile(a_fileName.c_str());
  //test for failure
  if(rootFile==NULL)
  {
    cout << "trouble opening file, doing nothing" << endl;
    return;
  }
  //pointer to assign the tree to
  TTree* rootTree;
  //point the tree in the file to the pointer
  rootFile->GetObject("eventTree",rootTree);
  //test for failure
  if(rootTree==NULL)
  {
    cout << "unable to obtain tree, doing nothing" << endl;
    return;
  }
  int index = 0;
  //just an alias for root's finikyness
  vector<float>* alias[16];
  for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
  {

    stringstream name;
    alias[index] = &(*iter);
    if(index < 4*a_numBoards)
    {
       name << "timeBase_" << index;
    }
    else
    {
       name << "waveForms_" << (index-4*a_numBoards);
    }
    //this tells root where to put data when GetEntry is called on the tree
    rootTree->SetBranchAddress(name.str().c_str(),&alias[index]);
    index++;
  }
  //how many events does the tree have
  int numEntries = rootTree->GetEntries();
  //loop over the enries
  for(int ie = 0;ie < numEntries;ie++)
  {
    //this pulls the data off disk and into memory
    rootTree->GetEntry(ie);
    //print the first 100 samples of the 0th channel of the event
    for(int is = 0; is < 100;is++)
    {
      cout << waveFormVec[4][is] << " ";
    }
    cout << endl;
  }

}


//basic event loop
TH1* getDifference(string a_fileName,int a_numBoards)
{
  TH1F* difference = new TH1F("timeDifference","Time Difference (ns)",10000,-100,100);
  vector<vector<float> > waveFormVec(4*2*a_numBoards);
  for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
  {
    iter->resize(1024);
  }

  TFile* rootFile = new TFile(a_fileName.c_str());
  if(rootFile==NULL)
  {
    cout << "trouble opening file, doing nothing" << endl;
    return NULL;
  }
  TTree* rootTree;
  rootFile->GetObject("eventTree",rootTree);
  if(rootTree==NULL)
  {
    cout << "unable to obtain tree, doing nothing" << endl;
    return NULL;
  }
  vector<TBranch*> branches;
  int index = 0;
  vector<float>* alias[16];
  for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
  {

    stringstream name;
    alias[index] = &(*iter);
    if(index < 4*a_numBoards)
    {
       name << "timeBase_" << index;
    }
    else
    {
       name << "waveForms_" << (index-4*a_numBoards);
    }
    rootTree->SetBranchAddress(name.str().c_str(),&alias[index]);
    index++;
  }
  int numEntries = rootTree->GetEntries();
  for(int ie = 0;ie < numEntries;ie++)
  {
    //0 2
    rootTree->GetEntry(ie);
    int minIndexLeft = std::min_element(waveFormVec[8].begin()+10,waveFormVec[8].end()-10)-waveFormVec[8].begin();
    int minIndexRight = std::min_element(waveFormVec[10].begin()+10,waveFormVec[10].end()-10)-waveFormVec[10].begin();
    // difference->Fill(waveFormVec[0][minIndexLeft]-waveFormVec[2][minIndexRight]);
    difference->Fill(minIndexLeft-minIndexRight);
    

  }
  return difference;

}
//copy and paste loop that does the most basic integration
TH1* getPulseIntegral(string a_fileName,int a_numBoards,int a_channel)
{
  TH1F* amplitude = new TH1F("integral","Charge",10000,-100,100);
  vector<vector<float> > waveFormVec(4*2*a_numBoards);
  for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
  {
    iter->resize(1024);
  }

  TFile* rootFile = new TFile(a_fileName.c_str());
  if(rootFile==NULL)
  {
    cout << "trouble opening file, doing nothing" << endl;
    return NULL;
  }
  TTree* rootTree;
  rootFile->GetObject("eventTree",rootTree);
  if(rootTree==NULL)
  {
    cout << "unable to obtain tree, doing nothing" << endl;
    return NULL;
  }
  vector<TBranch*> branches;
  int index = 0;
  vector<float>* alias[16];
  for(auto iter = waveFormVec.begin();iter!=waveFormVec.end();++iter)
  {

    stringstream name;
    alias[index] = &(*iter);
    if(index < 4*a_numBoards)
    {
       name << "timeBase_" << index;
    }
    else
    {
       name << "waveForms_" << (index-4*a_numBoards);
    }
    rootTree->SetBranchAddress(name.str().c_str(),&alias[index]);
    index++;
  }
  int numEntries = rootTree->GetEntries();
  for(int ie = 0;ie < numEntries;ie++)
  {
    //0 2
    rootTree->GetEntry(ie);

    // difference->Fill(waveFormVec[0][minIndexLeft]-waveFormVec[2][minIndexRight]);
    float sum = 0;
    float numTested=0;
    for(auto iter = waveFormVec[a_channel+a_numBoards*4].begin()+10;iter!=waveFormVec[a_channel+a_numBoards*4].begin()+100;++iter)
    {
      sum+=*iter;
      numTested++;
    }
    float average = sum/numTested;
    float charge = 0;
    auto timeIter =waveFormVec[a_channel].begin()+10; 
    for(auto iter = waveFormVec[a_channel+a_numBoards*4].begin()+10;iter!=waveFormVec[a_channel+a_numBoards*4].end()-10;++iter)
    {
      float deltaT = *(timeIter+1)-*timeIter;
      float sample = -(*iter)+average;
      charge += sample*deltaT/50.0;
      timeIter++;
    }
    amplitude->Fill(charge);

  }
  return amplitude;

}
/*

for(int i=1; i < 100;i++){ eventTree->Draw("waveForms_4:timeBase_4","","l",1,i);gSystem->ProcessEvents();usleep(100);gPad->Update();}
*/
