
typedef std::tuple<TH2S*,TH2S*,TH2S*,TH2S*> argbData;

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

TH2S* getColorHisto(const argbData &data, const int &rank) {
  switch(rank) {
  case 0:
    return std::get<0>(data);
  case 1:
    return std::get<1>(data);
  case 2:
    return std::get<2>(data);
  case 3:
    return std::get<3>(data);
  default:
    return nullptr;
  }
}

UInt_t setArgb32(const argbData &data, const UInt_t &xindx, const UInt_t &yindx) {
  UInt_t argb32 = 0;
  for(int ij=0;ij<nComp;ij++) {
    argb32 <<= 8;
    argb32 += getColorHisto(data, ij)->GetBinContent(xindx,yindx);
  }
  return argb32;
}

argbData getImage(TASImage image) {
  
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

int setImage(const argbData &data, TASImage &image) {
  UInt_t yPixels = image.GetHeight();
  UInt_t xPixels = image.GetWidth();
  
  UInt_t h_yPixels = (std::get<0>(data))->GetNbinsY();
  UInt_t h_xPixels = (std::get<0>(data))->GetNbinsX();

  if(yPixels != h_yPixels || xPixels != h_xPixels) {return 1;}

  UInt_t *argb   = image.GetArgbArray();

  for (int row=0; row<xPixels; ++row) {
    for (int col=0; col<yPixels; ++col) {
      int index = col*xPixels+row;
      argb[index] = setArgb32(data, row+1, yPixels-col);
    }
  }
    
  return 0;
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
  
  vector<argbData> hist_argb;
  hist_argb.reserve(frameCount);
  
  for(int fcnt = 0; fcnt<frameCount; fcnt++) {
    Ssiz_t lastindex = filelist[fcnt].Last('.'); 
    TString rawname = filelist[fcnt](0, lastindex);
    cout<<" rawname "<<rawname<<endl;
 
    TString filePath = dir + filelist[fcnt];
    // cout<<" filePath "<<filePath<<endl;
    TASImage image(filePath);

    auto returnHisto = getImage(image);

    TH2S* histos[nComp];
    for(int ij=0;ij<nComp;ij++) {
      histos[ij] = getColorHisto(returnHisto, ij);
      TString name = rawname + "_" + colorComponents[ij];
      histos[ij]->SetNameTitle(rawname,rawname);
      
      // TString tmpFilePath = dir + rawname + "_" + colorComponents[ij] + "_new.root";
      // histo->SaveAs(tmpFilePath);
    }
    hist_argb.push_back(std::make_tuple(histos[0], histos[1], histos[2], histos[3]));
    
  } // for(int fcnt = 0; fcnt<frameCount; fcnt++) {

  for(int fcnt = 1; fcnt<frameCount; fcnt++) {
    
    Ssiz_t lastindex = filelist[fcnt].Last('.'); 
    TString rawname = filelist[fcnt](0, lastindex);
    cout<<" rawname "<<rawname<<endl;

    TH2S* histos[nComp];
    for(int ij=0;ij<nComp;ij++) {
      
      histos[ij] = (TH2S*)(getColorHisto(hist_argb[fcnt-1], ij))->Clone();
      histos[ij]->SetDirectory(0);

      UInt_t h_yPixels = histos[ij]->GetNbinsY();
      UInt_t h_xPixels = histos[ij]->GetNbinsX();
      for (int row=0; row<h_xPixels; ++row) {
	for (int col=0; col<h_yPixels; ++col) {
	  Int_t tmpval = histos[ij]->GetBinContent(row+1, col+1) -
	    getColorHisto(hist_argb[fcnt], ij)->GetBinContent(row+1, col+1);
	  histos[ij]->SetBinContent(row+1, col+1, abs(tmpval));
	}
      }
      
      // TString tmpFilePath = dir + rawname + "_" + colorComponents[ij] + "_new.root";
      // histo1->SaveAs(tmpFilePath);

      // delete histo1;

    } // for(int ij=0;ij<nComp;ij++) {

    UInt_t h_yPixels = histos[0]->GetNbinsY();
    UInt_t h_xPixels = histos[0]->GetNbinsX();
        
    TASImage image(h_xPixels, h_yPixels);
    int status = setImage(std::make_tuple(histos[0], histos[1], histos[2], histos[3]), image);

    TString tmpFilePath = dir + rawname + "_new.jpg";
    image.WriteImage(tmpFilePath);
    
  }   // for(int fcnt = 1; fcnt<frameCount; fcnt++) {
  
}
