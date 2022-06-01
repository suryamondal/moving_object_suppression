
typedef std::tuple<TH2S*,TH2S*,TH2S*,TH2S*> argbData;

const int nComp = 4;
TString colorComponents[nComp] = {'A','R','G','B'};

TH2S* getColorHisto(const argbData &data,
		    const int &rank) {
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

Byte_t getArgb32Component(const TString &color,
			  const UInt_t &Argb) {
  int whichPart = -1;
  if(color == "A") {whichPart = 3;}
  if(color == "R") {whichPart = 2;}
  if(color == "G") {whichPart = 1;}
  if(color == "B") {whichPart = 0;}

  if(whichPart < 0) {return 0;};
  return Byte_t ( (Argb >> (whichPart * 8)) & 0xFF ); 
}

void setArgb32Component(const TString &color,
			UInt_t &Argb,
			const Byte_t &comp) {
  int whichPart = -1;
  if(color == "A") {whichPart = 3;}
  if(color == "R") {whichPart = 2;}
  if(color == "G") {whichPart = 1;}
  if(color == "B") {whichPart = 0;}

  if(whichPart < 0) {std::cout<<" wrong component "<<std::endl;};

  /** setting the color byte to zero */
  UInt_t tArgb = 0xFF;
  tArgb <<= whichPart * 8;
  tArgb ^= 0xFFFFFFFF;
  Argb &= tArgb;

  /** setting the color byte to comp */
  tArgb = comp;
  tArgb <<= whichPart * 8;
  Argb += tArgb;

  return Argb; 
}

UInt_t setArgb32(const argbData &data,
		 const UInt_t &xindx,
		 const UInt_t &yindx) {
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
	Byte_t fill = getArgb32Component(colorComponents[ij], argb[index]);
	histo[ij]->SetBinContent(row+1,yPixels-col,fill);
      }
    }
  }
  // delete argb;
  return std::make_tuple(histo[0], histo[1], histo[2], histo[3]);
}

int setImage(const argbData &data,
	     TASImage &image) {
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

TASImage getDifference(TASImage image1,
		       TASImage image2) {
  /** return = image1 - image2 */

  UInt_t yPixels1 = image1.GetHeight();
  UInt_t xPixels1 = image1.GetWidth();

  UInt_t yPixels2 = image2.GetHeight();
  UInt_t xPixels2 = image2.GetWidth();

  if(yPixels1 != yPixels2 || xPixels1 != xPixels2) {return TASImage();}
  
  UInt_t *argb1   = image1.GetArgbArray();
  UInt_t *argb2   = image2.GetArgbArray();

  TASImage image(xPixels1, yPixels1);
  UInt_t *argb    = image.GetArgbArray();
  
  for (int row=0; row<xPixels1; ++row) {
    for (int col=0; col<yPixels1; ++col) {
      int index = col*xPixels1+row;
      for(int ij=0;ij<nComp;ij++) {
	setArgb32Component(colorComponents[ij], argb[index],
			   (getArgb32Component(colorComponents[ij], argb1[index]) -
			    getArgb32Component(colorComponents[ij], argb2[index])) );
      }
    }
  }

  return image;
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
  
  vector<TASImage> hist_image;
  hist_image.reserve(frameCount);
  
  for(int fcnt = 0; fcnt<frameCount; fcnt++) {

    Ssiz_t lastindex = filelist[fcnt].Last('.'); 
    TString rawname = filelist[fcnt](0, lastindex);
    cout<<" rawname "<<rawname<<endl;
 
    TString filePath = dir + filelist[fcnt];
    TASImage image(filePath);
    hist_image.push_back(image);

    auto returnHisto = getImage(image);

    TH2S* histos[nComp];
    for(int ij=0;ij<nComp;ij++) {
      histos[ij] = getColorHisto(returnHisto, ij);
      TString name = rawname + "_" + colorComponents[ij];
      histos[ij]->SetNameTitle(rawname,rawname);
    }

    if(fcnt>0) {

      auto returnHisto1 = getImage(hist_image[hist_image.size()-2]);

      TH2S* histos1[nComp];
      for(int ij=0;ij<nComp;ij++) {
	histos1[ij] = getColorHisto(returnHisto1, ij);
	TString name = rawname + "_" + colorComponents[ij] + "_new";
	histos1[ij]->SetNameTitle(rawname,rawname);
      }

      for(int ij=0;ij<nComp;ij++) {
	UInt_t h_yPixels = histos1[ij]->GetNbinsY();
	UInt_t h_xPixels = histos1[ij]->GetNbinsX();
	for (int row=0; row<h_xPixels; ++row) {
	  for (int col=0; col<h_yPixels; ++col) {
	    Int_t tmpval = histos1[ij]->GetBinContent(row+1, col+1) -
	      histos[ij]->GetBinContent(row+1, col+1);
	    histos1[ij]->SetBinContent(row+1, col+1, abs(tmpval));
	  }
	}

      } // for(int ij=0;ij<nComp;ij++) {

      UInt_t h_yPixels = histos1[0]->GetNbinsY();
      UInt_t h_xPixels = histos1[0]->GetNbinsX();
        
      TASImage image1(h_xPixels, h_yPixels);
      int status = setImage(std::make_tuple(histos1[0], histos1[1], histos1[2], histos1[3]), image1);

      TString tmpFilePath = dir + rawname + "_new.jpg";
      image1.WriteImage(tmpFilePath);

      for(int ij=0;ij<nComp;ij++) {
	delete histos1[ij];
      }
      
    } // if(fcnt>0) {

    for(int ij=0;ij<nComp;ij++) {
      delete histos[ij];
    }
  } // for(int fcnt = 0; fcnt<frameCount; fcnt++) {
  
}
