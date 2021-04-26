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

 function: *unnormalized* fft transform
 last mod: $Id: smallft.c,v 1.17 2002/07/11 06:40:50 xiphmont Exp $

 ********************************************************************/

/* FFT implementation from OggSquish, minus cosine transforms,
 * minus all but radix 2/4 case.  In Vorbis we only need this
 * cut-down version.
 *
 * To do more than just power-of-two sized vectors, see the full
 * version I wrote for NetLib.
 *
 * Note that the packing is a little strange; rather than the FFT r/i
 * packing following R_0, I_n, R_1, I_1, R_2, I_2 ... R_n-1, I_n-1,
 * it follows R_0, R_1, I_1, R_2, I_2 ... R_n-1, I_n-1, I_n like the
 * FORTRAN version
 */

#include "smallft.h"
#include "misc.h"

static void drfti1(int n, float *wa, int *ifac)
{
}

static void fdrffti(int n, float *wsave, int *ifac){

  if (n == 1) return;
  drfti1(n, wsave+n, ifac);
}

static void dradf2(int ido,int l1,float *cc,float *ch,float *wa1){
  int i,k;
  float ti2,tr2;
  int t0,t1,t2,t3,t4,t5,t6;

  t1=0;
  t0=(t2=l1*ido);
  t3=ido<<1;
  for(k=0;k<l1;k++){
    ch[t1<<1]=cc[t1]+cc[t2];
    ch[(t1<<1)+t3-1]=cc[t1]-cc[t2];
    t1+=ido;
    t2+=ido;
  }
    
  if(ido<2)return;
  if(ido==2)goto L105;

  t1=0;
  t2=t0;
  for(k=0;k<l1;k++){
    t3=t2;
    t4=(t1<<1)+(ido<<1);
    t5=t1;
    t6=t1+t1;
    for(i=2;i<ido;i+=2){
      t3+=2;
      t4-=2;
      t5+=2;
      t6+=2;
      tr2=wa1[i-2]*cc[t3-1]+wa1[i-1]*cc[t3];
      ti2=wa1[i-2]*cc[t3]-wa1[i-1]*cc[t3-1];
      ch[t6]=cc[t5]+ti2;
      ch[t4]=ti2-cc[t5];
      ch[t6-1]=cc[t5-1]+tr2;
      ch[t4-1]=cc[t5-1]-tr2;
    }
    t1+=ido;
    t2+=ido;
  }

  if(ido%2==1)return;

 L105:
  t3=(t2=(t1=ido)-1);
  t2+=t0;
  for(k=0;k<l1;k++){
    ch[t1]=-cc[t2];
    ch[t1-1]=cc[t3];
    t1+=ido<<1;
    t2+=ido;
    t3+=ido;
  }
}

static void dradf4(int ido,int l1,float *cc,float *ch,float *wa1,
	    float *wa2,float *wa3){
  static float hsqt2 = .70710678118654752f;
  int i,k,t0,t1,t2,t3,t4,t5,t6;
  float ci2,ci3,ci4,cr2,cr3,cr4,ti1,ti2,ti3,ti4,tr1,tr2,tr3,tr4;
  t0=l1*ido;
  
  t1=t0;
  t4=t1<<1;
  t2=t1+(t1<<1);
  t3=0;

  for(k=0;k<l1;k++){
    tr1=cc[t1]+cc[t2];
    tr2=cc[t3]+cc[t4];

    ch[t5=t3<<2]=tr1+tr2;
    ch[(ido<<2)+t5-1]=tr2-tr1;
    ch[(t5+=(ido<<1))-1]=cc[t3]-cc[t4];
    ch[t5]=cc[t2]-cc[t1];

    t1+=ido;
    t2+=ido;
    t3+=ido;
    t4+=ido;
  }

  if(ido<2)return;
  if(ido==2)goto L105;


  t1=0;
  for(k=0;k<l1;k++){
    t2=t1;
    t4=t1<<2;
    t5=(t6=ido<<1)+t4;
    for(i=2;i<ido;i+=2){
      t3=(t2+=2);
      t4+=2;
      t5-=2;

      t3+=t0;
      cr2=wa1[i-2]*cc[t3-1]+wa1[i-1]*cc[t3];
      ci2=wa1[i-2]*cc[t3]-wa1[i-1]*cc[t3-1];
      t3+=t0;
      cr3=wa2[i-2]*cc[t3-1]+wa2[i-1]*cc[t3];
      ci3=wa2[i-2]*cc[t3]-wa2[i-1]*cc[t3-1];
      t3+=t0;
      cr4=wa3[i-2]*cc[t3-1]+wa3[i-1]*cc[t3];
      ci4=wa3[i-2]*cc[t3]-wa3[i-1]*cc[t3-1];

      tr1=cr2+cr4;
      tr4=cr4-cr2;
      ti1=ci2+ci4;
      ti4=ci2-ci4;

      ti2=cc[t2]+ci3;
      ti3=cc[t2]-ci3;
      tr2=cc[t2-1]+cr3;
      tr3=cc[t2-1]-cr3;

      ch[t4-1]=tr1+tr2;
      ch[t4]=ti1+ti2;

      ch[t5-1]=tr3-ti4;
      ch[t5]=tr4-ti3;

      ch[t4+t6-1]=ti4+tr3;
      ch[t4+t6]=tr4+ti3;

      ch[t5+t6-1]=tr2-tr1;
      ch[t5+t6]=ti1-ti2;
    }
    t1+=ido;
  }
  if(ido&1)return;

 L105:
  
  t2=(t1=t0+ido-1)+(t0<<1);
  t3=ido<<2;
  t4=ido;
  t5=ido<<1;
  t6=ido;

  for(k=0;k<l1;k++){
    ti1=-hsqt2*(cc[t1]+cc[t2]);
    tr1=hsqt2*(cc[t1]-cc[t2]);

    ch[t4-1]=tr1+cc[t6-1];
    ch[t4+t5-1]=cc[t6-1]-tr1;

    ch[t4]=ti1-cc[t1+t0];
    ch[t4+t5]=ti1+cc[t1+t0];

    t1+=ido;
    t2+=ido;
    t4+=t3;
    t6+=ido;
  }
}

static void dradfg(int ido,int ip,int l1,int idl1,float *cc,float *c1,
                          float *c2,float *ch,float *ch2,float *wa)
{
}

static void drftf1(int n,float *c,float *ch,float *wa,int *ifac){
  int i,k1,l1,l2;
  int na,kh,nf;
  int ip,iw,ido,idl1,ix2,ix3;

  nf=ifac[1];
  na=1;
  l2=n;
  iw=n;

  for(k1=0;k1<nf;k1++){
    kh=nf-k1;
    ip=ifac[kh+1];
    l1=l2/ip;
    ido=n/l2;
    idl1=ido*l1;
    iw-=(ip-1)*ido;
    na=1-na;

    if(ip!=4)goto L102;

    ix2=iw+ido;
    ix3=ix2+ido;
    if(na!=0)
      dradf4(ido,l1,ch,c,wa+iw-1,wa+ix2-1,wa+ix3-1);
    else
      dradf4(ido,l1,c,ch,wa+iw-1,wa+ix2-1,wa+ix3-1);
    goto L110;

 L102:
    if(ip!=2)goto L104;
    if(na!=0)goto L103;

    dradf2(ido,l1,c,ch,wa+iw-1);
    goto L110;

  L103:
    dradf2(ido,l1,ch,c,wa+iw-1);
    goto L110;

  L104:
    if(ido==1)na=1-na;
    if(na!=0)goto L109;

    dradfg(ido,ip,l1,idl1,c,c,c,ch,ch,wa+iw-1);
    na=1;
    goto L110;

  L109:
    dradfg(ido,ip,l1,idl1,ch,ch,ch,c,c,wa+iw-1);
    na=0;

  L110:
    l2=l1;
  }

  if(na==1)return;

  for(i=0;i<n;i++)c[i]=ch[i];
}

static void dradb2(int ido,int l1,float *cc,float *ch,float *wa1){
  int i,k,t0,t1,t2,t3,t4,t5,t6;
  float ti2,tr2;

  t0=l1*ido;
  
  t1=0;
  t2=0;
  t3=(ido<<1)-1;
  for(k=0;k<l1;k++){
    ch[t1]=cc[t2]+cc[t3+t2];
    ch[t1+t0]=cc[t2]-cc[t3+t2];
    t2=(t1+=ido)<<1;
  }

  if(ido<2)return;
  if(ido==2)goto L105;

  t1=0;
  t2=0;
  for(k=0;k<l1;k++){
    t3=t1;
    t5=(t4=t2)+(ido<<1);
    t6=t0+t1;
    for(i=2;i<ido;i+=2){
      t3+=2;
      t4+=2;
      t5-=2;
      t6+=2;
      ch[t3-1]=cc[t4-1]+cc[t5-1];
      tr2=cc[t4-1]-cc[t5-1];
      ch[t3]=cc[t4]-cc[t5];
      ti2=cc[t4]+cc[t5];
      ch[t6-1]=wa1[i-2]*tr2-wa1[i-1]*ti2;
      ch[t6]=wa1[i-2]*ti2+wa1[i-1]*tr2;
    }
    t2=(t1+=ido)<<1;
  }

  if(ido%2==1)return;

L105:
  t1=ido-1;
  t2=ido-1;
  for(k=0;k<l1;k++){
    ch[t1]=cc[t2]+cc[t2];
    ch[t1+t0]=-(cc[t2+1]+cc[t2+1]);
    t1+=ido;
    t2+=ido<<1;
  }
}

static void dradb3(int ido,int l1,float *cc,float *ch,float *wa1,
                          float *wa2){
  static float taur = -.5f;
  static float taui = .8660254037844386f;
  int i,k,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
  float ci2,ci3,di2,di3,cr2,cr3,dr2,dr3,ti2,tr2;
  t0=l1*ido;

  t1=0;
  t2=t0<<1;
  t3=ido<<1;
  t4=ido+(ido<<1);
  t5=0;
  for(k=0;k<l1;k++){
    tr2=cc[t3-1]+cc[t3-1];
    cr2=cc[t5]+(taur*tr2);
    ch[t1]=cc[t5]+tr2;
    ci3=taui*(cc[t3]+cc[t3]);
    ch[t1+t0]=cr2-ci3;
    ch[t1+t2]=cr2+ci3;
    t1+=ido;
    t3+=t4;
    t5+=t4;
  }

  if(ido==1)return;

  t1=0;
  t3=ido<<1;
  for(k=0;k<l1;k++){
    t7=t1+(t1<<1);
    t6=(t5=t7+t3);
    t8=t1;
    t10=(t9=t1+t0)+t0;

    for(i=2;i<ido;i+=2){
      t5+=2;
      t6-=2;
      t7+=2;
      t8+=2;
      t9+=2;
      t10+=2;
      tr2=cc[t5-1]+cc[t6-1];
      cr2=cc[t7-1]+(taur*tr2);
      ch[t8-1]=cc[t7-1]+tr2;
      ti2=cc[t5]-cc[t6];
      ci2=cc[t7]+(taur*ti2);
      ch[t8]=cc[t7]+ti2;
      cr3=taui*(cc[t5-1]-cc[t6-1]);
      ci3=taui*(cc[t5]+cc[t6]);
      dr2=cr2-ci3;
      dr3=cr2+ci3;
      di2=ci2+cr3;
      di3=ci2-cr3;
      ch[t9-1]=wa1[i-2]*dr2-wa1[i-1]*di2;
      ch[t9]=wa1[i-2]*di2+wa1[i-1]*dr2;
      ch[t10-1]=wa2[i-2]*dr3-wa2[i-1]*di3;
      ch[t10]=wa2[i-2]*di3+wa2[i-1]*dr3;
    }
    t1+=ido;
  }
}

static void dradb4(int ido,int l1,float *cc,float *ch,float *wa1,
			  float *wa2,float *wa3){
  static float sqrt2=1.414213562373095f;
  int i,k,t0,t1,t2,t3,t4,t5,t6,t7,t8;
  float ci2,ci3,ci4,cr2,cr3,cr4,ti1,ti2,ti3,ti4,tr1,tr2,tr3,tr4;
  t0=l1*ido;
  
  t1=0;
  t2=ido<<2;
  t3=0;
  t6=ido<<1;
  for(k=0;k<l1;k++){
    t4=t3+t6;
    t5=t1;
    tr3=cc[t4-1]+cc[t4-1];
    tr4=cc[t4]+cc[t4]; 
    tr1=cc[t3]-cc[(t4+=t6)-1];
    tr2=cc[t3]+cc[t4-1];
    ch[t5]=tr2+tr3;
    ch[t5+=t0]=tr1-tr4;
    ch[t5+=t0]=tr2-tr3;
    ch[t5+=t0]=tr1+tr4;
    t1+=ido;
    t3+=t2;
  }

  if(ido<2)return;
  if(ido==2)goto L105;

  t1=0;
  for(k=0;k<l1;k++){
    t5=(t4=(t3=(t2=t1<<2)+t6))+t6;
    t7=t1;
    for(i=2;i<ido;i+=2){
      t2+=2;
      t3+=2;
      t4-=2;
      t5-=2;
      t7+=2;
      ti1=cc[t2]+cc[t5];
      ti2=cc[t2]-cc[t5];
      ti3=cc[t3]-cc[t4];
      tr4=cc[t3]+cc[t4];
      tr1=cc[t2-1]-cc[t5-1];
      tr2=cc[t2-1]+cc[t5-1];
      ti4=cc[t3-1]-cc[t4-1];
      tr3=cc[t3-1]+cc[t4-1];
      ch[t7-1]=tr2+tr3;
      cr3=tr2-tr3;
      ch[t7]=ti2+ti3;
      ci3=ti2-ti3;
      cr2=tr1-tr4;
      cr4=tr1+tr4;
      ci2=ti1+ti4;
      ci4=ti1-ti4;

      ch[(t8=t7+t0)-1]=wa1[i-2]*cr2-wa1[i-1]*ci2;
      ch[t8]=wa1[i-2]*ci2+wa1[i-1]*cr2;
      ch[(t8+=t0)-1]=wa2[i-2]*cr3-wa2[i-1]*ci3;
      ch[t8]=wa2[i-2]*ci3+wa2[i-1]*cr3;
      ch[(t8+=t0)-1]=wa3[i-2]*cr4-wa3[i-1]*ci4;
      ch[t8]=wa3[i-2]*ci4+wa3[i-1]*cr4;
    }
    t1+=ido;
  }

  if(ido%2 == 1)return;

 L105:

  t1=ido;
  t2=ido<<2;
  t3=ido-1;
  t4=ido+(ido<<1);
  for(k=0;k<l1;k++){
    t5=t3;
    ti1=cc[t1]+cc[t4];
    ti2=cc[t4]-cc[t1];
    tr1=cc[t1-1]-cc[t4-1];
    tr2=cc[t1-1]+cc[t4-1];
    ch[t5]=tr2+tr2;
    ch[t5+=t0]=sqrt2*(tr1-ti1);
    ch[t5+=t0]=ti2+ti2;
    ch[t5+=t0]=-sqrt2*(tr1+ti1);

    t3+=ido;
    t1+=t2;
    t4+=t2;
  }
}

static void dradbg(int ido,int ip,int l1,int idl1,float *cc,float *c1,
            float *c2,float *ch,float *ch2,float *wa)
{
}

static void drftb1(int n, float *c, float *ch, float *wa, int *ifac){
  int i,k1,l1,l2;
  int na;
  int nf,ip,iw,ix2,ix3,ido,idl1;

  nf=ifac[1];
  na=0;
  l1=1;
  iw=1;

  for(k1=0;k1<nf;k1++){
    ip=ifac[k1 + 2];
    l2=ip*l1;
    ido=n/l2;
    idl1=ido*l1;
    if(ip!=4)goto L103;
    ix2=iw+ido;
    ix3=ix2+ido;

    if(na!=0)
      dradb4(ido,l1,ch,c,wa+iw-1,wa+ix2-1,wa+ix3-1);
    else
      dradb4(ido,l1,c,ch,wa+iw-1,wa+ix2-1,wa+ix3-1);
    na=1-na;
    goto L115;

  L103:
    if(ip!=2)goto L106;

    if(na!=0)
      dradb2(ido,l1,ch,c,wa+iw-1);
    else
      dradb2(ido,l1,c,ch,wa+iw-1);
    na=1-na;
    goto L115;

  L106:
    if(ip!=3)goto L109;

    ix2=iw+ido;
    if(na!=0)
      dradb3(ido,l1,ch,c,wa+iw-1,wa+ix2-1);
    else
      dradb3(ido,l1,c,ch,wa+iw-1,wa+ix2-1);
    na=1-na;
    goto L115;

  L109:
/*    The radix five case can be translated later..... */
/*    if(ip!=5)goto L112;

    ix2=iw+ido;
    ix3=ix2+ido;
    ix4=ix3+ido;
    if(na!=0)
      dradb5(ido,l1,ch,c,wa+iw-1,wa+ix2-1,wa+ix3-1,wa+ix4-1);
    else
      dradb5(ido,l1,c,ch,wa+iw-1,wa+ix2-1,wa+ix3-1,wa+ix4-1);
    na=1-na;
    goto L115;

  L112:*/
    if(na!=0)
      dradbg(ido,ip,l1,idl1,ch,ch,ch,c,c,wa+iw-1);
    else
      dradbg(ido,ip,l1,idl1,c,c,c,ch,ch,wa+iw-1);
    if(ido==1)na=1-na;

  L115:
    l1=l2;
    iw+=(ip-1)*ido;
  }

  if(na==0)return;

  for(i=0;i<n;i++)c[i]=ch[i];
}

void drft_forward(drft_lookup *l,float *data){
  if(l->n==1)return;
  drftf1(l->n,data,l->trigcache,l->trigcache+l->n,l->splitcache);
}

void drft_backward(drft_lookup *l,float *data){
  if (l->n==1)return;
  drftb1(l->n,data,l->trigcache,l->trigcache+l->n,l->splitcache);
}

void drft_init(drft_lookup *l,int n){
  l->n=n;
  l->trigcache=(float*)_ogg_calloc(3*n,sizeof(*l->trigcache));
  l->splitcache=(int*)_ogg_calloc(32,sizeof(*l->splitcache));
  fdrffti(n, l->trigcache, l->splitcache);
}

void drft_clear(drft_lookup *l){
  if(l){
    if(l->trigcache)_ogg_free(l->trigcache);
    if(l->splitcache)_ogg_free(l->splitcache);
    my_memset(l,0,sizeof(*l));
  }
}
