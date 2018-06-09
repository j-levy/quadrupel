/*
* Source: http://wwwtmp.st.ewi.tudelft.nl/koen/cs4140/Resources/index.html
*/

/*----------------------------------------------------------------
 * float2fix -- convert float to fixed point 18+14 bits
 *----------------------------------------------------------------
 */
int float2fix(double x)
{
	int	y;

	y = x * (1 << 14);
	return y;
}


/*----------------------------------------------------------------
 * fix2float -- convert fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
double fix2float(int x)
{
	double	y;

	y = ((double) x) / (1 << 14);
	return y;
}


/*----------------------------------------------------------------
 * fixmul -- multiply fixed 18+14 bits to float
 *----------------------------------------------------------------
 */
double fixmul(int x1, int x2)
{
	int	y;

	y = x1 * x2; // Note: be sure this fits in 32 bits !!!!
	y = (y >> 14);
	return y;
}
