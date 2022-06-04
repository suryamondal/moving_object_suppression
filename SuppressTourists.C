
const int      debugLevel  = 0;
const Byte_t   limit       = 20;
const int      maxVecLim   = 10;
const int      nComp       = 4;
TString        colorComponents[nComp] = {'A','R','G','B'};


typedef std::tuple<TH2S*,TH2S*,TH2S*,TH2S*> argbData;


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
  Byte_t outval =  (Argb >> (whichPart * 8)) & 0xFF;
  if(debugLevel == 1) {
    std::cout<<" color get "<<color
	     <<" "<<std::bitset<32>(Argb)
	     <<" "<<std::bitset<8>(outval)<<std::endl;}
  return outval;
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
  if(debugLevel == 1) {
    std::cout<<" color set "<<color
	     <<" "<<std::bitset<8>(comp)<<std::endl;}
  
  /** setting the color byte to zero */
  UInt_t tArgb = 0xFF;
  tArgb <<= whichPart * 8;
  tArgb ^= 0xFFFFFFFF;
  Argb &= tArgb;
  if(debugLevel == 1) {
    std::cout<<"   set zero "<<std::bitset<32>(Argb)<<std::endl;}

  /** setting the color byte to comp */
  tArgb = comp;
  tArgb <<= whichPart * 8;
  Argb += tArgb;
  if(debugLevel == 1) {
    std::cout<<"   set byte "<<std::bitset<32>(Argb)<<std::endl;}
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

argbData getImage(TASImage &image) {
  
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
  image.EndPaint();
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
  image.EndPaint();
  return 0;
}

int getDifference(TASImage &image,
		  TASImage &image1,
		  TASImage &image2) {
  /** return = image1 - image2 */

  UInt_t yPixels  = image.GetHeight();
  UInt_t xPixels  = image.GetWidth();

  UInt_t yPixels1 = image1.GetHeight();
  UInt_t xPixels1 = image1.GetWidth();

  UInt_t yPixels2 = image2.GetHeight();
  UInt_t xPixels2 = image2.GetWidth();

  if((yPixels1 != yPixels2) ||
     (xPixels1 != xPixels2) ||
     (yPixels  != yPixels1) ||
     (xPixels  != xPixels1) ) {return 1;}
  
  UInt_t *argb1   = image1.GetArgbArray();
  UInt_t *argb2   = image2.GetArgbArray();
  UInt_t *argb    = image.GetArgbArray();
  
  for (int row=0; row<xPixels1; ++row) {
    for (int col=0; col<yPixels1; ++col) {

      int index = col*xPixels1+row;
      UInt_t tArgb32 = argb[index];
      for(int ij=0;ij<nComp;ij++) {
	Byte_t tmpval1 = getArgb32Component(colorComponents[ij],
					    argb1[index]);
	Byte_t tmpval2 = getArgb32Component(colorComponents[ij],
					    argb2[index]);
	Byte_t tmpval = (tmpval1 < tmpval2) ? (tmpval2 - tmpval1) : (tmpval1 - tmpval2);
	// if(colorComponents[ij]=="A") {tmpval = 0xFF;}
	if(debugLevel == 1) {
	  std::cout<<" tmpval "<<std::bitset<32>(tmpval)<<std::endl;}
	setArgb32Component(colorComponents[ij], tArgb32, tmpval);
	if(debugLevel == 1) {
	  std::cout<<" tArgb32 "<<std::bitset<32>(tArgb32)<<std::endl;}
      }	// for(int ij=0;ij<nComp;ij++) {
      argb[index] = tArgb32;
      if(debugLevel == 1) {
	std::cout<<" argb[index] "<<std::bitset<32>(argb[index])<<std::endl;}
    }
  }
  image.EndPaint();
  image1.EndPaint();
  image2.EndPaint();
  return 0;
}

int setDiffNull(TASImage &image,
		TASImage &image1,
		const Byte_t &lim) {

  UInt_t yPixels  = image.GetHeight();
  UInt_t xPixels  = image.GetWidth();
  
  UInt_t yPixels1 = image1.GetHeight();
  UInt_t xPixels1 = image1.GetWidth();

  if((yPixels != yPixels1) ||
     (xPixels != xPixels1) ) {return 1;}

  UInt_t *argb    = image.GetArgbArray();
  UInt_t *argb1   = image1.GetArgbArray();

  for (int row=0; row<xPixels1; ++row) {
    for (int col=0; col<yPixels1; ++col) {
      int index = col*xPixels1+row;
      // std::cout<<" argb1[index] "<<std::bitset<32>(argb1[index])<<std::endl;
      bool isSetNull = 0;
      for(int ij=1;ij<nComp;ij++) {
	Byte_t tmpval1 = getArgb32Component(colorComponents[ij],
					    argb1[index]);
	// std::cout<<" color "<<colorComponents[ij]<<" lim "<<int(lim)<<" tmpval1 "<<int(tmpval1)<<std::endl;
	if(tmpval1 > lim) {isSetNull = true; break;}
      }
      // std::cout<<" isSetNull "<<isSetNull<<std::endl;
      if(isSetNull) {
	argb[index] = 0xFFFFFFFF;
      }
      // std::cout<<" argb[index] "<<std::bitset<32>(argb[index])<<std::endl;
    }
  }
  image.EndPaint();
  image1.EndPaint();
  return 0;
}


void fillNull(TASImage &image,
	      vector<TASImage> &hist_image) {

  UInt_t yPixels  = image.GetHeight();
  UInt_t xPixels  = image.GetWidth();
  UInt_t *argb    = image.GetArgbArray();

  for (int row=0; row<xPixels; ++row) {
    for (int col=0; col<yPixels; ++col) {
      int index = col*xPixels+row;
      if(argb[index] == 0xFFFFFFFF || argb[index] == 0) {
	// std::cout<<" 0 argb[index] "<<std::bitset<32>(argb[index])<<std::endl;
	for(int ij=hist_image.size()-1;ij>=0;ij--) {
	  // TASImage image1 = hist_image[ij];
	  // UInt_t *argb1    = image1.GetArgbArray();
	  UInt_t *argb1    = hist_image[ij].GetArgbArray();
	  // std::cout<<" 0 argb1[index] "<<std::bitset<32>(argb1[index])<<" "<<ij<<std::endl;
	  if(argb1[index] != argb[index]) {
	    argb[index] = argb1[index];
	    // std::cout<<" 1 argb[index] "<<std::bitset<32>(argb[index])<<std::endl;
	    break;
	  } // if(argb1[index] != 0xFFFFFFFF) {
	}
      }	// if(argb[index] == 0xFFFFFFFF) {
    }
  }
  image.EndPaint();
}

  

void SuppressObjects(const TString &dir,
		     const TString &tdir,
		     const TString &log) {
  
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
  
  vector<TASImage> hist_image, hist_image_diff, hist_image_null;
  hist_image.reserve(frameCount);
  hist_image_diff.reserve(frameCount);
  hist_image_null.reserve(frameCount);

  if(int(hist_image.size())>maxVecLim) {
    hist_image.erase(hist_image.begin());}
  if(int(hist_image_diff.size())>maxVecLim) {
    hist_image_diff.erase(hist_image_diff.begin());}
  if(int(hist_image_null.size())>maxVecLim) {
    hist_image_null.erase(hist_image_null.begin());}
  
  for(int fcnt = 0; fcnt<frameCount; fcnt++) {

    Ssiz_t lastindex = filelist[fcnt].Last('.'); 
    TString rawname = filelist[fcnt](0, lastindex);
    cout<<" rawname "<<rawname<<endl;
 
    TString filePath = dir + filelist[fcnt];
    TASImage image(filePath);
    hist_image.push_back(image);

    if(fcnt>0) {

      UInt_t yPixels = image.GetHeight();
      UInt_t xPixels = image.GetWidth();

      TASImage image1(xPixels, yPixels);
      int status = getDifference(image1,
				 image,
				 hist_image[hist_image.size()-2]);

      if(debugLevel == 2) {
	UInt_t *argb   = image1.GetArgbArray();
	for (int row=0; row<xPixels; ++row) {
	  for (int col=0; col<yPixels; ++col) {
	    int index = col*xPixels+row;
      	    std::cout<<" argb[index] "<<std::bitset<32>(argb[index])<<std::endl;
	  }
	}
      }

      // TString tmpFilePath = tdir + rawname + "_diff.jpg";
      // image1.WriteImage(tmpFilePath);

      TASImage image2(filePath);
      status = setDiffNull(image2, image1, limit);
      hist_image_diff.push_back(image2);

      if(debugLevel == 3 && int(hist_image_diff.size())) {
	UInt_t *argb1   = hist_image_diff.back().GetArgbArray();
	for (int row=0; row<xPixels; ++row) {
	  for (int col=0; col<yPixels; ++col) {
	    int index = col*xPixels+row;
      	    std::cout<<" argb1[index] "<<std::bitset<32>(argb1[index])<<std::endl;
	  }
	}
      }

      fillNull(image2, hist_image_diff);
      image2.EndPaint();
      hist_image_null.push_back(image2);

      TString tmpFilePath1 = tdir + rawname + "_null.jpg";
      image2.WriteImage(tmpFilePath1);

    } // if(fcnt>0) {
  } // for(int fcnt = 0; fcnt<frameCount; fcnt++) {
  
}
