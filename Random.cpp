namespace random
{

int xor128(void)
{ 
	static int x = 123456789;
	static int y = 362436069;
	static int z = 521288629;
	static int w = 88675123; 
	int t;

	t = x ^ (x << 11);
	x = y; y = z; z = w;
	return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)); 
}

}