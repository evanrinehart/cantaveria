/* dsp routines */

#include <math.h>
#include <stdlib.h>

#include <dsp.h>


#define PI2 2*3.14159265358

int sample_rate = 0;




void setup_lowpass(lowpass* lp, float f0, float srate){
	float T = 1/srate;
	float RC = 1/(PI2*f0);

	lp->alpha = T / (RC + T);
	lp->y = 0;
}

void apply_lowpass(lowpass* lp, float buf[], int count){
	int i;
	for(i=0; i<count; i++){
		buf[i] = lp->y + lp->alpha*(buf[i] - lp->y);
		lp->y = buf[i];
	}
}


void setup_highpass(highpass* hp, float f0, float srate){
	float T = 1/srate;
	float RC = 1/(PI2*f0);

	hp->alpha = RC / (RC + T);
	hp->y = 0;
	hp->x = 0;
}

void apply_highpass(highpass* hp, float buf[], int count){
	int i;
	for(i=0; i<count; i++){
		float x = buf[i];
		buf[i] = hp->alpha * (buf[i] + hp->y - hp->x);
		hp->x = x;
		hp->y = buf[i];
	}
}


void setup_lp2(biquad* bq, float f0, float Q, float srate){
	float w0 = PI2*f0 / srate;
	float s = sin(w0);
	float c = cos(w0);
	float alpha = s / (2*Q);

	bq->a0 = 1+alpha;
	bq->a1 = -2*c;
	bq->a2 = 1-alpha;
	bq->b0 = (1-c)/2;
	bq->b1 = 1-c;
	bq->b2 = (1-c)/2;
	bq->a0r = 1/bq->a0;
}

void setup_hp2(biquad* bq, float f0, float Q, float srate){
	float w0 = PI2*f0 / srate;
	float s = sin(w0);
	float c = cos(w0);
	float alpha = s / (2*Q);

	bq->a0 = 1+alpha;
	bq->a1 = -2*c;
	bq->a2 = 1-alpha;
	bq->b0 = (1+c)/2;
	bq->b1 = -1-c;
	bq->b2 = (1+c)/2;
	bq->a0r = 1/bq->a0;
}

void setup_bp2(biquad* bq, float f0, float BW, float srate){
	float w0 = PI2*f0 / srate;
	float s = sin(w0);
	float c = cos(w0);
	float alpha = s*sinh( log(2)/2 * BW * w0/s );

	bq->a0 = 1+alpha;
	bq->a1 = -2*c;
	bq->a2 = 1-alpha;
	bq->b0 = alpha;
	bq->b1 = 0;
	bq->b2 = -alpha;
	bq->a0r = 1/bq->a0;
}

void apply_biquad(biquad* bq, float buf[], int count){
	int i;
	for(i=0; i<count; i++){
		float x = buf[i];

		buf[i] = bq->a0r*(bq->b0*buf[i] + bq->b1*bq->x[0] + bq->b2*bq->x[1] 
				- bq->a1*bq->y[0] - bq->a2*bq->y[1]);

		bq->x[1] = bq->x[0];
		bq->x[0] = x;
		bq->y[1] = bq->y[0];
		bq->y[0] = buf[i];
	}
}



int reverse(int x){
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return((x >> 16) | (x << 16));
}

void ifft(float in[][2], float out[], const int N){

	int D = log2(N)-1;
	float T[D][N][2];

	int n, L;
	int m = 0;
	for(n=0; n<N; n++){/* computes top level by reordering input */
		int n1 = reverse(m++);
		int n2 = reverse(m++);
		if(n < 32){
			T[D-1][n][0] = in[n1][0] + in[n2][0];
			T[D-1][n][1] = in[n1][1] + in[n2][1];
		}
		else{
			T[D-1][n][0] = in[n1][0] - in[n2][0];
			T[D-1][n][1] = in[n1][1] - in[n2][1];
		}
	}

	for(L=D-2; L>=0; L--){/* compute lower levels in terms of one above */
		int j = 0;
		for(n=0; n<64; n++){
			int A = D+1;
			int J = 1<<A;
			float* T0 = T[A][j];
			float* T1 = T[A][j+1];
			float angle = PI2*((n>>A)<<A)/N;
			float C[2] = {cos(angle), sin(angle)};

			//T[L][n] = T0 + T1*C; /* mini fourier transform */
			T[L][n][0] = T1[0]*C[0] - T1[1]*C[1];
			T[L][n][1] = T1[0]*C[1] + T1[1]*C[0];
			T[L][n][0] += T0[0];
			T[L][n][1] += T0[1];

			j+=2;
			if(j == J) j=0;
		}
	}

	for(n=0; n<N; n++){/* compute output from bottom level */
		float* T0 = T[0][0];
		float* T1 = T[0][1];
		float angle = PI2*n/N;
		float C[2] = {cos(angle), sin(angle)};

		//out[n] = T0 + T1*circle[n];
		out[n] = T1[0]*C[0] - T1[1]*C[1];
		out[n] += T0[0];
	}

}





/* delay line routines for use with string synth, reverb chorus echo effects*/
typedef struct {
	float* buf;
	int ptr_in;
	int ptr_out;
	int len;
	int size;
} delayline;


/* write count samples to the beginning of the delay line */
void delay_write(delayline* d, float in[], int count){
	int i;
	if(d->ptr_in + count < d->size){
		for(i=0; i<count; i++){
			d->buf[d->ptr_in++] = in[i];
		}
	}
	else{
		int N = d->size - d->ptr_in;
		for(i=0; i<N; i++){
			d->buf[d->ptr_in++] = in[i];
		}
		d->ptr_in = 0;
		for(i=N; i < count; i++){
			d->buf[d->ptr_in++] = in[i];
		}
	}
}

/* read count samples from the end of the delay line */
void delay_read(delayline* d, float out[], int count){
	int i;
	if(d->ptr_out + count < d->size){
		for(i=0; i<count; i++){
			out[i] = d->buf[d->ptr_out++];
		}
	}
	else{
		int N = d->size - d->ptr_out;
		for(i=0; i<N; i++){
			out[i] = d->buf[d->ptr_out++];
		}
		d->ptr_out = 0;
		for(i=N; i < count; i++){
			out[i] = d->buf[d->ptr_out++];
		}
	}
}

/* same as delay_read but does not actually read anything */
/*
   void delay_drop(delayline* d, int count){
   if(d->ptr_out + count < d->size){
   d->ptr_count += count;
   }
   else{
   d->ptr_count = count - (d->size - d->ptr_out);
   }
   }*/

/* read count interpolated samples from t samples in the past */
/*
   void delay_extract(delayline* d, float t, float out[], int count){
   float x = d->ptr_in - t;
   if(x < 0) x += d->size;

   if(x + count < d->size){
   float x0 = floor(x);
   float x1 = x0+1.0;
   float y0 = d->buf[x0];
   float y1 = d->buf[x1];
   for(int i=0; i<count; i++){
   float m = (y1-y0)/(x1-x0);
   out[i] = m*(x-x0) + y0;
   x += 1.0;
   x0 += x1;
   x1 += 1.0;
   y0 = y1;
   y1 = d->buf[x1];
   }
   }
   else{
   int N = d->size - d->ptr_out;
   float x0 = floor(x);
   float x1 = x0+1.0;
   float y0 = d->buf[x0];
   float y1 = d->buf[x1];
   for(int i=0; i<N; i++){
   float m = (y1-y0)/(x1-x0);
   out[i] = m*(x-x0) + y0;
   x += 1.0;
   x0 += x1;
   x1 += 1.0;
   y0 = y1;
   y1 = d->buf[x1];
   }

   x0 = d->size - 1;
   x1 = d->size;
   y0 = d->buf[x0];
   y1 = d->buf[0];
   float m = (y1-y0)/(x1-x0);
   out[N] = m*(x-x0) + y0;

   x += 1.0;
   x -= d->size;
   x0 = 0;
   x1 = 1;
   y0 = d->buf[0];
   y1 = d->buf[1];
   for(int i=N+1; i < count; i++){
   float m = (y1-y0)/(x1-x0);
   out[i] = m*(x-x0) + y0;
   x += 1.0;
   x0 += x1;
   x1 += 1.0;
   y0 = y1;
   y1 = d->buf[x1];
   }
   }
   }*/

/* read samples using a variable delay signal */
void delay_variable(delayline* d, float t[], float out[], int count){
	int i;
	for(i=0; i<count; i++){
		//delay_extract(d, t[i]+i, out+i, count);
	}
}




/* synthesizes a band limited square/saw wave using 16x oversampling */
typedef struct {
	float dy; /* frequency */
	float y; /* phase counter */
	float x; /* low pass state */
} pulse;

void set_pulse_freq(pulse* s, float f){
	s->dy = 2*f/(sample_rate*16);
}

void generate_pulse(pulse* s, float step[], float out[], int count, int type){
	int N = count*16;
	float buf[N];
	int i;

	const float T = (1.0f/(sample_rate*16));
	const float A = T/(1.0f/(PI2*10000) + T);

	/* generate */
	if(type==0){
		for(i=0; i<N; i++){
			//s->y += s->dy;
			s->y += step[i];
			while(s->y > 1){s->y -= 2;}
			buf[i] = s->y < 0 ? 1 : -1;
		}
	}
	else{
		for(i=0; i<N; i++){
			buf[i] = s->y;
			//s->y += s->dy;
			s->y += step[i];
			while(s->y > 1){s->y -= 2;}
		}
	}

	/* low pass */
	for(i=0; i<N; i++){
		buf[i] = s->x + A*(buf[i] - s->x);
		s->x = buf[i];
	}

	/* downsample */
	int j = 0;
	for(i=0; i<count; i++){
		out[i] = buf[j];
		j+=16;
	}
}




/* PAD synth algorithm */
void pad_synth(float amp[], float out[], int size){
	int i;
	float (*in)[][2] = malloc(size*sizeof(float[2]));
	for(i=0; i<size; i++){
		float A = amp[i];
		float p = (rand()*PI2)/RAND_MAX;
		(*in)[i][0] = A*cos(p);
		(*in)[i][1] = A*sin(p);
	}
	ifft(*in, out, size);
	free(in);
}



