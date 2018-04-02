

Double_t GPulseShape(Double_t *x,Double_t *par){

  Double_t val=0;

  if(x[0]>0)
    val = par[2]*(exp(-x[0]/par[1])- exp(-x[0]/par[0]));
  return val;
}


TF1* Generate_SPE(Double_t &risetime,Double_t &falltime,Double_t scale=1){


  TF1 *f_spe= new TF1("f_pulse_shape",GPulseShape,-10,500,3);

  f_spe->SetNpx(5000);

  f_spe->SetParameters(risetime,falltime, scale);

  return f_spe;


}


void SiPM_SPE(){

  Double_t risetime, falltime;
  std::cout<<"Insert Rise and fall times in ns: "<<std::flush;
  std::cin>>risetime>>falltime;

  TF1 *fun=Generate_SPE(risetime,falltime);

  TCanvas *c1;
  fun->Draw("");

  double x = -10.0;
  int nPX;
  std::cout << "How many points to print out? Enter 0 for no printing\nFunction Range is [-10:510] : " << std::flush;
  std::cin >> nPX;

  for (int i=0; i<nPX; i++){
  x += 510.0/nPX;
  printf("%d %f %f\n", i, x, fun->Eval(x));

  }
}
