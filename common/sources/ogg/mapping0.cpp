/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: channel mapping 0 implementation
 last mod: $Id: mapping0.c,v 1.60 2003/09/01 23:05:49 xiphmont Exp $

 ********************************************************************/

#include "ogg.h"
#include "codec.h"
#include "codec_internal.h"
#include "codebook.h"
#include "window.h"
#include "registry.h"
#include "psy.h"
#include "misc.h"

/* simplistic, wasteful way of doing this (unique lookup for each
   mode/submapping); there should be a central repository for
   identical lookups.  That will require minor work, so I'm putting it
   off as low priority.

   Why a lookup for each backend in a given mode?  Because the
   blocksize is set by the mode, and low backend lookups may require
   parameters from other areas of the mode/mapping */

static void mapping0_free_info(vorbis_info_mapping *i){
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)i;
  if(info){
    my_memset(info,0,sizeof(*info));
    _ogg_free(info);
  }
}

static int ilog(unsigned int v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static void mapping0_pack(vorbis_info *vi,vorbis_info_mapping *vm,
			  oggpack_buffer *opb){
  int i;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)vm;

  /* another 'we meant to do it this way' hack...  up to beta 4, we
     packed 4 binary zeros here to signify one submapping in use.  We
     now redefine that to mean four bitflags that indicate use of
     deeper features; bit0:submappings, bit1:coupling,
     bit2,3:reserved. This is backward compatable with all actual uses
     of the beta code. */

  if(info->submaps>1){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->submaps-1,4);
  }else
    oggpack_write(opb,0,1);

  if(info->coupling_steps>0){
    oggpack_write(opb,1,1);
    oggpack_write(opb,info->coupling_steps-1,8);
    
    for(i=0;i<info->coupling_steps;i++){
      oggpack_write(opb,info->coupling_mag[i],ilog(vi->channels));
      oggpack_write(opb,info->coupling_ang[i],ilog(vi->channels));
    }
  }else
    oggpack_write(opb,0,1);
  
  oggpack_write(opb,0,2); /* 2,3:reserved */

  /* we don't write the channel submappings if we only have one... */
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++)
      oggpack_write(opb,info->chmuxlist[i],4);
  }
  for(i=0;i<info->submaps;i++){
    oggpack_write(opb,0,8); /* time submap unused */
    oggpack_write(opb,info->floorsubmap[i],8);
    oggpack_write(opb,info->residuesubmap[i],8);
  }
}

/* also responsible for range checking */
static vorbis_info_mapping *mapping0_unpack(vorbis_info *vi,oggpack_buffer *opb){
  int i;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)_ogg_calloc(1,sizeof(*info));
  codec_setup_info     *ci=(codec_setup_info     *)vi->codec_setup;
  my_memset(info,0,sizeof(*info));

  if(oggpack_read(opb,1))
    info->submaps=oggpack_read(opb,4)+1;
  else
    info->submaps=1;

  if(oggpack_read(opb,1)){
    info->coupling_steps=oggpack_read(opb,8)+1;

    for(i=0;i<info->coupling_steps;i++){
      int testM=info->coupling_mag[i]=oggpack_read(opb,ilog(vi->channels));
      int testA=info->coupling_ang[i]=oggpack_read(opb,ilog(vi->channels));

      if(testM<0 || 
	 testA<0 || 
	 testM==testA || 
	 testM>=vi->channels ||
	 testA>=vi->channels) goto err_out;
    }

  }

  if(oggpack_read(opb,2)>0)goto err_out; /* 2,3:reserved */
    
  if(info->submaps>1){
    for(i=0;i<vi->channels;i++){
      info->chmuxlist[i]=oggpack_read(opb,4);
      if(info->chmuxlist[i]>=info->submaps)goto err_out;
    }
  }
  for(i=0;i<info->submaps;i++){
    oggpack_read(opb,8); /* time submap unused */
    info->floorsubmap[i]=oggpack_read(opb,8);
    if(info->floorsubmap[i]>=ci->floors)goto err_out;
    info->residuesubmap[i]=oggpack_read(opb,8);
    if(info->residuesubmap[i]>=ci->residues)goto err_out;
  }

  return info;

 err_out:
  mapping0_free_info(info);
  return(NULL);
}

#include "os.h"
#include "lpc.h"
#include "lsp.h"
#include "envelope.h"
#include "mdct.h"
#include "psy.h"
#include "scales.h"



typedef struct {
  int sorted_index[VIF_POSIT+2];
  int forward_index[VIF_POSIT+2];
  int reverse_index[VIF_POSIT+2];
  
  int hineighbor[VIF_POSIT];
  int loneighbor[VIF_POSIT];
  int posts;

  int n;
  int quant_q;
  vorbis_info_floor1 *vi;

  long phrasebits;
  long postbits;
  long frames;
} vorbis_look_floor1;

extern int *floor1_fit(vorbis_block *vb,vorbis_look_floor1 *look,
		       const float *logmdct,   /* in */
		       const float *logmask);



extern int *floor1_interpolate_fit(vorbis_block *vb,vorbis_look_floor1 *look,
				   int *A,int *B,
				   int del);



extern "C" int floor1_encode(vorbis_block *vb,vorbis_look_floor1 *look,
			 int *post,int *ilogmask);


static int mapping0_forward(vorbis_block *vb){
  vorbis_dsp_state      *vd=vb->vd;
  vorbis_info           *vi=vd->vi;
  codec_setup_info      *ci=(codec_setup_info      *)vi->codec_setup;
  private_state         *b=(private_state         *)vb->vd->backend_state;
  vorbis_block_internal *vbi=(vorbis_block_internal *)vb->internal;
  int                    n=vb->pcmend;
  int i,j,k;
	int for_free=0;

  int    *nonzero    = (int*)my_malloc(sizeof(*nonzero)*vi->channels);
  float  **gmdct     = (float**)_vorbis_block_alloc(vb,vi->channels*sizeof(*gmdct));
  int    **ilogmaskch= (int**)_vorbis_block_alloc(vb,vi->channels*sizeof(*ilogmaskch));
  int ***floor_posts = (int***)_vorbis_block_alloc(vb,vi->channels*sizeof(*floor_posts));
  
  float global_ampmax=vbi->ampmax;
  float *local_ampmax=(float*)my_malloc(sizeof(*local_ampmax)*vi->channels);
  int blocktype=vbi->blocktype;

  int modenumber=vb->W;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)ci->map_param[modenumber];
  vorbis_look_psy *psy_look=
    b->psy+blocktype+(vb->W?2:0);

  vb->mode=modenumber;

  for(i=0;i<vi->channels;i++){
    float scale=4.f/n;
    float scale_dB;

    float *pcm     =vb->pcm[i]; 
    float *logfft  =pcm;

    gmdct[i]=(float*)_vorbis_block_alloc(vb,n/2*sizeof(**gmdct));

    scale_dB=todB(&scale);

  
    /* window the PCM data */
    _vorbis_apply_window(pcm,b->window,ci->blocksizes,vb->lW,vb->W,vb->nW);


    /* transform the PCM data */
    /* only MDCT right now.... */
    mdct_forward((mdct_lookup*)b->transform[vb->W][0],pcm,gmdct[i]);
    
    /* FFT yields more accurate tonal estimation (not phase sensitive) */
    drft_forward(&b->fft_look[vb->W],pcm);
    logfft[0]=scale_dB+todB(pcm);
    local_ampmax[i]=logfft[0];
    for(j=1;j<n-1;j+=2){
      float temp=pcm[j]*pcm[j]+pcm[j+1]*pcm[j+1];
      temp=logfft[(j+1)>>1]=scale_dB+.5f*todB(&temp);
      if(temp>local_ampmax[i])local_ampmax[i]=temp;
    }

    if(local_ampmax[i]>0.f)local_ampmax[i]=0.f;
    if(local_ampmax[i]>global_ampmax)global_ampmax=local_ampmax[i];


  }
  
  {
    float   *noise        = (float*)_vorbis_block_alloc(vb,n/2*sizeof(*noise));
    float   *tone         = (float*)_vorbis_block_alloc(vb,n/2*sizeof(*tone));
    
    for(i=0;i<vi->channels;i++){
      /* the encoder setup assumes that all the modes used by any
	 specific bitrate tweaking use the same floor */
      
      int submap=info->chmuxlist[i];
      
      /* the following makes things clearer to *me* anyway */
      float *mdct    =gmdct[i];
      float *logfft  =vb->pcm[i];
      
      float *logmdct =logfft+n/2;
      float *logmask =logfft;

      vb->mode=modenumber;

      floor_posts[i]=(int**)_vorbis_block_alloc(vb,PACKETBLOBS*sizeof(**floor_posts));
      my_memset(floor_posts[i],0,sizeof(**floor_posts)*PACKETBLOBS);
      
      for(j=0;j<n/2;j++)
	logmdct[j]=todB(mdct+j);

      
      /* first step; noise masking.  Not only does 'noise masking'
         give us curves from which we can decide how much resolution
         to give noise parts of the spectrum, it also implicitly hands
         us a tonality estimate (the larger the value in the
         'noise_depth' vector, the more tonal that area is) */

      _vp_noisemask(psy_look,
		    logmdct,
		    noise); /* noise does not have by-frequency offset
                               bias applied yet */

      /* second step: 'all the other crap'; all the stuff that isn't
         computed/fit for bitrate management goes in the second psy
         vector.  This includes tone masking, peak limiting and ATH */

      _vp_tonemask(psy_look,
		   logfft,
		   tone,
		   global_ampmax,
		   local_ampmax[i]);

      /* third step; we offset the noise vectors, overlay tone
	 masking.  We then do a floor1-specific line fit.  If we're
	 performing bitrate management, the line fit is performed
	 multiple times for up/down tweakage on demand. */
      
      _vp_offset_and_mix(psy_look,
			 noise,
			 tone,
			 1,
			 logmask);

      /* this algorithm is hardwired to floor 1 for now; abort out if
         we're *not* floor1.  This won't happen unless someone has
         broken the encode setup lib.  Guard it anyway. */
      if(ci->floor_type[info->floorsubmap[submap]]!=1)
	  {
		  	my_free(nonzero);
			my_free(local_ampmax);
			  return(-1);
	  }

      floor_posts[i][PACKETBLOBS/2]=
	floor1_fit(vb,(vorbis_look_floor1*)b->flr[info->floorsubmap[submap]],
		   logmdct,
		   logmask);
      
      /* are we managing bitrate?  If so, perform two more fits for
         later rate tweaking (fits represent hi/lo) */
      if(vorbis_bitrate_managed(vb) && floor_posts[i][PACKETBLOBS/2]){
	/* higher rate by way of lower noise curve */

	_vp_offset_and_mix(psy_look,
			   noise,
			   tone,
			   2,
			   logmask);

	
	floor_posts[i][PACKETBLOBS-1]=
	  floor1_fit(vb,(vorbis_look_floor1*)b->flr[info->floorsubmap[submap]],
		     logmdct,
		     logmask);
      
	/* lower rate by way of higher noise curve */
	_vp_offset_and_mix(psy_look,
			   noise,
			   tone,
			   0,
			   logmask);


	floor_posts[i][0]=
	  floor1_fit(vb,(vorbis_look_floor1*)b->flr[info->floorsubmap[submap]],

		     logmdct,
		     logmask);
	
	/* we also interpolate a range of intermediate curves for
           intermediate rates */
	for(k=1;k<PACKETBLOBS/2;k++)
	  floor_posts[i][k]=
	    floor1_interpolate_fit(vb,(vorbis_look_floor1*)b->flr[info->floorsubmap[submap]],
				   floor_posts[i][0],
				   floor_posts[i][PACKETBLOBS/2],
				   k*65536/(PACKETBLOBS/2));
	for(k=PACKETBLOBS/2+1;k<PACKETBLOBS-1;k++)
	  floor_posts[i][k]=
	    floor1_interpolate_fit(vb,(vorbis_look_floor1*)b->flr[info->floorsubmap[submap]],
				   floor_posts[i][PACKETBLOBS/2],
				   floor_posts[i][PACKETBLOBS-1],
				   (k-PACKETBLOBS/2)*65536/(PACKETBLOBS/2));
      }
    }
  }
  vbi->ampmax=global_ampmax;

  /*
    the next phases are performed once for vbr-only and PACKETBLOB
    times for bitrate managed modes.
    
    1) encode actual mode being used
    2) encode the floor for each channel, compute coded mask curve/res
    3) normalize and couple.
    4) encode residue
    5) save packet bytes to the packetblob vector
    
  */

  /* iterate over the many masking curve fits we've created */

    float **res_bundle=(float**)my_malloc(sizeof(*res_bundle)*vi->channels);
    float **couple_bundle=(float**)my_malloc(sizeof(*couple_bundle)*vi->channels);
    int *zerobundle=(int*)my_malloc(sizeof(*zerobundle)*vi->channels);
    int **sortindex=(int**)my_malloc(sizeof(*sortindex)*vi->channels);
    float **mag_memo;
    int **mag_sort;

    if(info->coupling_steps)
	{
      mag_memo=_vp_quantize_couple_memo(vb,
					&ci->psy_g_param,
					psy_look,
					info,
					gmdct);    
      
      mag_sort=_vp_quantize_couple_sort(vb,
					psy_look,
					info,
					mag_memo);    
    }

    my_memset(sortindex,0,sizeof(*sortindex)*vi->channels);
    
	if(psy_look->vi->normal_channel_p)
	{
		for_free=vi->channels;
      for(i=0;i<vi->channels;i++)
	  {
			float *mdct    =gmdct[i];
			sortindex[i]=(int*)my_malloc(sizeof(**sortindex)*n/2);
			_vp_noise_normalize_sort(psy_look,mdct,sortindex[i]);
      }
    }

    for(k=(vorbis_bitrate_managed(vb)?0:PACKETBLOBS/2);
	k<=(vorbis_bitrate_managed(vb)?PACKETBLOBS-1:PACKETBLOBS/2);
	k++){

      /* start out our new packet blob with packet type and mode */
      /* Encode the packet type */
      oggpack_write(&vb->opb,0,1);
      /* Encode the modenumber */
      /* Encode frame mode, pre,post windowsize, then dispatch */
      oggpack_write(&vb->opb,modenumber,b->modebits);
      if(vb->W){
	oggpack_write(&vb->opb,vb->lW,1);
	oggpack_write(&vb->opb,vb->nW,1);
      }

      /* encode floor, compute masking curve, sep out residue */
      for(i=0;i<vi->channels;i++){
	int submap=info->chmuxlist[i];
	float *mdct    =gmdct[i];
	float *res     =vb->pcm[i];
	int   *ilogmask=ilogmaskch[i]=(int*)
	  _vorbis_block_alloc(vb,n/2*sizeof(**gmdct));
      
	nonzero[i]=floor1_encode(vb,(vorbis_look_floor1*)b->flr[info->floorsubmap[submap]],
				 floor_posts[i][k],
				 ilogmask);
	_vp_remove_floor(psy_look,
			 mdct,
			 ilogmask,
			 res,
			 ci->psy_g_param.sliding_lowpass[vb->W][k]);

	_vp_noise_normalize(psy_look,res,res+n/2,sortindex[i]);

	
      }
      
      /* our iteration is now based on masking curve, not prequant and
	 coupling.  Only one prequant/coupling step */
      
      /* quantize/couple */
      /* incomplete implementation that assumes the tree is all depth
         one, or no tree at all */
      if(info->coupling_steps){
	_vp_couple(k,
		   &ci->psy_g_param,
		   psy_look,
		   info,
		   vb->pcm,
		   mag_memo,
		   mag_sort,
		   ilogmaskch,
		   nonzero,
		   ci->psy_g_param.sliding_lowpass[vb->W][k]);
      }
      
      /* classify and encode by submap */
      for(i=0;i<info->submaps;i++){
	int ch_in_bundle=0;
	long **classifications;
	int resnum=info->residuesubmap[i];

	for(j=0;j<vi->channels;j++){
	  if(info->chmuxlist[j]==i){
	    zerobundle[ch_in_bundle]=0;
	    if(nonzero[j])zerobundle[ch_in_bundle]=1;
	    res_bundle[ch_in_bundle]=vb->pcm[j];
	    couple_bundle[ch_in_bundle++]=vb->pcm[j]+n/2;
	  }
	}
	
	classifications=_residue_P[ci->residue_type[resnum]]->
	  Class(vb,b->residue[resnum],couple_bundle,zerobundle,ch_in_bundle);
	
	_residue_P[ci->residue_type[resnum]]->
	  forward(vb,b->residue[resnum],
		  couple_bundle,NULL,zerobundle,ch_in_bundle,classifications);
      }
      
      /* ok, done encoding.  Mark this protopacket and prepare next. */
      oggpack_writealign(&vb->opb);
      vbi->packetblob_markers[k]=oggpack_bytes(&vb->opb);
      
    }
    

	for(i=0;i<for_free;i++)
	  my_free(sortindex[i]);

	my_free(nonzero);
	my_free(local_ampmax);
	my_free(res_bundle);
	my_free(couple_bundle);
    my_free(zerobundle);
    my_free(sortindex);

  return(0);
}

static int mapping0_inverse(vorbis_block *vb,vorbis_info_mapping *l){
  vorbis_dsp_state     *vd=vb->vd;
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=(codec_setup_info     *)vi->codec_setup;
  private_state        *b=(private_state        *)vd->backend_state;
  vorbis_info_mapping0 *info=(vorbis_info_mapping0 *)l;
  int hs=ci->halfrate_flag; 

  int                   i,j;
  long                  n=vb->pcmend=ci->blocksizes[vb->W];

  float **pcmbundle=(float **)my_malloc(sizeof(*pcmbundle)*vi->channels);
  int    *zerobundle=(int*)my_malloc(sizeof(*zerobundle)*vi->channels);

  int   *nonzero  =(int*)my_malloc(sizeof(*nonzero)*vi->channels);
  void **floormemo=(void**)my_malloc(sizeof(*floormemo)*vi->channels);
  
  /* recover the spectral envelope; store it in the PCM vector for now */
  for(i=0;i<vi->channels;i++){
    int submap=info->chmuxlist[i];
    floormemo[i]=_floor_P[ci->floor_type[info->floorsubmap[submap]]]->
      inverse1(vb,b->flr[info->floorsubmap[submap]]);
    if(floormemo[i])
      nonzero[i]=1;
    else
      nonzero[i]=0;      
    my_memset(vb->pcm[i],0,sizeof(*vb->pcm[i])*n/2);
  }

  /* channel coupling can 'dirty' the nonzero listing */
  for(i=0;i<info->coupling_steps;i++){
    if(nonzero[info->coupling_mag[i]] ||
       nonzero[info->coupling_ang[i]]){
      nonzero[info->coupling_mag[i]]=1; 
      nonzero[info->coupling_ang[i]]=1; 
    }
  }

  /* recover the residue into our working vectors */
  for(i=0;i<info->submaps;i++){
    int ch_in_bundle=0;
    for(j=0;j<vi->channels;j++){
      if(info->chmuxlist[j]==i){
	if(nonzero[j])
	  zerobundle[ch_in_bundle]=1;
	else
	  zerobundle[ch_in_bundle]=0;
	pcmbundle[ch_in_bundle++]=vb->pcm[j];
      }
    }

    _residue_P[ci->residue_type[info->residuesubmap[i]]]->
      inverse(vb,b->residue[info->residuesubmap[i]],
	      pcmbundle,zerobundle,ch_in_bundle);
  }

  /* channel coupling */
  for(i=info->coupling_steps-1;i>=0;i--){
    float *pcmM=vb->pcm[info->coupling_mag[i]];
    float *pcmA=vb->pcm[info->coupling_ang[i]];

    for(j=0;j<n/2;j++){
      float mag=pcmM[j];
      float ang=pcmA[j];

      if(mag>0)
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag-ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag+ang;
	}
      else
	if(ang>0){
	  pcmM[j]=mag;
	  pcmA[j]=mag+ang;
	}else{
	  pcmA[j]=mag;
	  pcmM[j]=mag-ang;
	}
    }
  }

  /* compute and apply spectral envelope */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    int submap=info->chmuxlist[i];
    _floor_P[ci->floor_type[info->floorsubmap[submap]]]->
      inverse2(vb,b->flr[info->floorsubmap[submap]],
	       floormemo[i],pcm);
  }

  /* transform the PCM data; takes PCM vector, vb; modifies PCM vector */
  /* only MDCT right now.... */
  for(i=0;i<vi->channels;i++){
    float *pcm=vb->pcm[i];
    mdct_backward((mdct_lookup*)b->transform[vb->W][0],pcm,pcm);
  }


	my_free(pcmbundle);
	my_free(zerobundle);
	my_free(nonzero);
	my_free(floormemo);


  /* all done! */
  return(0);
}








/* export hooks */
vorbis_func_mapping mapping0_exportbundle={
  &mapping0_pack,
  &mapping0_unpack,
  &mapping0_free_info,
  &mapping0_forward,
  &mapping0_inverse
};

