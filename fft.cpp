#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pi.hpp"

static int ilog2(int n)
{
	int i;
	for(i=0; n>>i; ++i);
	return i;
}

extern const unsigned short bitrev[1<<16];
static unsigned revbits(unsigned k)
{
	return (bitrev[k&0xffff]<<16)|bitrev[k>>16];
}

#define COMPLEX_MULT(r, i, a, b, c, d) r=(a)*(c)-(b)*(d); i=(a)*(d)+(b)*(c)

static void asd_calc_fft_f(float* rdst, float* idst, const float* rsrc, const float* isrc, int n, float z0)
{
	int rb = 33-ilog2(n);

	// bit reverse copy stuff
	for(int i=0; i<n; ++i) {
		int r = revbits(i)>>rb;
//		printf("%d -> %d ; %f %f\n", i, r, rsrc[i], isrc[i]);
		rdst[r] = rsrc[i];
		idst[r] = isrc[i];
	}
//	printf("wqe: %f\n", rdst[0]);

	for(int m=2; m<=n; m<<=1) {
		float z = z0/m;
		float wmr = cos(z), wmi = sin(z);
		int m2=m/2;
		for(int k=0; k<n; k+=m) {
			float wr=1, wi=0;
			for(int j=k; j<k+m2; ++j) {
				float ur = rdst[j], ui = idst[j];
				int h = j + m2;
				float tr,ti;
				COMPLEX_MULT(tr,ti,wr,wi,rdst[h],idst[h]);

//				printf("jee %f %f (%d %d)\n", ur, ui, j, h);

				rdst[j] = ur+tr;
				rdst[h] = ur-tr;
				idst[j] = ui+ti;
				idst[h] = ui-ti;

				float tmpr=wr, tmpi=wi;
				COMPLEX_MULT(wr,wi,tmpr,tmpi,wmr,wmi);
			}
		}
	}
}

void asd_fft_f(float* rdst, float* idst, const float* rsrc, const float* isrc, int n)
{
	asd_calc_fft_f(rdst,idst,rsrc,isrc,n,2*M_PI);
}
void asd_ifft_f(float* rdst, float* idst, const float* rsrc, const float* isrc, int n)
{
	asd_calc_fft_f(rdst,idst,rsrc,isrc,n,-2*M_PI);
	float r = 1.f/n;
	for(int i=0; i<n; ++i) {
		rdst[i] *= r;
		idst[i] *= r;
	}
}
