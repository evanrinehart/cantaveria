typedef struct {
  float alpha;
  float y;
} lowpass;

typedef struct {
  float alpha;
  float y, x;
} highpass;

typedef struct {
  float a0,a1,a2,a0r;
  float b0,b1,b2;
  float x[3];
  float y[2];
} biquad;