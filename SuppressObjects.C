

const int nComp = 4;
TString colorComponents[nComp] = {'A','R','G','B'};


Byte_t getArgbComponent(const TString &color, const UInt_t &Argb) {
  int whichPart = -1;
  if(color == "A") {whichPart = 3;}
  if(color == "R") {whichPart = 2;}
  if(color == "G") {whichPart = 1;}
  if(color == "B") {whichPart = 0;}

  if(whichPart < 0) {return 0;};

  return Byte_t ( (Argb >> (whichPart * 8)) & 0xFF ); 
}

std::tuple<TH2S*,TH2S*,TH2S*,TH2S*> getImage(TASImage image) {
  
  UInt_t yPixels = image.GetHeight();
  UInt_t xPixels = image.GetWidth();
  UInt_t *argb   = image.GetArgbArray();

  TH2S* histo[nComp];

  for(int ij=0;ij<nComp;ij++) {
    histo[ij] = new TH2S(colorComponents[ij].Data(), colorComponents[ij].Data(),
			 xPixels,-1,1,yPixels,-1,1);
  }

  for (int row=0; row<xPixels; ++row) {
    for (int col=0; col<yPixels; ++col) {
      int index = col*xPixels+row;

      for(int ij=0;ij<nComp;ij++) {
	Byte_t fill = getArgbComponent(colorComponents[ij], argb[index]);
	histo[ij]->SetBinContent(row+1,yPixels-col,fill);
      }
    }
  }

  // delete argb;

  return std::make_tuple(histo[0], histo[1], histo[2], histo[3]);
}


void SuppressObjects(const TString &dir, const TString &log) {

  vector<TString> filelist;
  ifstream file_db(log);
  while(!file_db.eof()) {
    char indatafile[300] = {};
    file_db >> indatafile;
    if (strstr(indatafile,"#")) continue;
    if(file_db.eof()) break;
    filelist.push_back(TString(indatafile));
  }
  int frameCount = filelist.size();
  
  vector<TH2S*> hist_argb[nComp];
  for(int ij=0;ij<nComp;ij++) {
    hist_argb[ij].reserve(frameCount);
  }
  
  for(int fcnt = 0; fcnt<frameCount; fcnt++) {
    Ssiz_t lastindex = filelist[fcnt].Last('.'); 
    TString rawname = filelist[fcnt](0, lastindex);
    cout<<" rawname "<<rawname<<endl;
 
    TString filePath = dir + filelist[fcnt];
    // cout<<" filePath "<<filePath<<endl;
    TASImage image(filePath);

    auto returnHisto = getImage(image);

    for(int ij=0;ij<nComp;ij++) {
      TH2S* histo;
      switch(ij) {
      case 0:
	histo = std::get<0>(returnHisto); break;
      case 1:
	histo = std::get<1>(returnHisto); break;
      case 2:
	histo = std::get<2>(returnHisto); break;
      case 3:
	histo = std::get<3>(returnHisto); break;
      }
      TString name = rawname + "_" + colorComponents[ij];
      histo->SetNameTitle(rawname,rawname);
      hist_argb[ij].push_back(histo);

      // TString tmpFilePath = dir + rawname + "_" + colorComponents[ij] + "_new.root";
      // histo->SaveAs(tmpFilePath);
    }
    
  } // for(int fcnt = 0; fcnt<frameCount; fcnt++) {

  for(int fcnt = 1; fcnt<frameCount; fcnt++) {
    for(int ij=0;ij<nComp;ij++) {
      
      Ssiz_t lastindex = filelist[fcnt].Last('.'); 
      TString rawname = filelist[fcnt](0, lastindex);
      cout<<" rawname "<<rawname<<endl;
      
      TH2S* histo1 = (TH2S*)hist_argb[ij][fcnt-1]->Clone();
      histo1->SetDirectory(0);
      histo1->Add(hist_argb[ij][fcnt], -1);
      
      // TString tmpFilePath = dir + rawname + "_" + colorComponents[ij] + "_new.root";
      // histo1->SaveAs(tmpFilePath);

    } // for(int ij=0;ij<nComp;ij++) {
  }   // for(int fcnt = 1; fcnt<frameCount; fcnt++) {
  
}
