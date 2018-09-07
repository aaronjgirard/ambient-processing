/* 
This program reads in a binary vector (axis1=windows) and an auxilary 3D volume of data windows (axis1=time axis2=recievers axis3=windows).
The output is a new data volume where each windows that should be zero according to the vector is removed, outputting a data-selected volume.
*/ 

/*
  Copyright (C) 2012 The University of Western Australia
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <complex.h>
#include <rsf.h>


int main(int argc, char* argv[])
{
  
  int i,j,k,n1x,count,count2;
  int nx,nw,nt;
  float dx,dt,dw;
  sf_axis ax,aw,at,anew;
  /* Specify type and dimensionality of array */
  float ***sdata=NULL;
  float ***data=NULL;
  float *mask=NULL;
  /* Specify the file types - what I want read in */
  sf_file Fin=NULL;  /* input  */
  sf_file Fdata=NULL; /* input */
  sf_file Fout=NULL; /* output */
  
  /* parse info from command */
  sf_init(argc,argv);
  Fin = sf_input("in");         sf_settype(Fin,SF_FLOAT);
  Fdata = sf_input("data");	sf_settype(Fdata,SF_FLOAT);
  Fout = sf_output("out");      sf_settype(Fout,SF_FLOAT);

  if(!sf_histint(Fin,"n1",&n1x)) sf_error("No n1= in input");
  if(!sf_histint(Fdata,"n1",&n1x)) sf_error("No n1= in data");

  /* aw=windows axis
     ax=receivers axis
     at=time axis
     nomega=frequency axis */
  aw = sf_iaxa(Fdata,3);   sf_setlabel(aw,"window");
  ax = sf_iaxa(Fdata,2);   sf_setlabel(ax,"x"); 
  at = sf_iaxa(Fdata,1);   sf_setlabel(at,"time");   

  nx = sf_n(ax);   dx = sf_d(ax);
  nt = sf_n(at);   dt = sf_d(at);
  nw = sf_n(aw);   dw = sf_d(aw);
  fprintf(stderr," number of time samples: %d \n number of receivers: %d \n number of windows: %d \n",nt,nx,nw);

  //sf_oaxa(Fout,aw,3);
  sf_oaxa(Fout,ax,2);
  sf_oaxa(Fout,at,1);

  data  = sf_floatalloc3(nt,nx,nw);
  //sdata = sf_floatalloc3(nt,nx,nw);
  mask  = sf_floatalloc(nw);

  sf_floatread(data[0][0],nt*nx*nw,Fdata);
  sf_floatread(mask,nw,Fin);

  count=0;
  for(i=0; i<nw; i++){
    if(mask[i] != 0){
      count++;
    }
  }
  fprintf(stderr,"\nnew window total: %d  \n\n",count);

  /*------------------------------------------------------------*/
  /*
   *   MAIN LOOP
   */
  /*------------------------------------------------------------*/
  if(count < 1) {
    sf_warning("NO WINDOWS TO OUTPUT. \n");
    /* MUST INCLUDE A FILE OF ZEROS TO OUTPUT
            SIZE MUST BE nt*nx*1          */
    sdata = sf_floatalloc3(nt,nx,1);
    anew = sf_maxa(1,0,1);
    sf_oaxa(Fout,anew,3);
    for(j=0; j<nx; j++) { /* receiver loop */
      for(k=0; k<nt; k++) { /* time loop */
        sdata[0][j][k] = 0.0;
      }
    }
    sf_floatwrite(sdata[0][0],nt*nx*1,Fout);
    sf_warning("2D VOLUME OF ZEROS WRITTEN \n              ENDING HERE");
  } else {
    sdata = sf_floatalloc3(nt,nx,count);
    anew = sf_maxa(count,0,1);
    sf_oaxa(Fout,anew,3);

    sf_warning("START WINDOW SELECTION");
    count2=0;

    for(i=0; i<nw; i++) { /* window loop */
      //fprintf(stderr,"window = %d \n",i);
      if(mask[i] != 0) {
        //fprintf(stderr,"good windows = %d \n",count2);
        for(j=0; j<nx; j++) { /* receiver loop */
          //fprintf(stderr,"receiver # %d \n",j);
          for(k=0; k<nt; k++) { /* time loop */
            //fprintf(stderr,"timesamp = %d \n",k);
            sdata[count2][j][k] = data[i][j][k];
          }
        }
        count2++;
      }
    }

    fprintf(stderr,"\n");
    sf_warning("WRITING OUT 3D VOLUME \n");
    sf_floatwrite(sdata[0][0],nt*nx*count,Fout);
    sf_warning("3D VOLUME WRITTEN \n");
  }

  free(data);
  free(sdata);
  free(mask);

  return 0;
}

